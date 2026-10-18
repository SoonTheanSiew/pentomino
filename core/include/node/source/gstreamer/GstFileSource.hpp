// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#ifndef PENTO_CORE_NODE_SOURCE_GSTREAMER_GST_FILE_SOURCE_HPP
#define PENTO_CORE_NODE_SOURCE_GSTREAMER_GST_FILE_SOURCE_HPP

#include "common/gstreamer/GstAppPipeline.hpp"
#include "common/gstreamer/GstError.hpp"
#include "node/source/Source.hpp"

#include <cstddef>
#include <string>

#include <gst/gst.h>

namespace Pento
{
    // Reads a file with GStreamer's filesrc and sends it in chunks of bytes,
    // one Bitstream per chunk. It does not demux.
    class GstFileSource final : public Source
    {
      public:
        static constexpr std::size_t DEFAULT_CHUNK_SIZE = 64 * 1024;

        struct Config
        {
            std::string path;
            std::size_t chunkSize = DEFAULT_CHUNK_SIZE;
        };

        GstFileSource(std::string name, Config config);

        // filesrc, set up from the config. A pipeline of several GStreamer
        // nodes can use it too.
        static GstElement *MakeElement(const Config &config, GstResult &result);

        Status Init() override;
        void Terminate() override;
        Status Step() override;

      private:
        // Logs the result with the file's path and returns its status.
        Status FailWithPath(const GstResult &result) const;

        Config m_config;
        GstAppPipeline m_pipeline;
    };

}  // namespace Pento

#endif  // PENTO_CORE_NODE_SOURCE_GSTREAMER_GST_FILE_SOURCE_HPP
