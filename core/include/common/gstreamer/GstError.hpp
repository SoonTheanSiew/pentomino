// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#ifndef PENTO_CORE_COMMON_GSTREAMER_GST_ERROR_HPP
#define PENTO_CORE_COMMON_GSTREAMER_GST_ERROR_HPP

#include "common/Status.hpp"

#include <string>

#include <gst/gst.h>

namespace Pento
{
    // The status of a GStreamer step and, for an error, its message, which
    // the node passes to Fail().
    struct GstResult
    {
        Status status = Status::Ok;
        std::string message;
    };

    // Turns the GError of any GStreamer call or bus message into a Status.
    Status GstToStatus(const GError *error);

    // Initializes GStreamer.
    GstResult GstInitialize();

    // Takes the error posted on the pipeline's bus.
    GstResult GstPopBusError(GstElement *pipeline);

}  // namespace Pento

#endif  // PENTO_CORE_COMMON_GSTREAMER_GST_ERROR_HPP
