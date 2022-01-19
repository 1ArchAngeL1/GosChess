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
        src/exec/main.cpp
)
add_executable(${PROJECT_NAME} ${SOURCE_FILES} src/global/MainGameObjects/GameObjects.cpp src/global/MainGameObjects/GameObjects.h src/computation/GamePlayFunctional.cpp src/computation/GamePlayFunctional.h src/render/GameDraw.cpp src/render/GameDraw.h src/global/GameGlobals.cpp src/global/GameGlobals.h src/util/GameInput.cpp src/util/GameInput.h)