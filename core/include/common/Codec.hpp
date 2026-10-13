// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#ifndef PENTO_CORE_COMMON_CODEC_HPP
#define PENTO_CORE_COMMON_CODEC_HPP

namespace Pento
{

    enum class Codec
    {
        Unknown,  // bytes not split into frames yet, such as a container
        H264,
        Hevc,
        Av1
    };

}  // namespace Pento

#endif  // PENTO_CORE_COMMON_CODEC_HPP
