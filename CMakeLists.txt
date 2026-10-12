# SPDX-License-Identifier: MIT
# Copyright (c) 2026 SoonTheanSiew

cmake_minimum_required(VERSION 3.24)
project(pentomino VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# All executables end up in build/bin, whichever module they come from.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(PENTO_BUILD_TESTS "Build the unit tests" ON)

# --- JSON library ---------------------------------------------------------
# Use an installed nlohmann_json if there is one, otherwise fetch a pinned
# release.
find_package(nlohmann_json 3.11 QUIET)
if(NOT nlohmann_json_FOUND)
    include(FetchContent)
    set(JSON_BuildTests OFF CACHE INTERNAL "")
    set(JSON_Install OFF CACHE INTERNAL "")
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()

# --- warnings -------------------------------------------------------------
add_library(pento_warnings INTERFACE)
if(MSVC)
    target_compile_options(pento_warnings INTERFACE /W4)
else()
    target_compile_options(
        pento_warnings
        INTERFACE -Wall -Wextra -Wpedantic -Wshadow
    )
endif()

# --- components -----------------------------------------------------------
# core is the runtime that generated programs link. generator (model, loader,
# optimizer, emitter) turns a graph file into C++, and cli runs it. The
# generator does not link core, and core includes nothing from generator.
# Every component keeps its public headers in <component>/include/ and its
# sources in <component>/src/, with the same subpaths below those two roots.
# A header is included by its path below <component>/include/, for example
# #include "model/Graph.hpp".

add_subdirectory(core)
add_subdirectory(generator)
add_subdirectory(cli)

if(PENTO_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
