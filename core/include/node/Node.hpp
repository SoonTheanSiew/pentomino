// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#ifndef PENTO_CORE_NODE_NODE_HPP
#define PENTO_CORE_NODE_NODE_HPP

#include "common/Logging.hpp"
#include "common/Status.hpp"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace Pento
{

    class Edge;
    class Payload;

    // DESIGN NOTES
    // ------------
    // A Node is one component of the pipeline, for example a decoder.
    // Its inputs and outputs are numbered from 0. A source has no input.
    // Graph calls Init(), then Step() many times, then Terminate().
    // Each kind of node writes its own Step() with Pop() and Send().

    class Node : public Logging
    {
      public:
        using SPtr = std::shared_ptr<Node>;
        using UPtr = std::unique_ptr<Node>;

        virtual ~Node() = default;
        Node(const Node &) = delete;
        Node &operator=(const Node &) = delete;

        virtual Status Init() = 0;
        virtual void Terminate() = 0;

        // Does one unit of work, such as decoding one packet.
        virtual Status Step() = 0;

        // Called by Graph before Init(). Each kind of node checks its own
        // rules, for example that every output of a source is connected.
        virtual Status CheckConnections() const;

        bool HasInput() const;  // false for a source

        Status ConnectInput(std::size_t inputIndex, Edge &edge);
        Status ConnectOutput(std::size_t outputIndex, Edge &edge);

        void CloseOutputs();

      protected:
        // `name` is the node id from the graph file.
        Node(std::string name, std::size_t inputCount, std::size_t outputCount);

        bool IsOutputConnected(std::size_t outputIndex) const;

        // Takes the next payload from an input, waiting while it is empty.
        // Returns EndOfStream once the input has ended and Aborted when the
        // graph aborts.
        Status Pop(std::size_t inputIndex,
                   std::shared_ptr<const Payload> &payload);

        // Sends to every edge of the output. A full edge follows its overflow
        // rule: block, drop the oldest or drop the newest. If the result is
        // not Ok, return it from Step().
        Status Send(const std::shared_ptr<const Payload> &payload,
                    std::size_t outputIndex = 0);

      private:
        // One edge per input, any number of edges per output.
        std::vector<Edge *> m_inputEdges;
        std::vector<std::vector<Edge *>> m_outputEdges;
    };

}  // namespace Pento

#endif  // PENTO_CORE_NODE_NODE_HPP
