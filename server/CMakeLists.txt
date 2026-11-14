cmake_minimum_required(VERSION 3.19)

set(PROJECT_NAME server)

project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)

# The authority. Links the rules and the wire format and nothing else: no
# window, no renderer, no ImGui, so it builds and runs on a headless box.
add_library(server_core STATIC
        src/Match.cpp
        src/Server.cpp)

target_include_directories(server_core PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(server_core PUBLIC GosChess::chess GosChess::protocol)

target_compile_features(server_core PUBLIC cxx_std_20)

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE server_core)

# The match logic is separated from the socket loop so that a game can be
# driven to checkmate, stalemate and timeout in a test without a network.
add_executable(server_selftest tests/server_selftest.cpp)

target_link_libraries(server_selftest PRIVATE server_core)

add_test(NAME server_selftest COMMAND server_selftest)

# Drives the real server over real sockets, stepping it by hand so the test
# stays deterministic instead of racing a background thread.
add_executable(server_e2e_selftest tests/e2e_selftest.cpp)

target_link_libraries(server_e2e_selftest PRIVATE server_core)

add_test(NAME server_e2e_selftest COMMAND server_e2e_selftest)
