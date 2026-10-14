// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#include "node/Node.hpp"

#include <format>
#include <string>
#include <utility>

#include "graph/Edge.hpp"

namespace Pento
{

    Node::Node(std::string name, std::size_t inputCount,
               std::size_t outputCount)
        : Logging(std::move(name)),
          m_inputEdges(inputCount, nullptr),
          m_outputEdges(outputCount)
    {
    }

    Status Node::ConnectInput(std::size_t index, Edge &edge)
    {
        if (index >= m_inputEdges.size() || m_inputEdges[index] != nullptr)
            return Status::InvalidOperation;

        m_inputEdges[index] = &edge;

        return Status::Ok;
    }

    Status Node::ConnectOutput(std::size_t index, Edge &edge)
    {
        if (index >= m_outputEdges.size())
            return Status::InvalidOperation;

        m_outputEdges[index].push_back(&edge);

        return Status::Ok;
    }

    void Node::CloseOutputs()
    {
        for (const auto &edges : m_outputEdges)
        {
            for (Edge *edge : edges)
                edge->Close();
        }
    }

    bool Node::HasInput() const
    {
        return !m_inputEdges.empty();
    }

    Status Node::Pop(std::size_t input, std::shared_ptr<const Payload> &payload)
    {
        // TODO: a mux needs to pop whichever input has data first, so no
        // input can block another.
        if (input >= m_inputEdges.size())
        {
            const std::string detail = std::format(
                "Pop() from input edge {}, but the input edge "
                "count is {}",
                input, m_inputEdges.size());
            return Fail(Status::InvalidOperation, detail);
        }

        Edge *edge = m_inputEdges[input];
        if (edge == nullptr)
        {
            const std::string detail =
                std::format("input edge {} is not connected", input);
            return Fail(Status::InvalidOperation, detail);
        }

        return edge->Pop(payload);
    }

    Status Node::Send(const std::shared_ptr<const Payload> &payload,
                      std::size_t output)
    {
        if (payload == nullptr)
            return Fail(Status::InvalidOperation, "Send() got a null payload");

        if (output >= m_outputEdges.size())
        {
            const std::string detail =
                std::format("Send() to output {}, but the output count is {}",
                            output, m_outputEdges.size());
            return Fail(Status::InvalidOperation, detail);
        }

        // An output with no edge drops the payload.
        for (Edge *edge : m_outputEdges[output])
        {
            const Status pushed = edge->Push(payload);
            if (pushed != Status::Ok)
                return pushed;
        }

        return Status::Ok;
    }

}  // namespace Pento
