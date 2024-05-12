#include <iostream>
#include <thread>
#include <vector>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h> // Для close()
#include <cstring>
#include <random>

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
void register_user(int client_socket, const std::string& username, const std::string& password) {
    std::string request = "REGISTER " + username + " " + password;
    send_request(client_socket, request);
}

int main() {
    int num_users = 10; // Количество пользователей для регистрации
    int client_socket;
    struct sockaddr_in serv_addr;

    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        return -1;
    }

    if (connect(client_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed" << std::endl;
        return -1;
    }

    // Генерируем случайные имена пользователей и пароли
    std::vector<std::string> usernames = {"user13", "user23", "user33", "user43", "user53"};
    std::vector<std::string> passwords = {"password1", "password2", "password3", "password4", "password5"};

    // Создаем вектор потоков для выполнения запросов
    std::vector<std::thread> threads;
    for (int i = 0; i < num_users; ++i) {
        // Выбираем случайное имя пользователя и пароль
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, usernames.size() - 1);
        int index = dis(gen);
        std::string username = usernames[index];
        std::string password = passwords[index];

        // Создаем новый поток для каждой регистрации пользователя
        threads.emplace_back(std::thread(register_user, client_socket, username, password));
    }

    // Ждем завершения всех операций
    for (auto& thread : threads) {
        thread.join();
    }

    close(client_socket); // Закрываем сокет

    return 0;
}
