
cmake_minimum_required(VERSION 3.15)

set(PROJECT_NAME GosChess)

project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)

find_package(SFML COMPONENTS system window graphics network audio)

set(ImGui-SFML_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/sfml-imgui/lib/cmake/ImGui-SFML")

find_package(ImGui-SFML REQUIRED)

set(IMGUI_SFML_LIBS ImGui-SFML::ImGui-SFML)

file(
        GLOB_RECURSE SOURCE_FILES
        src/*.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} sfml-graphics sfml-window sfml-system sfml-network)

target_link_libraries(${PROJECT_NAME} ${IMGUI_SFML_LIBS})


