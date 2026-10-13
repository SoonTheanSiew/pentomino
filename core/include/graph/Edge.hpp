// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#ifndef PENTO_CORE_GRAPH_EDGE_HPP
#define PENTO_CORE_GRAPH_EDGE_HPP

#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "common/Logging.hpp"
#include "common/Status.hpp"

namespace Pento
{

    class Payload;

    // DESIGN NOTES
    // ------------
    // An Edge is a bounded queue of payloads from one node to the next. Its
    // capacity and overflow rule come from the graph file. One mutex guards it,
    // because DropOldest lets the upstream side remove payloads too.

    enum class Overflow
    {
        Block,       // the upstream node waits for a free slot
        DropOldest,  // the oldest queued payload makes room for the new one
        DropNewest   // the new payload is dropped, the queue stays as it is
    };

    constexpr std::size_t DEFAULT_EDGE_CAPACITY = 4;

    struct EdgeOptions
    {
        std::size_t capacity = DEFAULT_EDGE_CAPACITY;  // at least 1
        Overflow overflow = Overflow::Block;
    };

    class Edge : public Logging
    {
      public:
        using SPtr = std::shared_ptr<Edge>;
        using UPtr = std::unique_ptr<Edge>;

        // Graph names it "<upstream>.<output> -> <downstream>", for example
        // "demux.1 -> adec".
        explicit Edge(std::string name, const EdgeOptions &options = {});
        Edge(const Edge &) = delete;
        Edge &operator=(const Edge &) = delete;

        // Called by the upstream node. When the queue is full, follows the
        // overflow rule. Returns Aborted if the graph is shutting down.
        // InvalidOperation means a bug upstream: a null payload, or a push
        // after the node has finished.
        Status Push(std::shared_ptr<const Payload> payload);

        // Called by the downstream node. Waits while the queue is empty.
        // Returns Ok with the next payload, EndOfStream once the upstream node
        // has finished and the queue is empty, or Aborted if the graph aborts.
        Status Pop(std::shared_ptr<const Payload> &payload);

        // Called by Graph when the upstream node has finished. The downstream
        // node still gets what is queued, then EndOfStream.
        void Close();

        // Called by Graph when the graph aborts. Drops what is queued, so both
        // sides get Aborted right away.
        void Abort();

        // Payloads lost to DropOldest or DropNewest so far.
        std::uint64_t DroppedCount() const;

      private:
        enum class State
        {
            Open,
            Closed,  // the upstream node has finished
            Aborted  // the graph is shutting down
        };

        const Overflow m_overflow;
        std::vector<std::shared_ptr<const Payload>> m_slots;  // ring buffer
        std::size_t m_head = 0;  // next slot to pop
        std::size_t m_size = 0;  // payloads queued
        std::uint64_t m_droppedCount = 0;
        State m_state = State::Open;
        mutable std::mutex m_mutex;  // guards every member above
        std::condition_variable m_canPop;
        std::condition_variable m_canPush;
    };

}  // namespace Pento

#endif  // PENTO_CORE_GRAPH_EDGE_HPP
