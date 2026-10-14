// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#include "support/DummyNodes.hpp"

#include "payload/Payload.hpp"

#include <chrono>

namespace Pento
{
    static constexpr std::size_t NO_INPUT = 0;
    static constexpr std::size_t ONE_INPUT = 1;
    static constexpr std::size_t NO_OUTPUT = 0;
    static constexpr std::size_t ONE_OUTPUT = 1;
    static constexpr std::size_t FIRST_INPUT = 0;
    static constexpr std::chrono::nanoseconds NO_TIME{0};

    // --- DummyTag ------------------------------------------------------------

    DummyTag::DummyTag(int number) : m_number(number)
    {
    }

    int DummyTag::Number() const
    {
        return m_number;
    }

    // --- DummySource ---------------------------------------------------------

    DummySource::DummySource(int payloadCount)
        : Node("source", NO_INPUT, ONE_OUTPUT), m_payloadCount(payloadCount)
    {
    }

    Status DummySource::Init()
    {
        return Status::Ok;
    }

    void DummySource::Terminate()
    {
    }

    Status DummySource::Step()
    {
        if (m_sentCount == m_payloadCount)
            return Status::EndOfStream;

        ++m_sentCount;

        const auto content = std::make_shared<Bitstream>();
        const auto tag = std::make_shared<DummyTag>(m_sentCount);
        const auto untagged = std::make_shared<const Payload>(NO_TIME, content);
        const std::shared_ptr<const Payload> payload = untagged->Attach(tag);

        return Send(payload);
    }

    // --- DummyDemux ----------------------------------------------------------

    DummyDemux::DummyDemux(std::size_t outputCount)
        : Node("demux", ONE_INPUT, outputCount)
    {
    }

    Status DummyDemux::Init()
    {
        return Status::Ok;
    }

    void DummyDemux::Terminate()
    {
    }

    Status DummyDemux::Step()
    {
        std::shared_ptr<const Payload> input;
        const Status popped = Pop(FIRST_INPUT, input);

        if (popped != Status::Ok)
            return popped;

        const Status sent = Send(input, m_nextOutput);
        m_nextOutput =
            m_nextOutput == VIDEO_OUTPUT ? AUDIO_OUTPUT : VIDEO_OUTPUT;

        return sent;
    }

    // --- DummyDecode ---------------------------------------------------------

    DummyDecode::DummyDecode() : Node("decode", ONE_INPUT, ONE_OUTPUT)
    {
    }

    Status DummyDecode::Init()
    {
        return Status::Ok;
    }

    void DummyDecode::Terminate()
    {
    }

    Status DummyDecode::Step()
    {
        std::shared_ptr<const Payload> input;
        const Status popped = Pop(FIRST_INPUT, input);

        if (popped != Status::Ok)
            return popped;

        return Send(input);
    }

    // --- DummySink -----------------------------------------------------------

    DummySink::DummySink() : Node("sink", ONE_INPUT, NO_OUTPUT)
    {
    }

    Status DummySink::Init()
    {
        return Status::Ok;
    }

    void DummySink::Terminate()
    {
    }

    const std::vector<int> &DummySink::Received() const
    {
        return m_received;
    }

    Status DummySink::Step()
    {
        std::shared_ptr<const Payload> input;
        const Status popped = Pop(FIRST_INPUT, input);

        if (popped != Status::Ok)
            return popped;

        const DummyTag *tag = input->Find<DummyTag>();
        const int number = tag->Number();
        m_received.push_back(number);

        return Status::Ok;
    }

    // --- RunToEnd ------------------------------------------------------------

    Status RunToEnd(Node &node)
    {
        Status status = Status::Ok;

        while (status == Status::Ok)
            status = node.Step();

        node.CloseOutputs();

        return status;
    }

}  // namespace Pento
