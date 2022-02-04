cmake_minimum_required(VERSION 3.15)
# THIS HAS TO COME BEFORE THE PROJECT LINE

set(PROJECT_NAME GosChess)
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)

set(GCC_COVERAGE_COMPILE_FLAGS "-I/opt/homebrew/Cellar/sfml/2.5.1_1/include -L/opt/homebrew/Cellar/sfml/2.5.1_1/lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network")
# -Ofast -pthread
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
file (
        GLOB_RECURSE SOURCE_FILES
        src/*.cpp
)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})


#cmake_minimum_required(VERSION 3.15)
#
#
#set(PROJECT_NAME GosChess)
#
#project(PROJECT_NAME)
#
#set(CMAKE_CXX_STANDARD 17)
#
#set(SFML_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/external_libraries/SFML")
#
#
#set(CMAKE_MODULE_PATH "${SFML_ROOT}/build/src")
#
#set(SFML_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external_libraries/SFML/include")
#set(SFML_LIBRARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external_libraries/SFML/build/lib")
#
#include_directories(${SFML_INCLUDE_DIR})
#link_directories(${SFML_LIBRARY_DIR})
#
#find_package(SFML COMPONENTS system window graphics network audio)
#
#
#file (
#        GLOB_RECURSE SOURCE_FILES
#        src/*.cpp
#)
#
#add_executable(${PROJECT_NAME} ${SOURCE_FILES})
#
#target_link_libraries (${PROJECT_NAME} ${SFML_LIBRARY_DIR}/libsfml-audio.dylib
#        ${SFML_LIBRARY_DIR}/libsfml-system.dylib
#        ${SFML_LIBRARY_DIR}/libsfml-graphics.dylib
#        ${SFML_LIBRARY_DIR}/libsfml-network.dylib)
