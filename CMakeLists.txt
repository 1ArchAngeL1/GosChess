
cmake_minimum_required(VERSION 3.15)

set(PROJECT_NAME GosChess)

project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(client)

add_subdirectory(chess)

add_subdirectory(server)