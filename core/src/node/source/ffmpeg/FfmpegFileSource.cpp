// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#include "node/source/ffmpeg/FfmpegFileSource.hpp"

#include "common/ffmpeg/FfmpegError.hpp"
#include "payload/Payload.hpp"

#include <chrono>
#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

extern "C"
{
#include <libavutil/error.h>
}

namespace Pento
{
    // Raw file bytes have no time.
    static constexpr std::chrono::nanoseconds NO_TIME{0};

    FfmpegFileSource::FfmpegFileSource(std::string name, Config config)
        : Source(std::move(name)), m_config(std::move(config))
    {
    }

    FfmpegFileSource::~FfmpegFileSource()
    {
        Terminate();
    }

    Status FfmpegFileSource::Init()
    {
        const int opened =
            avio_open(&m_context, m_config.path.c_str(), AVIO_FLAG_READ);

        if (opened < 0)
        {
            const Status status = FfmpegToStatus(opened);
            const std::string detail = FfmpegErrorDetail(opened, m_config.path);
            return Fail(status, detail);
        }

        return Status::Ok;
    }

    void FfmpegFileSource::Terminate()
    {
        avio_closep(&m_context);
    }

    Status FfmpegFileSource::Step()
    {
        // TODO: reuse chunks through a pool if measuring shows that one
        // allocation per chunk costs too much.
        auto bitstream = std::make_shared<Bitstream>();
        bitstream->bytes.resize(m_config.chunkSize);

        const int bytesRead =
            avio_read(m_context, bitstream->bytes.data(),
                      static_cast<int>(bitstream->bytes.size()));

        if (bytesRead == AVERROR_EOF || bytesRead == 0)
            return Status::EndOfStream;

        if (bytesRead < 0)
        {
            const Status status = FfmpegToStatus(bytesRead);
            const std::string detail =
                FfmpegErrorDetail(bytesRead, m_config.path);
            return Fail(status, detail);
        }

        bitstream->bytes.resize(static_cast<std::size_t>(bytesRead));

        const auto payload =
            std::make_shared<const Payload>(NO_TIME, bitstream);

        return Send(payload);
    }

}  // namespace Pento
