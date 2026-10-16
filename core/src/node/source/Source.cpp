// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#include "node/source/Source.hpp"

#include <cstddef>
#include <string>
#include <utility>

namespace Pento
{
    static constexpr std::size_t INPUT_COUNT_0 = 0;
    static constexpr std::size_t OUTPUT_COUNT_1 = 1;
    static constexpr std::size_t OUTPUT_INDEX_0 = 0;

    Source::Source(std::string name)
        : Node(std::move(name), INPUT_COUNT_0, OUTPUT_COUNT_1)
    {
    }

    Status Source::CheckConnections() const
    {
        if (!IsOutputConnected(OUTPUT_INDEX_0))
            return Fail(Status::InvalidOperation, "output is not connected");

        return Status::Ok;
    }

}  // namespace Pento
