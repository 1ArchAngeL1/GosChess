
cmake_minimum_required(VERSION 3.15)

set(PROJECT_NAME fake_client)

set(CMAKE_CXX_STANDARD 20)

project(${PROJECT_NAME})

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(DEBUG_BUILD TRUE)
else()
    set(DEBUG_BUILD FALSE)
endif()

set(SFML_ROOT "${CMAKE_SOURCE_DIR}/libs/sfml")

set(CMAKE_MODULE_PATH "${SFML_ROOT}/lib/cmake")

set(SFML_STATIC_LIBRARIES TRUE)

find_package(SFML COMPONENTS system window graphics network audio REQUIRED)

set(SFML_LIBS sfml-system sfml-window sfml-graphics sfml-network sfml-audio)


file(
        GLOB_RECURSE SOURCE_FILES
        *.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} ${SFML_LIBS})


