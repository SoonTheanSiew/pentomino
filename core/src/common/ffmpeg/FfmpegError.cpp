// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#include "common/ffmpeg/FfmpegError.hpp"

#include <array>
#include <cerrno>
#include <format>
#include <string>
#include <string_view>

extern "C"
{
#include <libavutil/error.h>
}

namespace Pento
{
    Status FfmpegToStatus(int error)
    {
        switch (error)
        {
            case AVERROR(ENOENT):
                return Status::FileNotFound;
            case AVERROR(EACCES):
            case AVERROR(EPERM):
                return Status::PermissionDenied;
            case AVERROR_INVALIDDATA:
                return Status::InvalidInput;
            case AVERROR_OPTION_NOT_FOUND:
                return Status::InvalidParam;
            case AVERROR(ENOSYS):
            case AVERROR_PATCHWELCOME:
            case AVERROR_BSF_NOT_FOUND:
            case AVERROR_DECODER_NOT_FOUND:
            case AVERROR_DEMUXER_NOT_FOUND:
            case AVERROR_ENCODER_NOT_FOUND:
            case AVERROR_FILTER_NOT_FOUND:
            case AVERROR_MUXER_NOT_FOUND:
            case AVERROR_PROTOCOL_NOT_FOUND:
                return Status::Unsupported;
            case AVERROR(ECONNREFUSED):
            case AVERROR(ECONNRESET):
            case AVERROR(EHOSTUNREACH):
            case AVERROR(ENETUNREACH):
                return Status::NetworkError;
            case AVERROR(ENODEV):
                return Status::DeviceNotFound;
            case AVERROR(ENOMEM):
                return Status::OutOfMemory;
            case AVERROR(EIO):
                return Status::IoError;
            case AVERROR(ETIMEDOUT):
                return Status::Timeout;
            default:
                return Status::UnknownError;
        }
    }

    std::string FfmpegErrorText(int error)
    {
        std::array<char, AV_ERROR_MAX_STRING_SIZE> text{};
        av_strerror(error, text.data(), text.size());
        return text.data();
    }

    std::string FfmpegErrorDetail(int error, std::string_view what)
    {
        const std::string text = FfmpegErrorText(error);
        return std::format("{}: {}", what, text);
    }

}  // namespace Pento
