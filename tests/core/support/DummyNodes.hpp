// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#ifndef PENTO_TESTS_CORE_SUPPORT_DUMMY_NODES_HPP
#define PENTO_TESTS_CORE_SUPPORT_DUMMY_NODES_HPP

#include "node/Node.hpp"
#include "node/source/Source.hpp"
#include "payload/metadata/Metadata.hpp"

#include <cstddef>
#include <memory>
#include <vector>

namespace Pento
{
    // The payload number, counted from 1 by DummySource.
    class DummyTag : public Metadata
    {
      public:
        explicit DummyTag(int number);

        int Number() const;

      private:
        int m_number;
    };

    class DummySource : public Source
    {
      public:
        explicit DummySource(int payloadCount);

        Status Init() override;
        void Terminate() override;
        Status Step() override;

      private:
        int m_payloadCount;
        int m_sentCount = 0;
    };

    // Reads a file with one video and one audio stream, so it sends the
    // payloads to the video and audio outputs in turn.
    class DummyDemux : public Node
    {
      public:
        static constexpr std::size_t VIDEO_OUTPUT_INDEX = 0;
        static constexpr std::size_t AUDIO_OUTPUT_INDEX = 1;

        explicit DummyDemux(std::size_t outputCount);

        Status Init() override;
        void Terminate() override;
        Status Step() override;

      private:
        std::size_t m_nextOutputIndex = VIDEO_OUTPUT_INDEX;
    };

    // Sends every payload on as soon as it comes in.
    class DummyDecode : public Node
    {
      public:
        DummyDecode();

        Status Init() override;
        void Terminate() override;
        Status Step() override;
    };

    // Keeps the number of every payload it gets.
    class DummySink : public Node
    {
      public:
        DummySink();

        Status Init() override;
        void Terminate() override;
        Status Step() override;

        const std::vector<int> &Received() const;

      private:
        std::vector<int> m_received;
    };

    // Steps a node until it stops and closes its outputs, as Graph does.
    Status RunToEnd(Node &node);

}  // namespace Pento

#endif  // PENTO_TESTS_CORE_SUPPORT_DUMMY_NODES_HPP
