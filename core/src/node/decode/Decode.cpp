// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#include "node/decode/Decode.hpp"

#include <cstddef>
#include <string>
#include <utility>

namespace Pento
{
    static constexpr std::size_t INPUT_COUNT_1 = 1;
    static constexpr std::size_t OUTPUT_COUNT_1 = 1;
    static constexpr std::size_t INPUT_INDEX_0 = 0;
    static constexpr std::size_t OUTPUT_INDEX_0 = 0;

    Decode::Decode(std::string name, Config config)
        : Node(std::move(name), INPUT_COUNT_1, OUTPUT_COUNT_1), m_config(config)
    {
    }

    Status Decode::CheckConnections() const
    {
        if (!IsInputConnected(INPUT_INDEX_0))
            return Fail(Status::InvalidOperation, "input is not connected");

        if (!IsOutputConnected(OUTPUT_INDEX_0))
            return Fail(Status::InvalidOperation, "output is not connected");

        return Status::Ok;
    }

    const Decode::Config &Decode::GetConfig() const
    {
        return m_config;
    }

}  // namespace Pento
