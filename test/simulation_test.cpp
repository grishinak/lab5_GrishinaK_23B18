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

// Функция для регистрации нового пользователя с задержкой и измерением времени выполнения
void register_user_with_delay(int client_socket, const std::string& username,
                   const std::string& password) {
  std::string request = "REGISTER " + username + " " + password;
  send_request(client_socket, request);
}

// Функция для входа пользователя с задержкой и измерением времени выполнения
void login_user_with_delay(int client_socket, const std::string& username,
                const std::string& password) {
  std::string request = "LOGIN " + username + " " + password;
  send_request(client_socket, request);
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

    // Регистрация нового пользователя
    std::string username = "test_user";
    std::string password = "test_password";
    register_user_with_delay(client_socket, username, password);

    // Вход пользователя
    login_user_with_delay(client_socket, username, password);

    // Тестирование вычисления алгебраического выражения
    send_request(client_socket, "2+2*2-2");

    // Закрытие сокета
    close(client_socket);
    
    return 0;
}
