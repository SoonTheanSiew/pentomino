// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#ifndef PENTO_CORE_NODE_SOURCE_FFMPEG_FFMPEG_FILE_SOURCE_HPP
#define PENTO_CORE_NODE_SOURCE_FFMPEG_FFMPEG_FILE_SOURCE_HPP

#include "node/source/Source.hpp"

#include <cstddef>
#include <string>

extern "C"
{
#include <libavformat/avio.h>
}

namespace Pento
{
    // Reads a file with FFmpeg's avio and sends it in chunks of bytes, one
    // Bitstream per chunk. It does not demux.
    class FfmpegFileSource final : public Source
    {
      public:
        static constexpr std::size_t DEFAULT_CHUNK_SIZE = 64 * 1024;

        struct Config
        {
            std::string path;
            std::size_t chunkSize = DEFAULT_CHUNK_SIZE;
        };

        FfmpegFileSource(std::string name, Config config);
        ~FfmpegFileSource() override;

        Status Init() override;
        void Terminate() override;
        Status Step() override;

      private:
        Config m_config;
        AVIOContext *m_context = nullptr;
    };

}  // namespace Pento

#endif  // PENTO_CORE_NODE_SOURCE_FFMPEG_FFMPEG_FILE_SOURCE_HPP
