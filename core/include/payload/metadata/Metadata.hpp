// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#ifndef PENTO_CORE_PAYLOAD_METADATA_METADATA_HPP
#define PENTO_CORE_PAYLOAD_METADATA_METADATA_HPP

#include <memory>

namespace Pento
{

    // Base class of everything a node attaches to a Payload, such as detected
    // boxes. Each kind is a subclass in this directory.
    class Metadata
    {
      public:
        using SPtr = std::shared_ptr<Metadata>;
        using UPtr = std::unique_ptr<Metadata>;

        virtual ~Metadata() = default;
    };

}  // namespace Pento

#endif  // PENTO_CORE_PAYLOAD_METADATA_METADATA_HPP
