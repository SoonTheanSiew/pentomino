# SPDX-License-Identifier: MIT
# Copyright (c) 2026 SoonTheanSiew

add_library(pento_core STATIC src/payload/Payload.cpp)
target_include_directories(
    pento_core
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(pento_core PRIVATE pento_warnings)
