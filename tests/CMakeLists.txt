# SPDX-License-Identifier: MIT
# Copyright (c) 2026 SoonTheanSiew

# --- GoogleTest -----------------------------------------------------------
# Use an installed GoogleTest if there is one, otherwise fetch a pinned
# release.
find_package(GTest 1.14 QUIET)
if(NOT GTest_FOUND)
    include(FetchContent)
    set(INSTALL_GTEST OFF CACHE INTERNAL "")
    set(gtest_force_shared_crt ON CACHE INTERNAL "")
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.15.2
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(googletest)
endif()

add_executable(
    pento_tests
    core/common/TestLogging.cpp
    core/graph/TestEdge.cpp
    core/graph/TestGraph.cpp
    core/node/TestNode.cpp
    core/payload/TestPayload.cpp
    core/support/DummyNodes.cpp
)
target_include_directories(pento_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/core)
target_link_libraries(
    pento_tests
    PRIVATE pento_core pento_warnings GTest::gtest_main
)

if(TARGET pento_ffmpeg)
    target_sources(
        pento_tests
        PRIVATE core/node/source/ffmpeg/TestFfmpegFileSource.cpp
    )
    target_link_libraries(pento_tests PRIVATE pento_ffmpeg)
endif()

include(GoogleTest)
gtest_discover_tests(pento_tests)
