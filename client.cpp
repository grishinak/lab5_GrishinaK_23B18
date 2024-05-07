#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sstream>
#include "parser.h"

// Функция для отправки запроса на сервер и получения ответа
void send_request(int client_socket, const std::string& request) {
    char buffer[1024] = {0};
    send(client_socket, request.c_str(), request.length(), 0);
    recv(client_socket, buffer, 1024, 0);
    std::cout << buffer << std::endl;
}

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
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        return -1;
    }

    // Подключение к серверу
    if (connect(client_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed" << std::endl;
        return -1;
    }

    std::string command;
    // std::getline(std::cin, command); // Очищаем буфер после считывания имени пользователя???
    while (true) {
        std::cout << "Enter 'register' to create a new account, 'login' to login, 'history' to view your history, or 'exit' to quit: ";
        std::getline(std::cin, command);

        if (command == "exit") {
            // Отправляем команду на отключение и выходим из цикла
            send_request(client_socket, "EXIT");
            break;
        } else if (command == "register") {
            // Регистрация нового пользователя
            std::string username, password;
            std::cout << "Enter username: ";
            std::cin >> username;
            std::cout << "Enter password: ";
            std::cin >> password;
            send_request(client_socket, "REGISTER " + username + " " + password);
        } else if (command == "login") {
            // Вход пользователя
            std::string username, password;
            std::cout << "Enter username: ";
            std::cin >> username;
            std::cout << "Enter password: ";
            std::cin >> password;
            send_request(client_socket, "LOGIN " + username + " " + password);
        } else if (command == "history") {
            // Получение истории запросов пользователя
            // TODO
        } else {
            // Обработка алгебраического выражения
            // TODO
        }
    }

    close(client_socket);
    // Выводим сообщение об отключении от сервера
    std::cout << "Disconnected from server" << std::endl;
    return 0;
}
