// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#include "common/gstreamer/GstAppPipeline.hpp"

#include "common/gstreamer/GstContent.hpp"
#include "common/gstreamer/GstNames.hpp"
#include "graph/Edge.hpp"

#include <chrono>
#include <cstddef>
#include <format>
#include <memory>
#include <string>
#include <vector>

#include <gst/app/gstappsink.h>

namespace Pento
{
    // The number of buffers appsink holds when there is no appsrc, so a
    // source element such as filesrc waits instead of reading the whole file.
    // TODO: decide after measuring.
    static constexpr auto MAX_QUEUED_BUFFERS =
        static_cast<guint>(DEFAULT_EDGE_CAPACITY);

    // The time of a sample with no presentation time.
    static constexpr std::chrono::nanoseconds NO_TIME{0};

    static std::chrono::nanoseconds TimestampOf(const GstBuffer *buffer)
    {
        const GstClockTime pts = GST_BUFFER_PTS(buffer);
        if (!GST_CLOCK_TIME_IS_VALID(pts))
            return NO_TIME;
        const auto count = static_cast<std::chrono::nanoseconds::rep>(pts);
        return std::chrono::nanoseconds{count};
    }

    GstAppPipeline::~GstAppPipeline()
    {
        Stop();
    }

    GstResult GstAppPipeline::Start(const std::string &name,
                                    const std::vector<GstElement *> &elements,
                                    const char *outputCaps)
    {
        if (elements.empty())
            return {Status::InvalidOperation, "no elements to run"};

        GstResult result = GstInitialize();
        if (result.status != Status::Ok)
            return result;

        m_pipeline = gst_pipeline_new(name.c_str());
        GstBin *bin = GST_BIN(m_pipeline);
        for (GstElement *element : elements)
            gst_bin_add(bin, element);

        m_sink = gst_element_factory_make(APP_SINK_ELEMENT, nullptr);
        if (m_sink == nullptr)
        {
            Stop();
            return {Status::Unsupported, "GStreamer has no appsink"};
        }

        gst_bin_add(bin, m_sink);
        std::vector<GstElement *> chain = elements;
        chain.push_back(m_sink);

        for (std::size_t i = 1; i < chain.size(); ++i)
        {
            if (!gst_element_link(chain[i - 1], chain[i]))
            {
                const std::string message = std::format(
                    "cannot link {} to {}", GST_ELEMENT_NAME(chain[i - 1]),
                    GST_ELEMENT_NAME(chain[i]));
                Stop();
                return {Status::Unsupported, message};
            }
        }

        GstAppSink *appSink = GST_APP_SINK(m_sink);
        g_object_set(m_sink, SYNC_PROPERTY, FALSE, nullptr);
        if (outputCaps != nullptr)
        {
            GstCaps *sinkCaps = gst_caps_from_string(outputCaps);
            gst_app_sink_set_caps(appSink, sinkCaps);
            gst_caps_unref(sinkCaps);
        }
        gst_app_sink_set_max_buffers(appSink, MAX_QUEUED_BUFFERS);

        // A missing file or device fails here.
        const GstStateChangeReturn changed =
            gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
        if (changed == GST_STATE_CHANGE_FAILURE)
        {
            result = FailFromBus();
            Stop();
            return result;
        }

        return {.status = Status::Ok};
    }

    GstResult GstAppPipeline::Pull(std::shared_ptr<const Payload> &payload)
    {
        payload = nullptr;
        GstAppSink *appSink = GST_APP_SINK(m_sink);
        GstSample *sample = gst_app_sink_pull_sample(appSink);
        if (sample == nullptr)
        {
            if (gst_app_sink_is_eos(appSink))
                return {.status = Status::EndOfStream};
            return FailFromBus();
        }

        Content content;
        const GstResult result = GstSampleToContent(sample, content);
        if (result.status != Status::Ok)
        {
            gst_sample_unref(sample);
            return result;
        }

        const GstBuffer *buffer = gst_sample_get_buffer(sample);
        const std::chrono::nanoseconds timestamp = TimestampOf(buffer);
        gst_sample_unref(sample);

        payload = std::make_shared<const Payload>(timestamp, content);
        return {.status = Status::Ok};
    }

    void GstAppPipeline::Stop()
    {
        if (m_pipeline == nullptr)
            return;

        gst_element_set_state(m_pipeline, GST_STATE_NULL);
        gst_object_unref(m_pipeline);
        m_pipeline = nullptr;
        m_sink = nullptr;
    }

    GstResult GstAppPipeline::FailFromBus() const
    {
        return GstPopBusError(m_pipeline);
    }

}  // namespace Pento
