// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#include "graph/Edge.hpp"

#include "payload/Payload.hpp"

#include <algorithm>
#include <utility>
#include <vector>

namespace Pento
{

    Edge::Edge(std::string name, const EdgeOptions &options)
        : Logging(std::move(name)),
          m_overflow(options.overflow),
          m_slots(std::max<std::size_t>(options.capacity, 1))
    {
    }

    Status Edge::Push(std::shared_ptr<const Payload> payload)
    {
        if (payload == nullptr)
            return Fail(Status::InvalidOperation, "push of a null payload");

        std::shared_ptr<const Payload> dropped;

        {
            std::unique_lock lock(m_mutex);

            if (m_overflow == Overflow::Block)
            {
                m_canPush.wait(lock,
                               [this]
                               {
                                   return m_state != State::Open ||
                                          m_size < m_slots.size();
                               });
            }

            if (m_state == State::Aborted)
                return Status::Aborted;
            if (m_state == State::Closed)
                return Fail(Status::InvalidOperation, "push after close");

            if (m_size == m_slots.size())
            {
                ++m_droppedCount;
                if (m_overflow == Overflow::DropNewest)
                    return Status::Ok;
                dropped = std::move(m_slots[m_head]);
                m_head = (m_head + 1) % m_slots.size();
                --m_size;
            }

            m_slots[(m_head + m_size) % m_slots.size()] = std::move(payload);
            ++m_size;
        }

        m_canPop.notify_one();
        return Status::Ok;
    }

    Status Edge::Pop(std::shared_ptr<const Payload> &payload)
    {
        payload.reset();

        {
            std::unique_lock lock(m_mutex);
            m_canPop.wait(
                lock, [this] { return m_state != State::Open || m_size > 0; });

            if (m_size == 0)
            {
                return m_state == State::Aborted ? Status::Aborted
                                                 : Status::EndOfStream;
            }

            payload = std::move(m_slots[m_head]);
            m_head = (m_head + 1) % m_slots.size();
            --m_size;
        }

        m_canPush.notify_one();
        return Status::Ok;
    }

    void Edge::Close()
    {
        {
            const std::scoped_lock lock(m_mutex);
            m_state = State::Closed;
        }

        m_canPop.notify_all();
        m_canPush.notify_all();
    }

    void Edge::Abort()
    {
        std::vector<std::shared_ptr<const Payload>> discarded;

        {
            const std::scoped_lock lock(m_mutex);
            m_state = State::Aborted;
            discarded.reserve(m_size);

            for (std::size_t i = 0; i < m_size; ++i)
            {
                discarded.push_back(
                    std::move(m_slots[(m_head + i) % m_slots.size()]));
            }

            m_head = 0;
            m_size = 0;
        }

        m_canPop.notify_all();
        m_canPush.notify_all();
    }

    std::uint64_t Edge::DroppedCount() const
    {
        const std::scoped_lock lock(m_mutex);
        return m_droppedCount;
    }

}  // namespace Pento
