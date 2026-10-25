// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#ifndef PENTO_CORE_NODE_DECODE_DECODE_HPP
#define PENTO_CORE_NODE_DECODE_DECODE_HPP

#include "common/Codec.hpp"
#include "node/Node.hpp"

#include <cstddef>
#include <string>

namespace Pento
{
    // A node with one input of Bitstreams and one output of raw frames. When
    // its input ends, it sends the frames it still holds and then returns
    // EndOfStream.
    class Decode : public Node
    {
      public:
        enum class Device
        {
            Cpu,
            Gpu
        };

        // The kind of frame to send, chosen by the generator for the best
        // path between this node and the next.
        enum class Output
        {
            SystemFrame,
            DmaBuf,
            VaSurface
        };

        static constexpr std::size_t DEFAULT_EXTRA_FRAMES = 8;

        struct Config
        {
            Codec codec = Codec::H264;
            Device device = Device::Gpu;
            Output output = Output::VaSurface;
            // Frames the next nodes may hold at once, on top of what the
            // decoder needs, so it does not run out of surfaces.
            // TODO: decide the default after measuring.
            std::size_t extraFrames = DEFAULT_EXTRA_FRAMES;
        };

        // The input and the output must be connected.
        Status CheckConnections() const override;

      protected:
        Decode(std::string name, Config config);

        const Config &GetConfig() const;

      private:
        Config m_config;
    };

}  // namespace Pento

#endif  // PENTO_CORE_NODE_DECODE_DECODE_HPP
