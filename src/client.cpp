#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <sstream>
#include "parser.h"

// Объявление функций для работы клиента
void send_request(int client_socket, const std::string& request);
void register_user(int client_socket, const std::string& username,
                   const std::string& password);
void login_user(int client_socket, const std::string& username,
                const std::string& password);
void get_history(int client_socket, const std::string& username);

int main() {
  int client_socket;
  struct sockaddr_in serv_addr;

  // Создание сокета клиента
  if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    std::cerr << "Socket creation error" << std::endl;
    return -1;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(8080);

  // Преобразование IP адреса в формат сети
  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
    std::cerr << "Invalid address/ Address not supported" << std::endl;
    return -1;
  }

  // Подключение к серверу
  if (connect(client_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) <
      0) {
    std::cerr << "Connection Failed" << std::endl;
    return -1;
  }

  std::string command;
  std::string username, password;

  while (true) {
    std::cout << "Enter 'register' to create a new account, 'login' to login, "
                 "'history' to view your history, 'calculate' to be able to "
                 "calculate an algebraic expression or 'exit' to quit: ";
    std::cin >> command; //На самом деле нет команды calculate, просто что-либо отличное от перечисленных 

    if (command == "exit") {
      // Отправляем команду на отключение и выходим из цикла
      send_request(client_socket, "EXIT");
      break;
    } else if (command == "register") {
      // Регистрация нового пользователя
      std::cout << "Enter username: ";
      std::cin >> username;
      std::cout << "Enter password: ";
      std::cin >> password;
      register_user(client_socket, username, password);
    } else if (command == "login") {
      // Вход пользователя
      std::cout << "Enter username: ";
      std::cin >> username;
      std::cout << "Enter password: ";
      std::cin >> password;
      login_user(client_socket, username, password);
    } else if (command == "history") {
      // Получение истории запросов пользователя
      get_history(client_socket, username);
    } else {
      // Обработка алгебраического выражения
      std::string expression;
      std::cin.ignore();  // Очищаем буфер ввода
      std::cout << "Enter an expression to evaluate (e.g., '2+2', '3*4', "
                   "etc.), or 'exit' to quit: ";
      std::getline(std::cin, expression);
      send_request(client_socket, expression);
    }
  }

  close(client_socket);
  // Выводим сообщение об отключении от сервера
  std::cout << "Disconnected from server" << std::endl;
  return 0;
}

// Функция для отправки запроса на сервер и получения ответа
void send_request(int client_socket, const std::string& request) {
  char buffer[1024] = {0};
  send(client_socket, request.c_str(), request.length(), 0);
  recv(client_socket, buffer, 1024, 0);
  if (request != "EXIT") {
    std::cout << "Result: " << buffer << std::endl;
  }
}

// Функция для регистрации нового пользователя
void register_user(int client_socket, const std::string& username,
                   const std::string& password) {
  std::string request = "REGISTER " + username + " " + password;
  send_request(client_socket, request);
}

// Функция для входа пользователя
void login_user(int client_socket, const std::string& username,
                const std::string& password) {
  std::string request = "LOGIN " + username + " " + password;
  send_request(client_socket, request);
}

// Функция для получения истории запросов пользователя
void get_history(int client_socket, const std::string& username) {
  std::string request = "GET_HISTORY " + username;
  send_request(client_socket, request);
}