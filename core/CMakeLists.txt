# SPDX-License-Identifier: MIT
# Copyright (c) 2026 SoonTheanSiew

# --- logging --------------------------------------------------------------
# Use an installed spdlog if there is one, otherwise fetch a pinned release.
# PUBLIC, because Logging.hpp includes it.
find_package(spdlog 1.12 QUIET)
if(NOT spdlog_FOUND)
    include(FetchContent)
    set(SPDLOG_SHA256
        15a04e69c222eb6c01094b5c7ff8a249b36bb22788d72519646fb85feb267e67
    )
    FetchContent_Declare(
        spdlog
        URL https://github.com/gabime/spdlog/archive/refs/tags/v1.15.3.tar.gz
        URL_HASH SHA256=${SPDLOG_SHA256}
    )
    FetchContent_MakeAvailable(spdlog)
endif()

add_library(
    pento_core
    STATIC
    src/common/Logging.cpp
    src/graph/Edge.cpp
    src/graph/Graph.cpp
    src/node/Node.cpp
    src/node/source/Source.cpp
    src/payload/Payload.cpp
)
target_include_directories(
    pento_core
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(pento_core PUBLIC spdlog::spdlog PRIVATE pento_warnings)

# --- framework nodes ------------------------------------------------------
# The nodes of a framework build only when its development files are found.
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(FFMPEG QUIET IMPORTED_TARGET libavformat libavutil)
endif()

if(FFMPEG_FOUND)
    add_library(
        pento_ffmpeg
        STATIC
        src/common/ffmpeg/FfmpegError.cpp
        src/node/source/ffmpeg/FfmpegFileSource.cpp
    )
    target_link_libraries(
        pento_ffmpeg
        PUBLIC pento_core PkgConfig::FFMPEG
        PRIVATE pento_warnings
    )
else()
    message(STATUS "FFmpeg not found, its nodes are not built")
endif()
