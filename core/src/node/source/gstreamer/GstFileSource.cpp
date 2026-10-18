// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#include "node/source/gstreamer/GstFileSource.hpp"

#include "common/gstreamer/GstNames.hpp"

#include <format>
#include <memory>
#include <string>
#include <utility>

namespace Pento
{
    // The GStreamer element that reads the file.
    static constexpr const char *FILE_ELEMENT = "filesrc";

    GstFileSource::GstFileSource(std::string name, Config config)
        : Source(std::move(name)), m_config(std::move(config))
    {
    }

    GstElement *GstFileSource::MakeElement(const Config &config,
                                           GstResult &result)
    {
        result = GstInitialize();
        if (result.status != Status::Ok)
            return nullptr;

        GstElement *source = gst_element_factory_make(FILE_ELEMENT, nullptr);
        if (source == nullptr)
        {
            const std::string message =
                std::format("GStreamer has no {}", FILE_ELEMENT);
            result = {Status::Unsupported, message};
            return nullptr;
        }

        const auto blockSize = static_cast<guint>(config.chunkSize);
        g_object_set(source, LOCATION_PROPERTY, config.path.c_str(),
                     BLOCK_SIZE_PROPERTY, blockSize, nullptr);
        return source;
    }

    Status GstFileSource::Init()
    {
        GstResult result;
        GstElement *source = MakeElement(m_config, result);
        if (source == nullptr)
            return Fail(result.status, result.message);

        // Raw file bytes have no caps, so appsink takes anything.
        result = m_pipeline.Start(Name(), {source}, nullptr);
        if (result.status != Status::Ok)
            return FailWithPath(result);

        return Status::Ok;
    }

    void GstFileSource::Terminate()
    {
        m_pipeline.Stop();
    }

    Status GstFileSource::Step()
    {
        std::shared_ptr<const Payload> payload;
        const GstResult result = m_pipeline.Pull(payload);
        if (result.status == Status::EndOfStream)
            return Status::EndOfStream;
        if (result.status != Status::Ok)
            return FailWithPath(result);

        return Send(payload);
    }

    Status GstFileSource::FailWithPath(const GstResult &result) const
    {
        const std::string detail =
            std::format("{}: {}", m_config.path, result.message);
        return Fail(result.status, detail);
    }

}  // namespace Pento
