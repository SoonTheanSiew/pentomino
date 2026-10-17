// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#ifndef PENTO_CORE_PAYLOAD_CONTENT_SYSTEM_BITSTREAM_HPP
#define PENTO_CORE_PAYLOAD_CONTENT_SYSTEM_BITSTREAM_HPP

#include "common/Codec.hpp"

#include <chrono>
#include <cstdint>
#include <memory>
#include <vector>

namespace Pento
{

    // Encoded bytes in CPU memory, owned by the Bitstream: a chunk a source
    // reads, or one frame a demuxer splits out.
    struct Bitstream
    {
        using SPtr = std::shared_ptr<const Bitstream>;

        std::vector<std::uint8_t> bytes;
        // True when bytes is exactly one frame. The fields below only mean
        // something then.
        bool isCompleteFrame = false;
        Codec codec = Codec::Unknown;
        // Decode order. The presentation time is the Payload's timestamp.
        std::chrono::nanoseconds decodeTime{0};
        bool isKeyFrame = false;
    };

}  // namespace Pento

#endif  // PENTO_CORE_PAYLOAD_CONTENT_SYSTEM_BITSTREAM_HPP
