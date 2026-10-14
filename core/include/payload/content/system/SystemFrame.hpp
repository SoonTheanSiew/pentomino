// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#ifndef PENTO_CORE_PAYLOAD_CONTENT_SYSTEM_SYSTEM_FRAME_HPP
#define PENTO_CORE_PAYLOAD_CONTENT_SYSTEM_SYSTEM_FRAME_HPP

#include "common/Planes.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>

namespace Pento
{

    struct SystemPlane
    {
        const std::uint8_t *data = nullptr;
        std::uint32_t pitch = 0;  // bytes per row
    };

    // A raw frame in CPU memory.
    struct SystemFrame
    {
        using SPtr = std::shared_ptr<const SystemFrame>;

        std::uint32_t fourcc = 0;
        std::uint32_t width = 0;
        std::uint32_t height = 0;
        std::size_t planeCount = 0;
        std::array<SystemPlane, MAX_PLANES> planes{};
    };

}  // namespace Pento

#endif  // PENTO_CORE_PAYLOAD_CONTENT_SYSTEM_SYSTEM_FRAME_HPP
