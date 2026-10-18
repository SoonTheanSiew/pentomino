// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#include "common/gstreamer/GstContent.hpp"

#include <memory>

namespace Pento
{
    static GstResult ToBitstream(GstBuffer *buffer, Content &content)
    {
        const gsize size = gst_buffer_get_size(buffer);
        auto bitstream = std::make_shared<Bitstream>();
        bitstream->bytes.resize(size);
        const gsize copied =
            gst_buffer_extract(buffer, 0, bitstream->bytes.data(), size);
        if (copied != size)
            return {Status::IoError, "cannot read a GStreamer buffer"};

        content = bitstream;
        return {.status = Status::Ok};
    }

    GstResult GstSampleToContent(GstSample *sample, Content &content)
    {
        GstBuffer *buffer = gst_sample_get_buffer(sample);
        if (buffer == nullptr)
            return {Status::UnknownError, "the sample has no buffer"};

        return ToBitstream(buffer, content);
    }

}  // namespace Pento
