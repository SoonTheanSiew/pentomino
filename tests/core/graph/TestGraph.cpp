// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#include "graph/Graph.hpp"
#include "support/DummyNodes.hpp"

#include <cstddef>
#include <limits>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

using namespace Pento;

class GraphTest : public ::testing::Test
{
  protected:
    static constexpr std::size_t OUTPUT_COUNT_1 = 1;
    static constexpr std::size_t OUTPUT_COUNT_2 = 2;

    static constexpr std::size_t INPUT_INDEX_0 = 0;
    static constexpr std::size_t AUDIO_OUTPUT_INDEX =
        DummyDemux::AUDIO_OUTPUT_INDEX;

    static constexpr int ONE_PAYLOAD = 1;
    static constexpr int TWO_PAYLOADS = 2;
    static constexpr int NEGATIVE_COUNT = -1;
    static constexpr int ENDLESS = std::numeric_limits<int>::max();
    static constexpr int FIRST_PAYLOAD = 1;
    static constexpr int SECOND_PAYLOAD = 2;
};

TEST_F(GraphTest, PipelineRunsToEnd)
{
    Graph graph;
    auto &source = graph.Add<DummySource>(TWO_PAYLOADS);
    auto &decode = graph.Add<DummyDecode>();
    auto &sink = graph.Add<DummySink>();
    graph.Connect(source, decode);
    graph.Connect(decode, sink);

    Status status = graph.Init();
    ASSERT_EQ(status, Status::Ok);

    graph.Start();
    status = graph.Wait();
    graph.Terminate();

    const std::vector<int> expected = {FIRST_PAYLOAD, SECOND_PAYLOAD};
    EXPECT_EQ(status, Status::Ok);
    EXPECT_EQ(sink.Received(), expected);
}

TEST_F(GraphTest, DemuxSplitsStreams)
{
    Graph graph;
    auto &source = graph.Add<DummySource>(TWO_PAYLOADS);
    auto &demux = graph.Add<DummyDemux>(OUTPUT_COUNT_2);
    auto &video = graph.Add<DummySink>();
    auto &audio = graph.Add<DummySink>();
    graph.Connect(source, demux);
    graph.Connect(demux, video);
    graph.Connect(demux, AUDIO_OUTPUT_INDEX, audio, INPUT_INDEX_0);

    Status status = graph.Init();
    ASSERT_EQ(status, Status::Ok);

    graph.Start();
    status = graph.Wait();
    graph.Terminate();

    const std::vector<int> videoExpected = {FIRST_PAYLOAD};
    const std::vector<int> audioExpected = {SECOND_PAYLOAD};
    EXPECT_EQ(status, Status::Ok);
    EXPECT_EQ(video.Received(), videoExpected);
    EXPECT_EQ(audio.Received(), audioExpected);
}

TEST_F(GraphTest, OneOutputFeedsTwoSinks)
{
    Graph graph;
    auto &source = graph.Add<DummySource>(ONE_PAYLOAD);
    auto &sink1 = graph.Add<DummySink>();
    auto &sink2 = graph.Add<DummySink>();
    graph.Connect(source, sink1);
    graph.Connect(source, sink2);

    Status status = graph.Init();
    ASSERT_EQ(status, Status::Ok);

    graph.Start();
    status = graph.Wait();
    graph.Terminate();

    const std::vector<int> expected = {FIRST_PAYLOAD};
    EXPECT_EQ(status, Status::Ok);
    EXPECT_EQ(sink1.Received(), expected);
    EXPECT_EQ(sink2.Received(), expected);
}

TEST_F(GraphTest, FailedNodeStopsGraph)
{
    // The demux sends to output 1, which it does not have. The error stops
    // the whole graph, the endless source too.
    Graph graph;
    auto &source = graph.Add<DummySource>(ENDLESS);
    auto &demux = graph.Add<DummyDemux>(OUTPUT_COUNT_1);
    auto &video = graph.Add<DummySink>();
    graph.Connect(source, demux);
    graph.Connect(demux, video);

    Status status = graph.Init();
    ASSERT_EQ(status, Status::Ok);

    graph.Start();
    status = graph.Wait();
    graph.Terminate();

    EXPECT_EQ(status, Status::InvalidOperation);
}

TEST_F(GraphTest, StopEndsGraph)
{
    Graph graph;
    auto &source = graph.Add<DummySource>(ENDLESS);
    auto &sink = graph.Add<DummySink>();
    graph.Connect(source, sink);

    Status status = graph.Init();
    ASSERT_EQ(status, Status::Ok);

    graph.Start();
    status = graph.Stop();
    graph.Terminate();

    EXPECT_EQ(status, Status::Ok);
}

TEST_F(GraphTest, StopEndsWait)
{
    Graph graph;
    auto &source = graph.Add<DummySource>(ENDLESS);
    auto &sink = graph.Add<DummySink>();
    graph.Connect(source, sink);

    Status status = graph.Init();
    ASSERT_EQ(status, Status::Ok);

    // Stop() comes from another thread while Wait() blocks, as on Ctrl+C.
    graph.Start();
    std::thread stopper(&Graph::Stop, &graph);
    status = graph.Wait();
    stopper.join();
    graph.Terminate();

    EXPECT_EQ(status, Status::Ok);
}

TEST_F(GraphTest, FailedInitReturnsStatus)
{
    Graph graph;
    auto &source = graph.Add<DummySource>(NEGATIVE_COUNT);
    auto &sink = graph.Add<DummySink>();
    graph.Connect(source, sink);

    const Status status = graph.Init();

    EXPECT_EQ(status, Status::InvalidParam);
}

TEST_F(GraphTest, BadConnectFailsInit)
{
    // Two edges into the one input of a sink.
    Graph graph;
    auto &first = graph.Add<DummySource>(ONE_PAYLOAD);
    auto &second = graph.Add<DummySource>(ONE_PAYLOAD);
    auto &sink = graph.Add<DummySink>();
    graph.Connect(first, sink);
    graph.Connect(second, sink);

    const Status status = graph.Init();

    EXPECT_EQ(status, Status::InvalidOperation);
}

TEST_F(GraphTest, UnconnectedSourceFailsInit)
{
    Graph graph;
    graph.Add<DummySource>(ONE_PAYLOAD);

    const Status status = graph.Init();

    EXPECT_EQ(status, Status::InvalidOperation);
}
