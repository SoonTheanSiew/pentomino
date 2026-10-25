// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#include "node/Node.hpp"

#include "graph/Edge.hpp"

#include <format>
#include <string>
#include <utility>

namespace Pento
{

    Node::Node(std::string name, std::size_t inputCount,
               std::size_t outputCount)
        : Logging(std::move(name)),
          m_inputEdges(inputCount, nullptr),
          m_outputEdges(outputCount)
    {
    }

    Status Node::ConnectInput(std::size_t inputIndex, Edge &edge)
    {
        if (inputIndex >= m_inputEdges.size() ||
            m_inputEdges[inputIndex] != nullptr)
            return Status::InvalidOperation;

        m_inputEdges[inputIndex] = &edge;

        return Status::Ok;
    }

    Status Node::ConnectOutput(std::size_t outputIndex, Edge &edge)
    {
        if (outputIndex >= m_outputEdges.size())
            return Status::InvalidOperation;

        m_outputEdges[outputIndex].push_back(&edge);

        return Status::Ok;
    }

    Status Node::CheckConnections() const
    {
        return Status::Ok;
    }

    bool Node::IsInputConnected(std::size_t inputIndex) const
    {
        return inputIndex < m_inputEdges.size() &&
               m_inputEdges[inputIndex] != nullptr;
    }

    bool Node::IsOutputConnected(std::size_t outputIndex) const
    {
        return outputIndex < m_outputEdges.size() &&
               !m_outputEdges[outputIndex].empty();
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

    Status Node::Pop(std::size_t inputIndex,
                     std::shared_ptr<const Payload> &payload)
    {
        // TODO: a mux needs to pop whichever input has data first, so no
        // input can block another.
        if (inputIndex >= m_inputEdges.size())
        {
            const std::string detail = std::format(
                "Pop() from input edge {}, but the input edge "
                "count is {}",
                inputIndex, m_inputEdges.size());
            return Fail(Status::InvalidOperation, detail);
        }

        Edge *edge = m_inputEdges[inputIndex];
        if (edge == nullptr)
        {
            const std::string detail =
                std::format("input edge {} is not connected", inputIndex);
            return Fail(Status::InvalidOperation, detail);
        }

        return edge->Pop(payload);
    }

    Status Node::Send(const std::shared_ptr<const Payload> &payload,
                      std::size_t outputIndex)
    {
        if (payload == nullptr)
            return Fail(Status::InvalidOperation, "Send() got a null payload");

        if (outputIndex >= m_outputEdges.size())
        {
            const std::string detail =
                std::format("Send() to output {}, but the output count is {}",
                            outputIndex, m_outputEdges.size());
            return Fail(Status::InvalidOperation, detail);
        }

        // An output with no edge drops the payload.
        for (Edge *edge : m_outputEdges[outputIndex])
        {
            const Status status = edge->Push(payload);
            if (status != Status::Ok)
                return status;
        }

        return Status::Ok;
    }

}  // namespace Pento
