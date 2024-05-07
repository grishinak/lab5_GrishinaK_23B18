// client.cpp
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
    std::string response(buffer);
    if (!response.empty() && response != "nan") {
        std::cout << "Result: " << response << std::endl;
    } else if (response == "nan") {
        std::cout << "Error: Invalid expression" << std::endl;
    }
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

    std::string expression;
    while (true) {
        std::cout << "Enter an expression to evaluate (e.g., '2+2', '3*4', etc.), or 'exit' to quit: ";
        std::getline(std::cin, expression);

        if (expression == "exit") {
            // Отправляем команду на отключение и выходим из цикла
            send_request(client_socket, "EXIT");
            break;
        } else {
            // Отправляем выражение на сервер для вычисления
            send_request(client_socket, expression);
        }
    }

    close(client_socket);
    // Выводим сообщение об отключении от сервера
    std::cout << "Disconnected from server" << std::endl;
    return 0;
}
