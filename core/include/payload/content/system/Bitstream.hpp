// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#ifndef PENTO_CORE_PAYLOAD_CONTENT_SYSTEM_BITSTREAM_HPP
#define PENTO_CORE_PAYLOAD_CONTENT_SYSTEM_BITSTREAM_HPP

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>

#include "common/Codec.hpp"

namespace Pento
{

// Encoded bytes in CPU memory: a chunk a source reads, or one frame a demuxer
// splits out.
struct Bitstream
{
    using SPtr = std::shared_ptr<const Bitstream>;

    const std::uint8_t *data = nullptr;
    std::size_t size = 0;
    // True when data is exactly one frame. The fields below only mean
    // something then.
    bool isCompleteFrame = false;
    Codec codec = Codec::Unknown;
    // Decode order. The presentation time is the Payload's timestamp.
    std::chrono::nanoseconds decodeTime{0};
    bool isKeyFrame = false;
};

}  // namespace Pento

#endif  // PENTO_CORE_PAYLOAD_CONTENT_SYSTEM_BITSTREAM_HPP
