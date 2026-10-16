// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#include "graph/Graph.hpp"

#include <format>
#include <functional>
#include <string>

namespace Pento
{

    static constexpr std::size_t OUTPUT_INDEX_0 = 0;
    static constexpr std::size_t INPUT_INDEX_0 = 0;

    Graph::Graph() : Logging("graph")
    {
    }

    Graph::~Graph()
    {
        // Nodes hold raw pointers to the edges, so every thread ends first.
        static_cast<void>(Stop());
        Terminate();
    }

    Edge *Graph::Connect(Node &upstream, Node &downstream,
                         const EdgeOptions &options)
    {
        return Connect(upstream, OUTPUT_INDEX_0, downstream, INPUT_INDEX_0,
                       options);
    }

    Edge *Graph::Connect(Node &upstream, std::size_t outputIndex,
                         Node &downstream, std::size_t inputIndex,
                         const EdgeOptions &options)
    {
        std::string name =
            std::format("{}.{} -> {}.{}", upstream.Name(), outputIndex,
                        downstream.Name(), inputIndex);
        m_edges.push_back(std::make_unique<Edge>(std::move(name), options));
        Edge &added = *m_edges.back();

        Status status = upstream.ConnectOutput(outputIndex, added);
        if (status != Status::Ok)
        {
            Error("{}: {} has no output {}", added.Name(), upstream.Name(),
                  outputIndex);
            SetError(status);
            return nullptr;
        }

        status = downstream.ConnectInput(inputIndex, added);
        if (status != Status::Ok)
        {
            Error("{}: {} has no free input {}", added.Name(),
                  downstream.Name(), inputIndex);
            SetError(status);
            return nullptr;
        }

        return &added;
    }

    Status Graph::Init()
    {
        {
            const std::scoped_lock lock(m_statusMutex);
            if (m_status != Status::Ok)
                return m_status;
        }

        for (const Node::UPtr &node : m_nodes)
        {
            const Status status = node->CheckConnections();
            if (status != Status::Ok)
            {
                Error("{} failed the connection check with {}", node->Name(),
                      StatusName(status));
                return status;
            }
        }

        for (const Node::UPtr &node : m_nodes)
        {
            const Status status = node->Init();

            if (status != Status::Ok)
            {
                Error("{} failed to init with {}", node->Name(),
                      StatusName(status));
                Terminate();
                return status;
            }

            ++m_initializedCount;
        }

        return Status::Ok;
    }

    void Graph::Start()
    {
        for (const Node::UPtr &node : m_nodes)
        {
            Node &target = *node;
            m_threads.emplace_back(&Graph::RunNode, this, std::ref(target));
        }
    }

    Status Graph::Wait()
    {
        Join();

        const std::scoped_lock lock(m_statusMutex);
        return m_status;
    }

    Status Graph::Stop()
    {
        RequestStop();
        return Wait();
    }

    void Graph::RequestStop()
    {
        m_isStopped = true;

        for (const Edge::UPtr &edge : m_edges)
            edge->Abort();
    }

    void Graph::Terminate()
    {
        while (m_initializedCount > 0)
        {
            --m_initializedCount;
            m_nodes[m_initializedCount]->Terminate();
        }
    }

    void Graph::RunNode(Node &node)
    {
        Status status = Status::Ok;

        while (status == Status::Ok && !m_isStopped)
            status = node.Step();

        Debug("{} stopped with {}", node.Name(), StatusName(status));

        if (status == Status::EndOfStream)
        {
            node.CloseOutputs();
        }
        else if (IsError(status))
        {
            Error("{} failed with {}", node.Name(), StatusName(status));
            SetError(status);
            RequestStop();
        }
    }

    void Graph::SetError(Status status)
    {
        const std::scoped_lock lock(m_statusMutex);
        if (m_status == Status::Ok)
            m_status = status;
    }

    void Graph::Join()
    {
        const std::scoped_lock lock(m_joinMutex);

        for (std::thread &thread : m_threads)
            thread.join();

        m_threads.clear();
    }

}  // namespace Pento
