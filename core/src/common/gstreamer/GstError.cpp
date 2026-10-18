// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#include "common/gstreamer/GstError.hpp"

#include <string>

namespace Pento
{
    static Status ResourceErrorToStatus(gint code)
    {
        switch (code)
        {
            case GST_RESOURCE_ERROR_NOT_FOUND:
                return Status::FileNotFound;
            case GST_RESOURCE_ERROR_NOT_AUTHORIZED:
                return Status::PermissionDenied;
            case GST_RESOURCE_ERROR_OPEN_READ:
            case GST_RESOURCE_ERROR_OPEN_WRITE:
            case GST_RESOURCE_ERROR_OPEN_READ_WRITE:
            case GST_RESOURCE_ERROR_READ:
            case GST_RESOURCE_ERROR_WRITE:
            case GST_RESOURCE_ERROR_SEEK:
            case GST_RESOURCE_ERROR_NO_SPACE_LEFT:
                return Status::IoError;
            default:
                return Status::UnknownError;
        }
    }

    static Status StreamErrorToStatus(gint code)
    {
        switch (code)
        {
            case GST_STREAM_ERROR_DECODE:
            case GST_STREAM_ERROR_DEMUX:
            case GST_STREAM_ERROR_FORMAT:
                return Status::InvalidInput;
            case GST_STREAM_ERROR_CODEC_NOT_FOUND:
            case GST_STREAM_ERROR_WRONG_TYPE:
                return Status::Unsupported;
            default:
                return Status::UnknownError;
        }
    }

    Status GstToStatus(const GError *error)
    {
        // GQuark domains are not constants, so they cannot be switch cases.
        if (error->domain == GST_RESOURCE_ERROR)
            return ResourceErrorToStatus(error->code);
        if (error->domain == GST_STREAM_ERROR)
            return StreamErrorToStatus(error->code);
        if (error->domain == GST_CORE_ERROR &&
            error->code == GST_CORE_ERROR_MISSING_PLUGIN)
            return Status::Unsupported;

        return Status::UnknownError;
    }

    GstResult GstInitialize()
    {
        GError *error = nullptr;

        if (gst_init_check(nullptr, nullptr, &error))
            return {.status = Status::Ok};

        if (error == nullptr)
            return {Status::UnknownError, "GStreamer failed to initialize"};

        GstResult result{Status::UnknownError, error->message};
        g_error_free(error);

        return result;
    }

    GstResult GstPopBusError(GstElement *pipeline)
    {
        GstBus *bus = gst_element_get_bus(pipeline);
        GstMessage *message = gst_bus_pop_filtered(bus, GST_MESSAGE_ERROR);
        gst_object_unref(bus);

        if (message == nullptr)
            return {Status::UnknownError, "GStreamer failed without a message"};

        GError *error = nullptr;
        gchar *debug = nullptr;

        gst_message_parse_error(message, &error, &debug);

        const Status status = GstToStatus(error);
        GstResult result{status, error->message};

        g_error_free(error);
        g_free(debug);
        gst_message_unref(message);

        return result;
    }

}  // namespace Pento
