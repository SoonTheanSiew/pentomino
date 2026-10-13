// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#ifndef PENTO_CORE_COMMON_PLANES_HPP
#define PENTO_CORE_COMMON_PLANES_HPP

#include <cstddef>

namespace Pento
{

    // Most formats need one to three planes, such as Y and UV for NV12.
    inline constexpr std::size_t MAX_PLANES = 4;

}  // namespace Pento

#endif  // PENTO_CORE_COMMON_PLANES_HPP
