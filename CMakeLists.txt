cmake_minimum_required(VERSION 3.20)
project(breakout LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# --- raylib -------------------------------------------------------------------
include(FetchContent)

# raylib 5.5 declares an old cmake_minimum_required; CMake 4.x refuses
# (or warns about) those unless a minimum policy version is provided.
if(NOT DEFINED CMAKE_POLICY_VERSION_MINIMUM)
    set(CMAKE_POLICY_VERSION_MINIMUM 3.10)
endif()

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)
if(EMSCRIPTEN)
    set(PLATFORM Web CACHE STRING "" FORCE)
endif()

FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/5.5.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(raylib)

# raylib's vendored libraries (miniaudio, stb) warn under newer compilers.
# That code isn't ours, so silence it; our own target keeps -Wall -Wextra.
target_compile_options(raylib PRIVATE -w)

# --- game ---------------------------------------------------------------------
if(EMSCRIPTEN)
    # Emits index.html + index.js + index.wasm.
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

add_executable(breakout
    src/main.cpp
    src/Game.cpp
    src/Paddle.cpp
    src/Ball.cpp
    src/Level.cpp
    src/Particles.cpp
    src/Storage.cpp
)

target_link_libraries(breakout PRIVATE raylib)
target_compile_options(breakout PRIVATE -Wall -Wextra)

if(EMSCRIPTEN)
    set(SHELL_FILE ${CMAKE_SOURCE_DIR}/web/shell.html)
    set_target_properties(breakout PROPERTIES
        OUTPUT_NAME index
        LINK_DEPENDS ${SHELL_FILE}   # relink when the shell page changes
    )
    target_link_options(breakout PRIVATE
        -sUSE_GLFW=3
        -sALLOW_MEMORY_GROWTH=1
        -sGL_ENABLE_GET_PROC_ADDRESS=1   # raylib looks up GL functions at runtime
        --shell-file ${SHELL_FILE}
    )
endif()
