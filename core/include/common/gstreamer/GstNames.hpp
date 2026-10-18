// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#ifndef PENTO_CORE_COMMON_GSTREAMER_GST_NAMES_HPP
#define PENTO_CORE_COMMON_GSTREAMER_GST_NAMES_HPP

namespace Pento
{
    // GStreamer element and property names that more than one node uses.

    // --- elements -----------------------------------------------------------

    // The end of a pipeline.
    inline constexpr const char *APP_SINK_ELEMENT = "appsink";

    // --- properties ---------------------------------------------------------

    // The same name on every element that has the property.
    inline constexpr const char *BLOCK_SIZE_PROPERTY = "blocksize";
    inline constexpr const char *LOCATION_PROPERTY = "location";
    inline constexpr const char *SYNC_PROPERTY = "sync";

}  // namespace Pento

#endif  // PENTO_CORE_COMMON_GSTREAMER_GST_NAMES_HPP
