// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#ifndef PENTO_CORE_GRAPH_GRAPH_HPP
#define PENTO_CORE_GRAPH_GRAPH_HPP

#include "common/Logging.hpp"
#include "common/Status.hpp"
#include "graph/Edge.hpp"
#include "node/Node.hpp"

#include <atomic>
#include <concepts>
#include <cstddef>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

namespace Pento
{

    // DESIGN NOTES
    // ------------
    // A Graph owns the nodes and edges of one pipeline and runs them:
    // Add() and Connect(), then Init(), Start(), Wait() or Stop(), Terminate().
    //
    // Every node runs on its own thread. The edges are the only place where
    // threads meet. When a node fails, Graph aborts every edge, so every
    // other node stops at its next Pop() or Send(), or before its next Step().

    class Graph : public Logging
    {
      public:
        using SPtr = std::shared_ptr<Graph>;
        using UPtr = std::unique_ptr<Graph>;

        Graph();
        ~Graph();
        Graph(const Graph &) = delete;
        Graph &operator=(const Graph &) = delete;

        // Creates a node in place. Nodes are added in topological order, which
        // is the order Init() runs in.
        template <class NodeType, class... Args>
            requires std::derived_from<NodeType, Node>
        NodeType &Add(Args &&...args);

        // Connects the first output of `upstream` to `downstream`, once per
        // downstream node. On a bad connection, returns nullptr and Init()
        // returns the error.
        Edge *Connect(Node &upstream, Node &downstream,
                      const EdgeOptions &options = {});

        // Connects output `outputIndex` of `upstream` to input `inputIndex` of
        // `downstream`.
        Edge *Connect(Node &upstream, std::size_t outputIndex, Node &downstream,
                      std::size_t inputIndex, const EdgeOptions &options = {});

        // Fails on a bad Connect(), or stops at the first node that fails and
        // terminates the ones already initialized.
        Status Init();

        // Starts one thread per node.
        void Start();

        // Returns the first error, or Ok.
        Status Wait();

        // Stops every node at once, drops what is queued and waits for the
        // threads. Callable from any thread.
        Status Stop();

        // Terminates every initialized node, in reverse order of Init().
        void Terminate();

      private:
        // Steps one node on its own thread until it stops.
        void RunNode(Node &node);

        void SetError(Status status);
        void RequestStop();  // sets the flag and aborts every edge, no join
        void Join();

        std::vector<Node::UPtr> m_nodes;
        std::vector<Edge::UPtr> m_edges;
        std::vector<std::thread> m_threads;
        std::size_t m_initializedCount = 0;
        std::mutex m_joinMutex;  // Stop() and Wait() may join at once
        std::atomic<bool> m_isStopped{false};
        std::mutex m_statusMutex;  // guards m_status
        Status m_status = Status::Ok;
    };

    // Templates are defined here, below the class, because every caller needs
    // to see them.

    template <class NodeType, class... Args>
        requires std::derived_from<NodeType, Node>
    NodeType &Graph::Add(Args &&...args)
    {
        auto node = std::make_unique<NodeType>(std::forward<Args>(args)...);
        NodeType &added = *node;
        m_nodes.push_back(std::move(node));
        return added;
    }

}  // namespace Pento

#endif  // PENTO_CORE_GRAPH_GRAPH_HPP
