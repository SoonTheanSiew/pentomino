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

    enum class [[nodiscard(
        "a Status reports failures and must be checked")]] Status
    {
        Ok,
        EndOfStream,  // a source has no more data, not an error
        Aborted,      // stopped before the stream ended, not an error

        InvalidParam,      // a Config value the node cannot use
        InvalidInput,      // corrupt or unexpected data on an input
        InvalidOperation,  // a call that is not allowed here, a bug in the
                           // caller
        Unsupported,       // the machine cannot do what the generator chose
        FileNotFound,      // a file or path does not exist
        PermissionDenied,  // a file or device exists but may not be opened
        IoError,           // reading or writing a file failed
        NetworkError,      // a connection failed or was lost
        DeviceNotFound,    // no GPU, NPU or other device to run on
        DeviceFailed,      // the driver or device reported an error
        OutOfMemory,       // an allocation failed, on the host or a device
        Timeout,           // something did not answer in time
        UnknownError       // none of the above fits, the log has the details
    };

    // Neither Ok, EndOfStream nor Aborted.
    constexpr bool IsError(Status status);

    // For log messages. Only called when something is reported.
    constexpr std::string_view StatusName(Status status);

    // Constexpr functions are defined here, below the declarations, because
    // every caller needs to see them.

    constexpr bool IsError(Status status)
    {
        return status != Status::Ok && status != Status::EndOfStream &&
               status != Status::Aborted;
    }

    constexpr std::string_view StatusName(Status status)
    {
        switch (status)
        {
            case Status::Ok:
                return "Ok";
            case Status::EndOfStream:
                return "EndOfStream";
            case Status::Aborted:
                return "Aborted";
            case Status::InvalidParam:
                return "InvalidParam";
            case Status::InvalidInput:
                return "InvalidInput";
            case Status::InvalidOperation:
                return "InvalidOperation";
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
            case Status::UnknownError:
                return "UnknownError";
        }
        return "Unknown";
    }

}  // namespace Pento

#endif  // PENTO_CORE_COMMON_STATUS_HPP
