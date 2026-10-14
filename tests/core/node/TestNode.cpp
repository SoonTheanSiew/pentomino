// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#include "graph/Edge.hpp"
#include "support/DummyNodes.hpp"

#include <cstddef>
#include <vector>

#include <gtest/gtest.h>

using namespace Pento;

class NodeTest : public ::testing::Test
{
  protected:
    static constexpr std::size_t ONE_OUTPUT = 1;
    static constexpr std::size_t TWO_OUTPUTS = 2;

    static constexpr std::size_t FIRST_INPUT = 0;
    static constexpr std::size_t VIDEO_OUTPUT = 0;
    static constexpr std::size_t AUDIO_OUTPUT = 1;

    static constexpr int ONE_PAYLOAD = 1;
    static constexpr int TWO_PAYLOADS = 2;
    static constexpr int FIRST_PAYLOAD = 1;
    static constexpr int SECOND_PAYLOAD = 2;
};

TEST_F(NodeTest, PipelineRunsToEnd)
{
    Edge sourceToDecode("source -> decode");
    Edge decodeToSink("decode -> sink");

    DummySource source(TWO_PAYLOADS);
    DummyDecode decode;
    DummySink sink;

    const Status sourceOut = source.ConnectOutput(VIDEO_OUTPUT, sourceToDecode);
    const Status decodeIn = decode.ConnectInput(FIRST_INPUT, sourceToDecode);
    const Status decodeOut = decode.ConnectOutput(VIDEO_OUTPUT, decodeToSink);
    const Status sinkIn = sink.ConnectInput(FIRST_INPUT, decodeToSink);

    ASSERT_EQ(sourceOut, Status::Ok);
    ASSERT_EQ(decodeIn, Status::Ok);
    ASSERT_EQ(decodeOut, Status::Ok);
    ASSERT_EQ(sinkIn, Status::Ok);

    const Status sourceEnd = RunToEnd(source);
    const Status decodeEnd = RunToEnd(decode);
    const Status sinkEnd = RunToEnd(sink);

    const std::vector<int> expected = {FIRST_PAYLOAD, SECOND_PAYLOAD};
    EXPECT_EQ(sourceEnd, Status::EndOfStream);
    EXPECT_EQ(decodeEnd, Status::EndOfStream);
    EXPECT_EQ(sinkEnd, Status::EndOfStream);
    EXPECT_EQ(sink.Received(), expected);
}

TEST_F(NodeTest, DemuxSplitsStreams)
{
    Edge sourceToDemux("source -> demux");
    Edge demuxToVideo("demux.0 -> video");
    Edge demuxToAudio("demux.1 -> audio");

    DummySource source(TWO_PAYLOADS);
    DummyDemux demux(TWO_OUTPUTS);
    DummySink video;
    DummySink audio;

    const Status sourceOut = source.ConnectOutput(VIDEO_OUTPUT, sourceToDemux);
    const Status demuxIn = demux.ConnectInput(FIRST_INPUT, sourceToDemux);
    const Status videoOut = demux.ConnectOutput(VIDEO_OUTPUT, demuxToVideo);
    const Status audioOut = demux.ConnectOutput(AUDIO_OUTPUT, demuxToAudio);
    const Status videoIn = video.ConnectInput(FIRST_INPUT, demuxToVideo);
    const Status audioIn = audio.ConnectInput(FIRST_INPUT, demuxToAudio);

    ASSERT_EQ(sourceOut, Status::Ok);
    ASSERT_EQ(demuxIn, Status::Ok);
    ASSERT_EQ(videoOut, Status::Ok);
    ASSERT_EQ(audioOut, Status::Ok);
    ASSERT_EQ(videoIn, Status::Ok);
    ASSERT_EQ(audioIn, Status::Ok);

    const Status sourceEnd = RunToEnd(source);
    const Status demuxEnd = RunToEnd(demux);
    const Status videoEnd = RunToEnd(video);
    const Status audioEnd = RunToEnd(audio);

    const std::vector<int> videoExpected = {FIRST_PAYLOAD};
    const std::vector<int> audioExpected = {SECOND_PAYLOAD};

    EXPECT_EQ(sourceEnd, Status::EndOfStream);
    EXPECT_EQ(demuxEnd, Status::EndOfStream);
    EXPECT_EQ(videoEnd, Status::EndOfStream);
    EXPECT_EQ(audioEnd, Status::EndOfStream);

    EXPECT_EQ(video.Received(), videoExpected);
    EXPECT_EQ(audio.Received(), audioExpected);
}

TEST_F(NodeTest, OneOutputFeedsTwoSinks)
{
    Edge sourceToLeft("source -> left");
    Edge sourceToRight("source -> right");

    DummySource source(ONE_PAYLOAD);
    DummySink left;
    DummySink right;

    const Status leftOut = source.ConnectOutput(VIDEO_OUTPUT, sourceToLeft);
    const Status rightOut = source.ConnectOutput(VIDEO_OUTPUT, sourceToRight);
    const Status leftIn = left.ConnectInput(FIRST_INPUT, sourceToLeft);
    const Status rightIn = right.ConnectInput(FIRST_INPUT, sourceToRight);

    ASSERT_EQ(leftOut, Status::Ok);
    ASSERT_EQ(rightOut, Status::Ok);
    ASSERT_EQ(leftIn, Status::Ok);
    ASSERT_EQ(rightIn, Status::Ok);

    const Status sourceEnd = RunToEnd(source);
    const Status leftEnd = RunToEnd(left);
    const Status rightEnd = RunToEnd(right);

    const std::vector<int> expected = {FIRST_PAYLOAD};

    EXPECT_EQ(sourceEnd, Status::EndOfStream);
    EXPECT_EQ(leftEnd, Status::EndOfStream);
    EXPECT_EQ(rightEnd, Status::EndOfStream);

    EXPECT_EQ(left.Received(), expected);
    EXPECT_EQ(right.Received(), expected);
}

TEST_F(NodeTest, UnconnectedOutputDropsPayload)
{
    // A demuxer whose audio nobody wants.
    Edge sourceToDemux("source -> demux");
    Edge demuxToVideo("demux.0 -> video");

    DummySource source(TWO_PAYLOADS);
    DummyDemux demux(TWO_OUTPUTS);
    DummySink video;

    const Status sourceOut = source.ConnectOutput(VIDEO_OUTPUT, sourceToDemux);
    const Status demuxIn = demux.ConnectInput(FIRST_INPUT, sourceToDemux);
    const Status videoOut = demux.ConnectOutput(VIDEO_OUTPUT, demuxToVideo);
    const Status videoIn = video.ConnectInput(FIRST_INPUT, demuxToVideo);

    ASSERT_EQ(sourceOut, Status::Ok);
    ASSERT_EQ(demuxIn, Status::Ok);
    ASSERT_EQ(videoOut, Status::Ok);
    ASSERT_EQ(videoIn, Status::Ok);

    const Status sourceEnd = RunToEnd(source);
    const Status demuxEnd = RunToEnd(demux);
    const Status videoEnd = RunToEnd(video);

    const std::vector<int> videoExpected = {FIRST_PAYLOAD};

    EXPECT_EQ(sourceEnd, Status::EndOfStream);
    EXPECT_EQ(demuxEnd, Status::EndOfStream);
    EXPECT_EQ(videoEnd, Status::EndOfStream);

    EXPECT_EQ(video.Received(), videoExpected);
}

TEST_F(NodeTest, MissingOutputFails)
{
    // A demuxer that gets audio but has only a video output.
    Edge sourceToDemux("source -> demux");
    DummySource source(TWO_PAYLOADS);
    DummyDemux demux(ONE_OUTPUT);

    const Status sourceOut = source.ConnectOutput(VIDEO_OUTPUT, sourceToDemux);
    const Status demuxIn = demux.ConnectInput(FIRST_INPUT, sourceToDemux);
    ASSERT_EQ(sourceOut, Status::Ok);
    ASSERT_EQ(demuxIn, Status::Ok);

    const Status sourceEnd = RunToEnd(source);
    const Status demuxEnd = RunToEnd(demux);

    EXPECT_EQ(sourceEnd, Status::EndOfStream);
    EXPECT_EQ(demuxEnd, Status::InvalidOperation);
}

TEST_F(NodeTest, AbortedInputStopsSink)
{
    Edge input("source -> sink");
    DummySink sink;

    const Status sinkIn = sink.ConnectInput(FIRST_INPUT, input);
    ASSERT_EQ(sinkIn, Status::Ok);

    input.Abort();

    const Status stepped = sink.Step();

    EXPECT_EQ(stepped, Status::Aborted);
    EXPECT_TRUE(sink.Received().empty());
}

TEST_F(NodeTest, AbortedOutputStopsSource)
{
    // Graph aborts every edge when the pipeline stops on an error.
    Edge output("source -> sink");

    DummySource source(TWO_PAYLOADS);
    const Status sourceOut = source.ConnectOutput(VIDEO_OUTPUT, output);
    ASSERT_EQ(sourceOut, Status::Ok);

    output.Abort();

    const Status stepped = source.Step();

    EXPECT_EQ(stepped, Status::Aborted);
}

TEST_F(NodeTest, UnconnectedInputFails)
{
    DummyDecode decode;

    const Status stepped = decode.Step();

    EXPECT_EQ(stepped, Status::InvalidOperation);
}

TEST_F(NodeTest, BadConnectFails)
{
    Edge first("first");
    Edge second("second");
    DummySource source(ONE_PAYLOAD);
    DummySink sink;

    const Status connected = sink.ConnectInput(FIRST_INPUT, first);
    ASSERT_EQ(connected, Status::Ok);

    const Status inputOnSource = source.ConnectInput(FIRST_INPUT, first);
    const Status secondInput = sink.ConnectInput(FIRST_INPUT, second);
    const Status outputOnSink = sink.ConnectOutput(VIDEO_OUTPUT, second);

    EXPECT_EQ(inputOnSource, Status::InvalidOperation);
    EXPECT_EQ(secondInput, Status::InvalidOperation);
    EXPECT_EQ(outputOnSink, Status::InvalidOperation);
}
