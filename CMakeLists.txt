
cmake_minimum_required(VERSION 3.19)

set(PROJECT_NAME GosChess)

project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)

# ctest is enabled at the top so that `ctest` run from one build directory
# picks up the checks defined in every subproject.
enable_testing()

# ---------------------------------------------------------------------------
# SFML
#
# Resolved here rather than inside the client because the server needs the
# network and system modules too, and a dependency found twice in two
# subdirectories is a dependency that can be found at two different versions.
# The client adds Dear ImGui on top of this for itself.
#
# libs/sfml is a set of prebuilt *Windows* (MinGW) binaries: the archives
# contain COFF .obj members and the CMake package hard-codes the Win32 system
# libraries (gdi32, winmm, ws2_32, opengl32, glu32, imm32). They can only be
# used when the target platform is Windows; everywhere else SFML is taken from
# the system or built from source.
# ---------------------------------------------------------------------------

# audio is included for the move and interface sounds, which are synthesised
# at run time rather than shipped as files.
set(SFML_COMPONENTS system window graphics network audio)

if (WIN32)

    set(SFML_ROOT "${CMAKE_SOURCE_DIR}/libs/sfml")

    set(SFML_DIR "${SFML_ROOT}/lib/cmake/SFML")

    set(SFML_STATIC_LIBRARIES TRUE)

    find_package(SFML COMPONENTS ${SFML_COMPONENTS} REQUIRED)

else ()

    # libs/sfml holds Windows binaries, and find_package will happily reuse a
    # cached SFML_DIR pointing at them on any platform. The result is a link
    # against .dll files and an error - "unknown file type" - that says nothing
    # about the actual cause. A build directory configured once with that value
    # keeps failing until it is deleted, so the value is rejected here instead.
    if (SFML_DIR AND SFML_DIR MATCHES "^${CMAKE_SOURCE_DIR}/libs/sfml")
        message(WARNING
                "SFML_DIR points at the bundled Windows SFML in libs/sfml, which cannot be "
                "linked on this platform. Ignoring it and looking for SFML elsewhere.")
        unset(SFML_DIR CACHE)
    endif ()

    # An already installed SFML 2 is used when present (e.g. `brew install sfml@2`),
    # otherwise it is fetched and built from source. SFML 3 is not API compatible
    # with this code base, so the major version is pinned.
    find_package(SFML 2 COMPONENTS ${SFML_COMPONENTS} CONFIG QUIET
            HINTS /opt/homebrew/opt/sfml@2 /usr/local/opt/sfml@2)

    if (NOT SFML_FOUND)
        message(STATUS "SFML 2 not installed - building it from source")
        include(FetchContent)
        set(BUILD_SHARED_LIBS OFF)
        set(SFML_BUILD_AUDIO ON)
        set(SFML_BUILD_EXAMPLES OFF)
        set(SFML_BUILD_DOC OFF)
        FetchContent_Declare(SFML
                GIT_REPOSITORY https://github.com/SFML/SFML.git
                GIT_TAG 2.6.2
                GIT_SHALLOW TRUE)
        FetchContent_MakeAvailable(SFML)
    endif ()

endif ()

list(TRANSFORM SFML_COMPONENTS PREPEND sfml- OUTPUT_VARIABLE SFML_LIBS)

# The server has no business linking a window or a renderer.
set(SFML_NET_LIBS sfml-network sfml-system)

# ---------------------------------------------------------------------------
# Subprojects, dependencies first
# ---------------------------------------------------------------------------

# The rules engine: no dependencies of its own.
add_subdirectory(chess)

# The wire format, shared so that the client and the server cannot drift.
add_subdirectory(protocol)

# The authority.
add_subdirectory(server)

# The window.
add_subdirectory(client)
