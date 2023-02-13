

cmake_minimum_required(VERSION 3.15)

set(PROJECT_NAME client)

project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)


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

if (DEBUG_BUILD)
    set(ImGui-SFML_DIR "${CMAKE_SOURCE_DIR}/libs/imgui-sfml/debug/lib/cmake/ImGui-SFML")
else()
    set(ImGui-SFML_DIR "${CMAKE_SOURCE_DIR}/libs/imgui-sfml/release/lib/cmake/ImGui-SFML")
endif()

find_package(ImGui-SFML REQUIRED)

set(IMGUI_SFML_LIBS ImGui-SFML::ImGui-SFML)

file(
        GLOB_RECURSE SOURCE_FILES
        ../client/*.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} ${IMGUI_SFML_LIBS} ${SFML_LIBS})


