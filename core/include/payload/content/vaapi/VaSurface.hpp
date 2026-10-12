// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#ifndef PENTO_CORE_PAYLOAD_CONTENT_VAAPI_VA_SURFACE_HPP
#define PENTO_CORE_PAYLOAD_CONTENT_VAAPI_VA_SURFACE_HPP

#include <cstdint>
#include <memory>

namespace Pento
{

// A raw frame in a VA surface. The surface id is only valid within its
// display, so a node with another display imports it through a dmabuf.
struct VaSurface
{
    using SPtr = std::shared_ptr<const VaSurface>;

    void *display = nullptr;    // VADisplay
    std::uint32_t surface = 0;  // VASurfaceID
    std::uint32_t fourcc = 0;   // VA fourcc
    std::uint32_t width = 0;
    std::uint32_t height = 0;
};

}  // namespace Pento

#endif  // PENTO_CORE_PAYLOAD_CONTENT_VAAPI_VA_SURFACE_HPP
