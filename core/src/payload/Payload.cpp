// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#include "payload/Payload.hpp"

#include <utility>

namespace Pento
{

Payload::Payload(std::chrono::nanoseconds timestamp, Content content)
    : m_timestamp(timestamp), m_content(std::move(content))
{
}

std::chrono::nanoseconds Payload::Timestamp() const
{
    return m_timestamp;
}

const Content &Payload::GetContent() const
{
    return m_content;
}

std::shared_ptr<const Payload> Payload::Attach(
    std::shared_ptr<const Metadata> metadata) const
{
    // The copy brings the timestamp, content and existing metadata along.
    auto copy = std::make_shared<Payload>(*this);
    copy->m_metadata.push_back(std::move(metadata));
    return copy;
}

std::shared_ptr<const Payload> Payload::WithContent(Content content) const
{
    // The copy brings the timestamp and all metadata along.
    auto copy = std::make_shared<Payload>(*this);
    copy->m_content = std::move(content);
    return copy;
}

}  // namespace Pento
