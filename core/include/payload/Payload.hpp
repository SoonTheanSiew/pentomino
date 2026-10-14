// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#ifndef PENTO_CORE_PAYLOAD_PAYLOAD_HPP
#define PENTO_CORE_PAYLOAD_PAYLOAD_HPP

#include "payload/content/Content.hpp"
#include "payload/metadata/Metadata.hpp"

#include <chrono>
#include <concepts>
#include <memory>
#include <vector>

namespace Pento
{

    // DESIGN NOTES
    // ------------
    // A Payload is what travels along an edge: one Content (a frame, a packet,
    // raw bytes) plus the Metadata attached to it. It never changes once
    // created, because several nodes may read it at once. Attach() and
    // WithContent() return a new Payload instead.

    class Payload
    {
      public:
        using SPtr = std::shared_ptr<Payload>;
        using UPtr = std::unique_ptr<Payload>;

        Payload(std::chrono::nanoseconds timestamp, Content content);

        // Presentation time of the content.
        std::chrono::nanoseconds Timestamp() const;

        // Read it with std::get_if, for example
        // std::get_if<VaSurface::SPtr>(&payload.GetContent()).
        const Content &GetContent() const;

        // The first attached metadata of the given type, or nullptr.
        template <class MetadataType>
            requires std::derived_from<MetadataType, Metadata>
        const MetadataType *Find() const;

        // Returns a copy with `metadata` added. This Payload is not changed.
        [[nodiscard]] std::shared_ptr<const Payload> Attach(
            std::shared_ptr<const Metadata> metadata) const;

        // Returns a copy with `content` in place of the old one. The timestamp
        // and metadata stay.
        [[nodiscard]] std::shared_ptr<const Payload> WithContent(
            Content content) const;

      private:
        std::chrono::nanoseconds m_timestamp;
        Content m_content;
        std::vector<std::shared_ptr<const Metadata>> m_metadata;
    };

    // Find() is a template, so it is defined here, below the class, where every
    // caller can see it.
    template <class MetadataType>
        requires std::derived_from<MetadataType, Metadata>
    const MetadataType *Payload::Find() const
    {
        for (const auto &metadata : m_metadata)
        {
            if (const auto *found =
                    dynamic_cast<const MetadataType *>(metadata.get()))
                return found;
        }
        return nullptr;
    }

}  // namespace Pento

#endif  // PENTO_CORE_PAYLOAD_PAYLOAD_HPP
