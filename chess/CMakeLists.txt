cmake_minimum_required(VERSION 3.15)

set(PROJECT_NAME chess)

project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)

# The rules engine. Deliberately free of SFML, of any windowing or rendering
# dependency, and of global game state, so that the same code can run inside
# the GUI client and inside a headless server handling many games at once.
add_library(${PROJECT_NAME} STATIC
        src/Board.cpp
        src/Move.cpp
        src/Rules.cpp
        src/GameState.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

add_library(GosChess::chess ALIAS ${PROJECT_NAME})

# A plain assert-driven binary rather than a test framework: it has to build
# and run anywhere the server does, with no extra dependency to install.
add_executable(chess_selftest tests/selftest.cpp)

target_link_libraries(chess_selftest PRIVATE ${PROJECT_NAME})

add_test(NAME chess_selftest COMMAND chess_selftest)
