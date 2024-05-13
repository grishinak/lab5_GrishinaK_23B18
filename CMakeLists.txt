cmake_minimum_required(VERSION 3.0)
project(math_server)

set(CMAKE_CXX_STANDARD 11)

find_package(SQLite3 REQUIRED)

#Сервер
add_executable(server src/server.cpp)
target_link_libraries(server SQLite::SQLite3)

#Клиент
add_executable(client src/client.cpp)

#Тесты
add_executable(registration_test test/registration_test.cpp)
add_executable(calculation_test test/calculation_test.cpp)