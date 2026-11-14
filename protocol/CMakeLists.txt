cmake_minimum_required(VERSION 3.19)

set(PROJECT_NAME protocol)

project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)

# The wire format. Linked by both the client and the server so that there is
# exactly one definition of what goes over the socket; the previous build had
# the packet operators inside the client, which left nothing to stop the two
# ends from drifting apart.
#
# Depends on sfml-network for sf::Packet - which supplies the length prefixed
# framing - and on the chess library for color_t, but on nothing that draws.
add_library(${PROJECT_NAME} STATIC
        src/Protocol.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(${PROJECT_NAME} PUBLIC GosChess::chess sfml-network sfml-system)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

add_library(GosChess::protocol ALIAS ${PROJECT_NAME})

add_executable(protocol_selftest tests/protocol_selftest.cpp)

target_link_libraries(protocol_selftest PRIVATE ${PROJECT_NAME})

add_test(NAME protocol_selftest COMMAND protocol_selftest)
