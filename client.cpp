#include <iostream>
#include <boost/asio.hpp>

using namespace boost::asio;
using ip::tcp;

int main() {
    try {
        io_service ioService;
        tcp::socket socket(ioService);
        socket.connect(tcp::endpoint(ip::address::from_string("127.0.0.1"), 1234));

        // Ввод пользовательского выражения
        std::string expression;
        std::cout << "Enter an arithmetic expression: ";
        std::getline(std::cin, expression);

        // Отправка выражения серверу
        write(socket, buffer(expression));

        // Получение и вывод результата от сервера
        char reply[1024];
        size_t replyLength = socket.read_some(buffer(reply));
        std::cout << "Result: " << std::string(reply, replyLength) << std::endl;
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
