#include <arpa/inet.h>
#include <netinet/in.h>
#include <sqlite3.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>
#include <vector>
#include "parser.h"

// Структура для хранения запроса и ответа
struct Query {
  std::string username;
  std::string expression;
  std::string result;
};

// Объявление функций для работы с базой данных
bool register_user(sqlite3* db, const std::string& username,
                   const std::string& password);
bool login_user(sqlite3* db, const std::string& username,
                const std::string& password);
bool username_exists(sqlite3* db, const std::string& username);
void save_query(sqlite3* db, const std::string& username,
                const std::string& expression, const std::string& result);
void send_response(int client_socket, const std::string& response);

// Объявление функций для работы сервера
void handle_request(int client_socket, sqlite3* db);
std::string get_user_history(sqlite3* db, const std::string& username);

// Глобальный мьютекс для синхронизации доступа к базе данных
std::mutex db_mutex;

int main() {
  int server_socket;
  struct sockaddr_in address;
  int opt = 1;
  int addrlen = sizeof(address);

  // Создание сокета сервера
  if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  // Настройка параметров сокета
  if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt))) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(8080);

  // Привязка сокета к адресу и порту
  if (bind(server_socket, (struct sockaddr*)&address, sizeof(address)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  // Прослушивание сокета
  if (listen(server_socket, 10) < 0) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  // Подключение к базе данных SQLite
  sqlite3* db;
  if (sqlite3_open("queries.db", &db)) {
    std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
    return 0;
  }

  // Создание таблицы для хранения пользователей, если её ещё нет
  std::string create_users_table_query =
      "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY AUTOINCREMENT, "
      "username TEXT UNIQUE, password TEXT);";
  {
    std::lock_guard<std::mutex> lock(
        db_mutex);  // Захватываем мьютекс перед выполнением запроса
    sqlite3_exec(db, create_users_table_query.c_str(), NULL, 0, NULL);
  }

  // Создание таблицы для хранения истории запросов, если её ещё нет
  std::string create_history_table_query =
      "CREATE TABLE IF NOT EXISTS history (id INTEGER PRIMARY KEY "
      "AUTOINCREMENT, username TEXT, expression TEXT, result TEXT);";
  {
    std::lock_guard<std::mutex> lock(
        db_mutex);  // Захватываем мьютекс перед выполнением запроса
    sqlite3_exec(db, create_history_table_query.c_str(), NULL, 0, NULL);
  }

  while (true) {
    int client_socket;
    // Принятие нового подключения
    if ((client_socket = accept(server_socket, (struct sockaddr*)&address,
                                (socklen_t*)&addrlen)) < 0) {
      perror("accept");
      exit(EXIT_FAILURE);
    }

    // Создание нового потока для обработки запроса
    std::thread(handle_request, client_socket, db).detach();
  }

  sqlite3_close(db);
  return 0;
}

// Функция для обработки запросов на сервере
void handle_request(int client_socket, sqlite3* db) {
  // Получаем информацию об адресе клиента
  struct sockaddr_in client_address;
  socklen_t client_addr_len = sizeof(client_address);
  getpeername(client_socket, (struct sockaddr*)&client_address,
              &client_addr_len);

  // Выводим сообщение о подключении клиента
  std::cout << "🟢 Client connected. Address: "
            << inet_ntoa(client_address.sin_addr)
            << ", Port: " << ntohs(client_address.sin_port) << std::endl;

  char buffer[1024] = {0};
  while (true) {
    recv(client_socket, buffer, 1024, 0);
    std::string request(buffer);

    // Разбираем команду пользователя и его имя
    std::istringstream iss(request);
    std::string command, username;
    iss >> command >> username;

    if (command == "REGISTER") {
      // Регистрация нового пользователя
      std::string password;
      iss >> password;

      if (username.empty() || password.empty()) {
        send_response(client_socket,
                      "Error: Username or password cannot be empty.");
      } else {
        // Проверяем, не занято ли имя пользователя
        {
          std::lock_guard<std::mutex> lock(
              db_mutex);  // Захватываем мьютекс перед выполнением запроса
          if (username_exists(db, username)) {
            send_response(client_socket, "Error: Username already exists.");
            continue;  // Пропускаем оставшуюся часть обработки запроса
          }
        }

        // Регистрируем нового пользователя
        bool registration_successful = false;
        {
          std::lock_guard<std::mutex> lock(
              db_mutex);  // Захватываем мьютекс перед выполнением запроса
          registration_successful = register_user(db, username, password);
        }

        if (registration_successful) {
          send_response(client_socket,
                        "Registration successful. You can now login.");
        } else {
          send_response(client_socket, "Error: Registration failed.");
        }
      }
    } else if (command == "LOGIN") {
      // Вход пользователя
      std::string password;
      iss >> password;

      if (username.empty() || password.empty()) {
        send_response(client_socket,
                      "Error: Username or password cannot be empty.");
      } else {
        bool login_successful = false;
        {
          std::lock_guard<std::mutex> lock(
              db_mutex);  // Захватываем мьютекс перед выполнением запроса
          login_successful = login_user(db, username, password);
        }

        if (login_successful) {
          send_response(client_socket, "Login successful.");
        } else {
          send_response(client_socket, "Error: Invalid username or password.");
        }
      }
    } else if (command == "GET_HISTORY") {
      // Получение истории запросов данного пользователя
      std::string history;
      {
        std::lock_guard<std::mutex> lock(
            db_mutex);  // Захватываем мьютекс перед выполнением запроса
        history = get_user_history(db, username);
      }
      send_response(client_socket, history);
    } else if (command == "EXIT") {
      // Если получена команда на отключение, закрываем соединение с клиентом и завершаем обработку запросов
      close(client_socket);
      std::cout << "🔴 Client disconnected. Address: "
                << inet_ntoa(client_address.sin_addr)
                << ", Port: " << ntohs(client_address.sin_port) << std::endl;
      return;
    } else {
      // Выполняем вычисления
      std::string expression = command;
      double result = evaluate_expression(expression);

      // Сохраняем запрос и результат в базе данных
      Query query;
      query.username = username;
      query.expression = expression;
      query.result = std::to_string(
          result);  // Преобразуем результат в строку для сохранения в базе данных
      {
        std::lock_guard<std::mutex> lock(
            db_mutex);  // Захватываем мьютекс перед выполнением запроса
        save_query(db, query.username, query.expression, query.result);
      }

      // Отправляем результат клиенту
      send_response(client_socket, query.result);
    }
  }
}

// Функция для получения истории запросов пользователя
std::string get_user_history(sqlite3* db, const std::string& username) {
  std::string history;
  std::stringstream ss;
  ss << "SELECT expression, result FROM history WHERE username='" << username
     << "';";
  std::string query = ss.str();
  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
    while (sqlite3_step(stmt) == SQLITE_ROW) {
      std::string expression =
          reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
      std::string result =
          reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
      history += "Expression: " + expression + ", Result: " + result + "\n";
    }
    sqlite3_finalize(stmt);
  }
  return history;
}

// Функция для регистрации нового пользователя
bool register_user(sqlite3* db, const std::string& username,
                   const std::string& password) {
  std::stringstream ss;
  ss << "INSERT INTO users (username, password) VALUES ('" << username << "', '"
     << password << "');";
  std::string sql = ss.str();
  int result = sqlite3_exec(db, sql.c_str(), NULL, 0, NULL);
  return result == SQLITE_OK;
}
// Функция для входа нового пользователя
bool login_user(sqlite3* db, const std::string& username,
                const std::string& password) {
  std::string query = "SELECT COUNT(*) FROM users WHERE username='" + username +
                      "' AND password='" + password + "';";
  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      int count = sqlite3_column_int(stmt, 0);
      sqlite3_finalize(stmt);
      return count > 0;
    }
    sqlite3_finalize(stmt);
  }
  return false;
}

// Функция для проверки существования имени пользователя
bool username_exists(sqlite3* db, const std::string& username) {
  std::string query =
      "SELECT COUNT(*) FROM users WHERE username='" + username + "';";
  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      int count = sqlite3_column_int(stmt, 0);
      sqlite3_finalize(stmt);
      return count > 0;
    }
    sqlite3_finalize(stmt);
  }
  return false;
}

// Функция для сохранения запроса и результата в базе данных
void save_query(sqlite3* db, const std::string& username,
                const std::string& expression, const std::string& result) {
  std::stringstream ss;
  ss << "INSERT INTO history (username, expression, result) VALUES ('"
     << username << "', '" << expression << "', '" << result << "');";
  std::string sql = ss.str();
  sqlite3_exec(db, sql.c_str(), NULL, 0, NULL);
}

// Функция для отправки сообщений клиенту
void send_response(int client_socket, const std::string& response) {
  send(client_socket, response.c_str(), response.length(), 0);
}
