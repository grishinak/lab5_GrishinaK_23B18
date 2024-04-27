cmake_minimum_required(VERSION 3.10)
project(calculator_server_client)

# Настройка пакетов Boost и SQLite3
find_package(Boost 1.65 REQUIRED COMPONENTS system)
find_library(SQLITE3_LIBRARY sqlite3)

# Сборка сервера
add_executable(server server.cpp)
target_link_libraries(server PRIVATE ${Boost_LIBRARIES} ${SQLITE3_LIBRARY})

# Сборка клиента
add_executable(client client.cpp)
target_link_libraries(client PRIVATE ${Boost_LIBRARIES})

# Компиляция и сборка базы данных SQLite
add_executable(database database.cpp)
target_link_libraries(database PRIVATE ${SQLITE3_LIBRARY})
