// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#include <gtest/gtest.h>

#include <chrono>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <thread>

#include "graph/Edge.hpp"
#include "payload/Payload.hpp"

using namespace Pento;

// Each payload carries a DummyTag with its number, so a test can tell which
// ones came out of the edge and in what order.
class EdgeTest : public ::testing::Test
{
  protected:
    static constexpr std::size_t SMALL_CAPACITY = 2;
    static constexpr int FIRST = 1;
    static constexpr int SECOND = 2;
    static constexpr int THIRD = 3;
    static constexpr int PAYLOAD_COUNT = 100;
    static constexpr int NO_NUMBER = -1;

    // Marks a payload with a number, for tests only.
    struct DummyTag : Metadata
    {
        explicit DummyTag(int value) : number(value)
        {
        }
        int number;
    };

    static std::shared_ptr<const Payload> MakePayload(int number)
    {
        auto payload = std::make_shared<const Payload>(
            std::chrono::nanoseconds(0), std::make_shared<Bitstream>());
        return payload->Attach(std::make_shared<DummyTag>(number));
    }

    static int NumberOf(const std::shared_ptr<const Payload> &payload)
    {
        return payload->Find<DummyTag>()->number;
    }

    // Pops one payload, expects Ok and returns its number.
    static int PopNumber(Edge &edge)
    {
        std::shared_ptr<const Payload> payload;
        EXPECT_EQ(edge.Pop(payload), Status::Ok);
        return payload != nullptr ? NumberOf(payload) : NO_NUMBER;
    }

    // Pops once and returns only the status, for the end of a stream.
    static Status PopStatus(Edge &edge)
    {
        std::shared_ptr<const Payload> payload;
        return edge.Pop(payload);
    }

    static void PushAll(Edge &edge, std::initializer_list<int> numbers)
    {
        for (int number : numbers)
            ASSERT_EQ(edge.Push(MakePayload(number)), Status::Ok);
    }
};

TEST_F(EdgeTest, CloseLetsTheQueueDrainInOrder)
{
    Edge edge("edge", {});
    PushAll(edge, {FIRST, SECOND, THIRD});
    edge.Close();

    EXPECT_EQ(PopNumber(edge), FIRST);
    EXPECT_EQ(PopNumber(edge), SECOND);
    EXPECT_EQ(PopNumber(edge), THIRD);
    EXPECT_EQ(PopStatus(edge), Status::EndOfStream);
}

TEST_F(EdgeTest, AbortDropsWhatIsQueued)
{
    Edge edge("edge", {});
    auto first = MakePayload(FIRST);
    std::weak_ptr<const Payload> watcher = first;

    ASSERT_EQ(edge.Push(std::move(first)), Status::Ok);

    edge.Abort();

    EXPECT_TRUE(watcher.expired());
    EXPECT_EQ(PopStatus(edge), Status::Aborted);
    EXPECT_EQ(edge.Push(MakePayload(SECOND)), Status::Aborted);
}

TEST_F(EdgeTest, InvalidPushesAreRejected)
{
    Edge edge("edge", {});
    EXPECT_EQ(edge.Push(nullptr), Status::InvalidOperation);

    PushAll(edge, {FIRST});
    edge.Close();
    EXPECT_EQ(edge.Push(MakePayload(SECOND)), Status::InvalidOperation);

    EXPECT_EQ(PopNumber(edge), FIRST);
    EXPECT_EQ(PopStatus(edge), Status::EndOfStream);
}

TEST_F(EdgeTest, DropOldest)
{
    Edge edge("edge",
              {.capacity = SMALL_CAPACITY, .overflow = Overflow::DropOldest});
    auto first = MakePayload(FIRST);
    std::weak_ptr<const Payload> watcher = first;

    ASSERT_EQ(edge.Push(std::move(first)), Status::Ok);
    PushAll(edge, {SECOND, THIRD});

    EXPECT_EQ(edge.DroppedCount(), 1U);
    EXPECT_TRUE(watcher.expired());
    EXPECT_EQ(PopNumber(edge), SECOND);
    EXPECT_EQ(PopNumber(edge), THIRD);
}

TEST_F(EdgeTest, DropNewest)
{
    Edge edge("edge",
              {.capacity = SMALL_CAPACITY, .overflow = Overflow::DropNewest});
    PushAll(edge, {FIRST, SECOND, THIRD});

    EXPECT_EQ(edge.DroppedCount(), 1U);
    EXPECT_EQ(PopNumber(edge), FIRST);
    EXPECT_EQ(PopNumber(edge), SECOND);
}

TEST_F(EdgeTest, Block)
{
    // With one slot, the producer has to wait for every single Pop().
    Edge edge("edge", {.capacity = 1, .overflow = Overflow::Block});

    std::thread producer(
        [&edge]
        {
            // EXPECT, not ASSERT: a failed Push() must not skip Close(), or
            // the consumer below would wait forever.
            for (int number = 0; number < PAYLOAD_COUNT; ++number)
                EXPECT_EQ(edge.Push(MakePayload(number)), Status::Ok);
            edge.Close();
        });

    int expected = 0;
    std::shared_ptr<const Payload> payload;

    while (edge.Pop(payload) == Status::Ok)
        EXPECT_EQ(NumberOf(payload), expected++);
    producer.join();

    EXPECT_EQ(expected, PAYLOAD_COUNT);
    EXPECT_EQ(edge.DroppedCount(), 0U);
}

TEST_F(EdgeTest, CloseWakesABlockedPop)
{
    Edge edge("edge", {});
    Status popped = Status::Ok;

    std::thread consumer([&edge, &popped] { popped = PopStatus(edge); });

    edge.Close();
    consumer.join();

    EXPECT_EQ(popped, Status::EndOfStream);
}

TEST_F(EdgeTest, AbortWakesABlockedPush)
{
    Edge edge("edge", {.capacity = 1, .overflow = Overflow::Block});
    PushAll(edge, {FIRST});
    Status pushed = Status::Ok;

    std::thread producer([&edge, &pushed]
                         { pushed = edge.Push(MakePayload(SECOND)); });

    edge.Abort();
    producer.join();

    EXPECT_EQ(pushed, Status::Aborted);
}
