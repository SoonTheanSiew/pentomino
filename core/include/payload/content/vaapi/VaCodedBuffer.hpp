// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#ifndef PENTO_CORE_PAYLOAD_CONTENT_VAAPI_VA_CODED_BUFFER_HPP
#define PENTO_CORE_PAYLOAD_CONTENT_VAAPI_VA_CODED_BUFFER_HPP

#include "common/Codec.hpp"

#include <chrono>
#include <cstdint>
#include <memory>

namespace Pento
{

    // One encoded frame in a VA coded buffer (VAEncCodedBufferType), as a VA
    // encoder writes it. Read the bytes with vaMapBuffer().
    struct VaCodedBuffer
    {
        using SPtr = std::shared_ptr<const VaCodedBuffer>;

        void *display = nullptr;   // VADisplay
        std::uint32_t buffer = 0;  // VABufferID
        Codec codec = Codec::Unknown;
        // Decode order. The presentation time is the Payload's timestamp.
        std::chrono::nanoseconds decodeTime{0};
        bool isKeyFrame = false;
    };

}  // namespace Pento

#endif  // PENTO_CORE_PAYLOAD_CONTENT_VAAPI_VA_CODED_BUFFER_HPP
