#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib> // Для функции atof
#include <sstream> // Для работы с потоками

// Функция для вычисления выражения
double calculateExpression(const std::string& expression) {
    std::istringstream iss(expression);
    double num1, num2;
    char op;
    iss >> num1 >> op >> num2;
    switch(op) {
        case '+':
            return num1 + num2;
        case '-':
            return num1 - num2;
        case '*':
            return num1 * num2;
        case '/':
            if(num2 != 0) {
                return num1 / num2;
            } else {
                return 0; // Обработка деления на ноль
            }
        default:
            return 0; // Обработка некорректного оператора
    }
}

int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(1234);
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Error binding socket" << std::endl;
        return 1;
    }

    if (listen(serverSocket, 5) == -1) {
        std::cerr << "Error listening on socket" << std::endl;
        return 1;
    }

    std::cout << "Server listening on port 1234..." << std::endl;

    while (true) {
        sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (clientSocket == -1) {
            std::cerr << "Error accepting connection" << std::endl;
            continue;
        }

        std::cout << "Client connected" << std::endl;

        // Принимаем запрос от клиента
        char buffer[1024] = {0};
        int bytesRead = read(clientSocket, buffer, sizeof(buffer));
        if (bytesRead == -1) {
            std::cerr << "Error reading from socket" << std::endl;
            close(clientSocket);
            continue;
        }

        // Вычисляем результат выражения
        std::string expression(buffer);
        double result = calculateExpression(expression);

        // Отправляем результат обратно клиенту
        std::ostringstream oss;
        oss << "Result: " << result;
        std::string resultMsg = oss.str();
        write(clientSocket, resultMsg.c_str(), resultMsg.length());

        // Закрываем соединение
        close(clientSocket);
    }

    close(serverSocket);

    return 0;
}