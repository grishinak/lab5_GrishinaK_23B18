#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(1234);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // IP адрес сервера
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Error connecting to server" << std::endl;
        return 1;
    }

    std::cout << "Connected to server" << std::endl;

    // Ввод запроса
    std::string expression;
    std::cout << "Enter an expression (e.g., 10 + 5): ";
    std::getline(std::cin, expression);

    // Отправка запроса
    write(clientSocket, expression.c_str(), expression.length());

    // Получение ответа
    char buffer[1024] = {0};
    int bytesRead = read(clientSocket, buffer, sizeof(buffer));
    if (bytesRead == -1) {
        std::cerr << "Error reading from socket" << std::endl;
        return 1;
    }

    std::cout << "Server response: " << buffer << std::endl;

    // Закрытие соединения
    close(clientSocket);

    return 0;
}