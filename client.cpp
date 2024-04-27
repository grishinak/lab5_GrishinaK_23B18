#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cmath>
#include <limits>

// Предварительные объявления функций парсера
double parse_expression(const std::string& expression, int& i);
double parse_term(const std::string& expression, int& i);
double parse_factor(const std::string& expression, int& i);

// Функция для отправки запроса на сервер и получения ответа
void send_request(int client_socket, const std::string& request) {
    char buffer[1024] = {0};
    send(client_socket, request.c_str(), request.length(), 0);
    recv(client_socket, buffer, 1024, 0);
    std::cout << buffer << std::endl;
}

// Функция для вычисления математического выражения
double evaluate_expression(const std::string& expression) {
    // Реализация простого парсера
    int i = 0;
    return parse_expression(expression, i);
}

// Функции парсера
double parse_expression(const std::string& expression, int& i) {
    double result = parse_term(expression, i);

    while (i < expression.length()) {
        if (expression[i] == '+') {
            i++;
            result += parse_term(expression, i);
        } else if (expression[i] == '-') {
            i++;
            result -= parse_term(expression, i);
        } else {
            break;
        }
    }

    return result;
}

double parse_term(const std::string& expression, int& i) {
    double result = parse_factor(expression, i);

    while (i < expression.length()) {
        if (expression[i] == '*') {
            i++;
            result *= parse_factor(expression, i);
        } else if (expression[i] == '/') {
            i++;
            double denominator = parse_factor(expression, i);
            if (denominator != 0.0) {
                result /= denominator;
            } else {
                std::cerr << "Error: Division by zero" << std::endl;
                return std::numeric_limits<double>::quiet_NaN();
            }
        } else {
            break;
        }
    }

    return result;
}

double parse_factor(const std::string& expression, int& i) {
    double result = 0.0;

    if (i < expression.length()) {
        if (expression[i] == '(') {
            i++;
            result = parse_expression(expression, i);
            if (i < expression.length() && expression[i] == ')') {
                i++;
            } else {
                std::cerr << "Error: Unbalanced parentheses" << std::endl;
                return std::numeric_limits<double>::quiet_NaN();
            }
        } else if (isdigit(expression[i]) || expression[i] == '-' || expression[i] == '+') {
            size_t j;
            result = std::stod(expression.substr(i), &j);
            i += j;
        } else {
            std::cerr << "Error: Invalid expression" << std::endl;
            return std::numeric_limits<double>::quiet_NaN();
        }
    }

    return result;
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
    std::cout << "Enter algebraic expression or type 'history' to view your history: ";
    std::getline(std::cin, command);

    if (command == "history") {
        // Отправляем запрос на получение истории
        send_request(client_socket, "GET_HISTORY");
    } else {
        // Вычисляем выражение
        double result = evaluate_expression(command);
        if (!std::isnan(result)) {
            std::cout << "Result: " << result << std::endl;

            // Отправляем выражение серверу
            send_request(client_socket, command);
        }
    }

    close(client_socket);
    return 0;
}
