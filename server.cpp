// server.cpp
#include <iostream>
#include <thread>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>
#include <sstream>
#include <sqlite3.h>
#include <arpa/inet.h>
#include "parser.h"

// Структура для хранения запроса и ответа
struct Query {
    std::string username;
    std::string expression;
    std::string result;
};

// Объявление функции username_exists
bool username_exists(sqlite3* db, const std::string& username);

// Объявление функции register_user
bool register_user(sqlite3* db, const std::string& username, const std::string& password);

// Функция для отправки сообщений клиенту
void send_response(int client_socket, const std::string& response) {
    send(client_socket, response.c_str(), response.length(), 0);
}

// Функция для обработки запросов на сервере
void handle_request(int client_socket, sqlite3* db) {
    // Получаем информацию об адресе клиента
    struct sockaddr_in client_address;
    socklen_t client_addr_len = sizeof(client_address);
    getpeername(client_socket, (struct sockaddr*)&client_address, &client_addr_len);

    // Выводим сообщение о подключении клиента
    std::cout << "Client connected. Address: " << inet_ntoa(client_address.sin_addr) << ", Port: " << ntohs(client_address.sin_port) << std::endl;

    char buffer[1024] = {0};
    while (true) {
        recv(client_socket, buffer, 1024, 0);
        std::string request(buffer);

        // Разбираем команду пользователя и его имя
        std::istringstream iss(request);
        std::string expression;
        iss >> expression;

        if (expression == "EXIT") {
            // Если получена команда на отключение, закрываем соединение с клиентом и завершаем обработку запросов
            close(client_socket);
            std::cout << "Client disconnected. Address: " << inet_ntoa(client_address.sin_addr) << ", Port: " << ntohs(client_address.sin_port) << std::endl;
            return;
        }

        // Вычисляем выражение и отправляем результат клиенту
        double result = evaluate_expression(expression);
        send_response(client_socket, std::to_string(result));
    }
}

// Функция для регистрации нового пользователя
bool register_user(sqlite3* db, const std::string& username, const std::string& password) {
    std::stringstream ss;
    ss << "INSERT INTO users (username, password) VALUES ('" << username << "', '" << password << "');";
    std::string sql = ss.str();
    int result = sqlite3_exec(db, sql.c_str(), NULL, 0, NULL);
    return result == SQLITE_OK;
}

// Функция для проверки существования имени пользователя
bool username_exists(sqlite3* db, const std::string& username) {
    std::string query = "SELECT COUNT(*) FROM users WHERE username='" + username + "';";
    sqlite3_stmt *stmt;
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
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    // Привязка сокета к адресу и порту
    if (bind(server_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Прослушивание сокета
    if (listen(server_socket, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Подключение к базе данных SQLite
    sqlite3* db;
    if (sqlite3_open("queries.db", &db)) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return 0;
    }

    // Создание таблицы для хранения запросов, если её ещё нет
    std::string create_table_query = "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT UNIQUE, password TEXT);";
    sqlite3_exec(db, create_table_query.c_str(), NULL, 0, NULL);

    while (true) {
        int client_socket;
        // Принятие нового подключения
        if ((client_socket = accept(server_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // Создание нового потока для обработки запроса
        std::thread(handle_request, client_socket, db).detach();
    }

    sqlite3_close(db);
    return 0;
}
