cmake_minimum_required(VERSION 3.0)
project(math_server)

set(CMAKE_CXX_STANDARD 11)

find_package(SQLite3 REQUIRED)

add_executable(server server.cpp)
target_link_libraries(server SQLite::SQLite3)

add_executable(client client.cpp)
