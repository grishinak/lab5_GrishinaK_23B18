#include <iostream>
#include <string>
#include <thread>
#include <boost/asio.hpp>
#include <sqlite3.h>

using namespace boost::asio;
using ip::tcp;

// Функция для вычисления алгебраического выражения
double calculateExpression(const std::string& expression) {
    // Простая реализация вычисления алгебраического выражения
    // (для простоты примера не учитываются все возможные операции и ошибки)
    return std::stod(expression);
}

// Обработчик клиентских соединений
void clientHandler(tcp::socket socket, sqlite3* db) {
    try {
        // Получаем алгебраическое выражение от клиента
        boost::system::error_code error;
        char data[1024];
        size_t length = socket.read_some(buffer(data), error);
        if (error == error::eof)
            return;
        else if (error)
            throw boost::system::system_error(error);

        std::string expression(data, length);
        // Вычисляем результат выражения
        double result = calculateExpression(expression);

        // Сохраняем запрос и результат в базе данных
        std::string sqlQuery = "INSERT INTO calculations (expression, result) VALUES ('" + expression + "', " + std::to_string(result) + ")";
        sqlite3_exec(db, sqlQuery.c_str(), NULL, 0, NULL);

        // Отправляем результат клиенту
        write(socket, buffer(std::to_string(result)), error);
        if (error)
            throw boost::system::system_error(error);
    }
    catch (std::exception& e) {
        std::cerr << "Exception in thread: " << e.what() << std::endl;
    }
}

int main() {
    // Инициализация базы данных SQLite
    sqlite3* db;
    int rc = sqlite3_open(":memory:", &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    }
    else {
        std::cout << "Opened database successfully" << std::endl;
    }

    // Создание таблицы для сохранения вычислений
    char* errorMsg;
    const char* createTableQuery = "CREATE TABLE calculations (id INTEGER PRIMARY KEY AUTOINCREMENT, expression TEXT, result REAL);";
    rc = sqlite3_exec(db, createTableQuery, NULL, 0, &errorMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errorMsg << std::endl;
        sqlite3_free(errorMsg);
    }
    else {
        std::cout << "Table created successfully" << std::endl;
    }

    // Инициализация сетевого сервера
    io_service ioService;
    tcp::acceptor acceptor(ioService, tcp::endpoint(tcp::v4(), 1234));

    // Запуск бесконечного цикла ожидания и обработки соединений
    while (true) {
        tcp::socket socket(ioService);
        acceptor.accept(socket);
        std::thread(clientHandler, std::move(socket), db).detach();
    }

    // Закрытие базы данных
    sqlite3_close(db);

    return 0;
}
