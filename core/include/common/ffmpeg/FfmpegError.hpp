// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#ifndef PENTO_CORE_COMMON_FFMPEG_FFMPEG_ERROR_HPP
#define PENTO_CORE_COMMON_FFMPEG_FFMPEG_ERROR_HPP

#include "common/Status.hpp"

#include <string>
#include <string_view>

namespace Pento
{
    // Turns an AVERROR code from any FFmpeg call into a Status. The caller
    // handles AVERROR(EAGAIN) and AVERROR_EOF itself, they are not errors.
    Status FfmpegToStatus(int error);

    // FFmpeg's message for an AVERROR code, for log messages.
    std::string FfmpegErrorText(int error);

    // "<what>: <FFmpeg's message>", the detail to pass to Fail().
    std::string FfmpegErrorDetail(int error, std::string_view what);

}  // namespace Pento

#endif  // PENTO_CORE_COMMON_FFMPEG_FFMPEG_ERROR_HPP
