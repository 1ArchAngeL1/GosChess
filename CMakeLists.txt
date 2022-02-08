#cmake_minimum_required(VERSION 3.15)
#
#set(PROJECT_NAME GosChess)
#
#project(${PROJECT_NAME})
#
#set(CMAKE_CXX_STANDARD 20)
#
#set(GCC_COVERAGE_COMPILE_FLAGS "-I/opt/homebrew/Cellar/sfml/2.5.1_1/include -L/opt/homebrew/Cellar/sfml/2.5.1_1/lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network")
#
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
#file (
#        GLOB_RECURSE SOURCE_FILES
#        src/*.cpp
#)
#add_executable(${PROJECT_NAME} ${SOURCE_FILES})


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


