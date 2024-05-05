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
    std::string expression;
    std::string result;
};

// Функция обработки запроса на сервере
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

        if (request == "GET_HISTORY") {
            // Получаем историю запросов клиента
            std::string client_history_query = "SELECT expression, result FROM queries;";
            sqlite3_stmt *stmt;
            std::stringstream response;
            if (sqlite3_prepare_v2(db, client_history_query.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
                while (sqlite3_step(stmt) == SQLITE_ROW) {
                    const unsigned char *expression = sqlite3_column_text(stmt, 0);
                    const unsigned char *result = sqlite3_column_text(stmt, 1);
                    response << expression << ": " << result << std::endl;
                }
                sqlite3_finalize(stmt);
            } else {
                response << "Error retrieving history.";
            }

            // Отправляем историю клиенту
            std::string response_str = response.str();
            send(client_socket, response_str.c_str(), response_str.length(), 0);
        } else if (request == "EXIT") {
            // Если получена команда на отключение, закрываем сокет и выводим сообщение об отключении клиента
            close(client_socket);
            std::cout << "Client disconnected. Address: " << inet_ntoa(client_address.sin_addr) << ", Port: " << ntohs(client_address.sin_port) << std::endl;
            return;
        } else {
            // Выполнение вычислений
            std::string expression = request;
            double result = evaluate_expression(expression);

            // Сохранение запроса и результата в базу данных
            Query query;
            query.expression = expression;
            query.result = std::to_string(result); // Преобразуем результат в строку для сохранения в базу данных

            // Вставка запроса и результата в базу данных
            std::stringstream ss;
            ss << "INSERT INTO queries (expression, result) VALUES ('" << query.expression << "', '" << query.result << "');";
            std::string sql = ss.str();
            sqlite3_exec(db, sql.c_str(), NULL, 0, NULL);

            // Отправка результата клиенту
            send(client_socket, query.result.c_str(), query.result.length(), 0);
        }
    }
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
    std::string create_table_query = "CREATE TABLE IF NOT EXISTS queries (id INTEGER PRIMARY KEY AUTOINCREMENT, expression TEXT, result TEXT);";
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
