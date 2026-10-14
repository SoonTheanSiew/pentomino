// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#ifndef PENTO_CORE_PAYLOAD_CONTENT_CONTENT_HPP
#define PENTO_CORE_PAYLOAD_CONTENT_CONTENT_HPP

#include "payload/content/dmabuf/DmaBuf.hpp"
#include "payload/content/system/Bitstream.hpp"
#include "payload/content/system/SystemFrame.hpp"
#include "payload/content/vaapi/VaCodedBuffer.hpp"
#include "payload/content/vaapi/VaSurface.hpp"

#include <variant>

namespace Pento
{

    // One of the types in the subdirectories here, held by a shared_ptr.
    // When the last Payload lets go, the deleter gives the memory back.
    using Content =
        std::variant<Bitstream::SPtr, VaCodedBuffer::SPtr, SystemFrame::SPtr,
                     DmaBuf::SPtr, VaSurface::SPtr>;

}  // namespace Pento

#endif  // PENTO_CORE_PAYLOAD_CONTENT_CONTENT_HPP
