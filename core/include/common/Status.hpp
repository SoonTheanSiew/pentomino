// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#ifndef PENTO_CORE_COMMON_STATUS_HPP
#define PENTO_CORE_COMMON_STATUS_HPP

#include <string_view>

namespace Pento
{

// DESIGN NOTES
// ------------
// Every call in core that can fail returns a Status. It is a plain enum, so
// returning it costs nothing on the normal path. [[nodiscard]] makes the
// compiler warn when a caller ignores one.

enum class [[nodiscard("a Status reports failures and must be checked")]] Status
{
    Ok,
    EndOfStream,  // finished normally, not an error

    InvalidParam,      // a Config value the node cannot use
    InvalidInput,      // corrupt or unexpected data on an input
    Unsupported,       // the machine cannot do what the generator chose
    FileNotFound,      // a file or path does not exist
    PermissionDenied,  // a file or device exists but may not be opened
    IoError,           // reading or writing a file failed
    NetworkError,      // a connection failed or was lost
    DeviceNotFound,    // no GPU, NPU or other device to run on
    DeviceFailed,      // the driver or device reported an error
    OutOfMemory,       // an allocation failed, on the host or a device
    Timeout            // something did not answer in time
};

constexpr bool IsError(Status status)
{
    return status != Status::Ok && status != Status::EndOfStream;
}

// For log messages. Only called when something is reported.
constexpr std::string_view StatusName(Status status)
{
    switch (status)
    {
        case Status::Ok:
            return "Ok";
        case Status::EndOfStream:
            return "EndOfStream";
        case Status::InvalidParam:
            return "InvalidParam";
        case Status::InvalidInput:
            return "InvalidInput";
        case Status::Unsupported:
            return "Unsupported";
        case Status::FileNotFound:
            return "FileNotFound";
        case Status::PermissionDenied:
            return "PermissionDenied";
        case Status::IoError:
            return "IoError";
        case Status::NetworkError:
            return "NetworkError";
        case Status::DeviceNotFound:
            return "DeviceNotFound";
        case Status::DeviceFailed:
            return "DeviceFailed";
        case Status::OutOfMemory:
            return "OutOfMemory";
        case Status::Timeout:
            return "Timeout";
    }
    return "Unknown";
}

}  // namespace Pento

#endif  // PENTO_CORE_COMMON_STATUS_HPP
