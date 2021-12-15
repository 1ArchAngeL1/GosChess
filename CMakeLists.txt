cmake_minimum_required(VERSION 3.15)
# THIS HAS TO COME BEFORE THE PROJECT LINE

set(PROJECT_NAME GosChess)
project(PROJECT_NAME)
set(CMAKE_CXX_STANDARD 14)

set(GCC_COVERAGE_COMPILE_FLAGS "-I/opt/homebrew/Cellar/sfml/2.5.1_1/include -L/opt/homebrew/Cellar/sfml/2.5.1_1/lib -lsfml-graphics -lsfml-window -lsfml-system")
# -Ofast -pthread
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
file (
        GLOB_RECURSE SOURCE_FILES
        exec/main.cpp
)
add_executable(${PROJECT_NAME} ${SOURCE_FILES} computation/GameObjects.cpp computation/GameObjects.h computation/GameInitFunctional.cpp computation/GameInitFunctional.h render/GameDraw.cpp render/GameDraw.h global/BoardAtributtes.cpp global/BoardAtributtes.h)