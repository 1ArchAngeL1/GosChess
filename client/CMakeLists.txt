cmake_minimum_required(VERSION 3.19)

set(PROJECT_NAME client)

project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)

# ---------------------------------------------------------------------------
# Dear ImGui
#
# SFML itself is resolved in the top level CMakeLists, since the server needs
# it too. Only the client needs ImGui, so it is built here.
#
# libs/imgui-sfml holds prebuilt Windows (MinGW) binaries and can only be used
# when the target platform is Windows; elsewhere Dear ImGui and its SFML
# backend are compiled from source.
# ---------------------------------------------------------------------------

if (WIN32)

    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(ImGui-SFML_DIR "${CMAKE_SOURCE_DIR}/libs/imgui-sfml/debug/lib/cmake/ImGui-SFML")
    else ()
        set(ImGui-SFML_DIR "${CMAKE_SOURCE_DIR}/libs/imgui-sfml/release/lib/cmake/ImGui-SFML")
    endif ()

    find_package(ImGui-SFML REQUIRED)

    set(IMGUI_SFML_LIBS ImGui-SFML::ImGui-SFML)

else ()

    include(FetchContent)

    # ImGui-SFML is not packaged anywhere, so Dear ImGui and the SFML backend are
    # compiled straight from their sources into one static library. Doing it here
    # rather than through ImGui-SFML's own CMake keeps the two versions pinned
    # together and avoids its install/export machinery.
    FetchContent_Declare(imgui
            GIT_REPOSITORY https://github.com/ocornut/imgui.git
            GIT_TAG v1.89.9
            GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(imgui)

    FetchContent_Populate(imgui_sfml
            QUIET
            GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
            GIT_TAG v2.6.1
            GIT_SHALLOW TRUE
            SOURCE_DIR "${CMAKE_BINARY_DIR}/_deps/imgui_sfml-src"
            BINARY_DIR "${CMAKE_BINARY_DIR}/_deps/imgui_sfml-build"
            SUBBUILD_DIR "${CMAKE_BINARY_DIR}/_deps/imgui_sfml-subbuild")

    find_package(OpenGL REQUIRED)

    add_library(imgui-sfml STATIC
            "${imgui_SOURCE_DIR}/imgui.cpp"
            "${imgui_SOURCE_DIR}/imgui_draw.cpp"
            "${imgui_SOURCE_DIR}/imgui_tables.cpp"
            "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
            "${imgui_sfml_SOURCE_DIR}/imgui-SFML.cpp")

    target_include_directories(imgui-sfml PUBLIC
            "${imgui_SOURCE_DIR}"
            "${imgui_sfml_SOURCE_DIR}")

    target_compile_definitions(imgui-sfml PUBLIC IMGUI_USER_CONFIG="imconfig-SFML.h")

    target_link_libraries(imgui-sfml PUBLIC
            sfml-graphics sfml-window sfml-system OpenGL::GL)

    set(IMGUI_SFML_LIBS imgui-sfml)

endif ()

# ---------------------------------------------------------------------------
# Target
# ---------------------------------------------------------------------------

file(
        GLOB_RECURSE SOURCE_FILES
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# tests/ carries its own main() and is built as a separate target below.
list(FILTER SOURCE_FILES EXCLUDE REGEX "${CMAKE_CURRENT_SOURCE_DIR}/tests/")

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# The chess rules come from the shared library rather than from a copy inside
# the client, so the client and the server can never disagree about them.
target_link_libraries(${PROJECT_NAME} GosChess::chess GosChess::protocol ${IMGUI_SFML_LIBS} ${SFML_LIBS})

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

# Assets are loaded through this prefix instead of an absolute path baked into
# the sources, so the build is not tied to one machine.
target_compile_definitions(${PROJECT_NAME} PRIVATE
        GOSCHESS_RESOURCES="${CMAKE_SOURCE_DIR}/resources")

# ---------------------------------------------------------------------------
# Tests
#
# The board orientation maths is deliberately SFML free (Render/BoardView.*),
# so it can be checked here without opening a window or needing a display.
# ---------------------------------------------------------------------------

add_executable(client_view_selftest
        tests/view_selftest.cpp
        Render/BoardView.cpp
        Game/ClientGame.cpp)

# ClientGame carries the server-fed state, so the wire format comes with it.
target_link_libraries(client_view_selftest PRIVATE GosChess::chess GosChess::protocol)

target_include_directories(client_view_selftest PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

add_test(NAME client_view_selftest COMMAND client_view_selftest)
