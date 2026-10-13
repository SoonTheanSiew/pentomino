// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#ifndef PENTO_CORE_PAYLOAD_CONTENT_DMABUF_DMA_BUF_HPP
#define PENTO_CORE_PAYLOAD_CONTENT_DMABUF_DMA_BUF_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>

#include "common/Planes.hpp"

namespace Pento
{

    // Where one plane sits inside the dmabuf.
    struct DmaBufPlane
    {
        std::uint32_t offset = 0;
        std::uint32_t pitch = 0;  // bytes per row
    };

    // A raw frame in one dmabuf, with its planes at different offsets.
    struct DmaBuf
    {
        using SPtr = std::shared_ptr<const DmaBuf>;

        int fd = -1;
        std::size_t size = 0;        // of the whole dmabuf
        std::uint32_t fourcc = 0;    // DRM fourcc
        std::uint64_t modifier = 0;  // DRM format modifier, 0 is linear
        std::uint32_t width = 0;
        std::uint32_t height = 0;
        std::size_t planeCount = 0;
        std::array<DmaBufPlane, MAX_PLANES> planes{};
    };

}  // namespace Pento

#endif  // PENTO_CORE_PAYLOAD_CONTENT_DMABUF_DMA_BUF_HPP
