// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#ifndef PENTO_CORE_COMMON_GSTREAMER_GST_CONTENT_HPP
#define PENTO_CORE_COMMON_GSTREAMER_GST_CONTENT_HPP

#include "common/gstreamer/GstError.hpp"
#include "payload/content/Content.hpp"
#include "payload/content/system/Bitstream.hpp"

#include <gst/gst.h>

namespace Pento
{
    // Turns a sample from appsink into content. Encoded bytes are copied
    // into a Bitstream.
    GstResult GstSampleToContent(GstSample *sample, Content &content);

}  // namespace Pento

#endif  // PENTO_CORE_COMMON_GSTREAMER_GST_CONTENT_HPP
