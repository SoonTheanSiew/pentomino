// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#ifndef PENTO_CORE_COMMON_GSTREAMER_GST_APP_PIPELINE_HPP
#define PENTO_CORE_COMMON_GSTREAMER_GST_APP_PIPELINE_HPP

#include "common/gstreamer/GstError.hpp"
#include "payload/Payload.hpp"

#include <memory>
#include <string>
#include <vector>

#include <gst/gst.h>

namespace Pento
{
    // DESIGN NOTES
    // ------------
    // element ! ... ! appsink, run by one node. Adjacent GStreamer nodes
    // can share one pipeline, with no appsink between them.

    class GstAppPipeline
    {
      public:
        GstAppPipeline() = default;
        ~GstAppPipeline();
        GstAppPipeline(const GstAppPipeline &) = delete;
        GstAppPipeline &operator=(const GstAppPipeline &) = delete;

        // outputCaps may be nullptr. The pipeline owns the elements, even on
        // a failure.
        GstResult Start(const std::string &name,
                        const std::vector<GstElement *> &elements,
                        const char *outputCaps);

        // Waits for a sample, EndOfStream after the last.
        GstResult Pull(std::shared_ptr<const Payload> &payload);

        void Stop();

      private:
        GstResult FailFromBus() const;

        GstElement *m_pipeline = nullptr;
        GstElement *m_sink = nullptr;  // appsink
    };

}  // namespace Pento

#endif  // PENTO_CORE_COMMON_GSTREAMER_GST_APP_PIPELINE_HPP
