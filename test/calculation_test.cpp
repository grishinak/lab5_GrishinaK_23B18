#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <chrono>

// Функция для отправки запроса на сервер и получения ответа с измерением времени выполнения
void send_request(int client_socket, const std::string& request) {
    char buffer[1024] = {0};
    auto start_time = std::chrono::high_resolution_clock::now(); // Записываем время начала отправки запроса
    send(client_socket, request.c_str(), request.length(), 0);
    recv(client_socket, buffer, 1024, 0);
    auto end_time = std::chrono::high_resolution_clock::now(); // Записываем время получения ответа
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "Expression: " << request << ", Result: " << buffer << ", Execution Time: " << duration.count() << " milliseconds" << std::endl;
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
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        close(client_socket);
        return -1;
    }

    // Подключение к серверу
    if (connect(client_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed" << std::endl;
        close(client_socket);
        return -1;
    }

    // Тестирование вычисления алгебраического выражения
    send_request(client_socket, "2+2");
    send_request(client_socket, "3*4");
    send_request(client_socket, "(5+6)*7");
    send_request(client_socket, "sqrt(16)");
    send_request(client_socket, "log(100)");
    send_request(client_socket, "sin(0.5)");

    // Закрытие сокета
    close(client_socket);
    
    return 0;
}
