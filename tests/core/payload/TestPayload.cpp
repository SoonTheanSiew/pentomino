// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#include "payload/Payload.hpp"

#include <chrono>
#include <cstdint>
#include <memory>
#include <variant>

#include <gtest/gtest.h>

using namespace Pento;

// Builds payloads the way nodes do, so each test only shows its scenario.
class PayloadTest : public ::testing::Test
{
  protected:
    static constexpr std::chrono::nanoseconds FRAME_TIME{33'000'000};
    static constexpr std::uint32_t DECODED_SURFACE_ID = 42;
    static constexpr std::uint32_t SCALED_SURFACE_ID = 43;
    static constexpr std::uint32_t NV12_FOURCC = 0x3231564E;  // VA_FOURCC_NV12
    static constexpr std::uint32_t FRAME_WIDTH = 1920;
    static constexpr std::uint32_t FRAME_HEIGHT = 1080;
    static constexpr int PERSON_COUNT = 3;
    static constexpr int ADULT_COUNT = 2;

    // Stand-ins: these tests are about Payload, not about real metadata.
    struct DummyDetections : Metadata
    {
        explicit DummyDetections(int value) : count(value)
        {
        }
        int count;
    };

    struct DummyClassifications : Metadata
    {
        explicit DummyClassifications(int value) : count(value)
        {
        }
        int count;
    };

    // A surface the way a decoder hands it out: the deleter stands in for
    // returning it to the decoder's pool, and sets isReleased when that
    // happens.
    static VaSurface::SPtr MakeSurface(std::uint32_t surface,
                                       bool *isReleased = nullptr)
    {
        return VaSurface::SPtr(new VaSurface{.surface = surface,
                                             .fourcc = NV12_FOURCC,
                                             .width = FRAME_WIDTH,
                                             .height = FRAME_HEIGHT},
                               [isReleased](const VaSurface *frame)
                               {
                                   if (isReleased != nullptr)
                                       *isReleased = true;
                                   delete frame;
                               });
    }

    static std::shared_ptr<const Payload> MakeFramePayload()
    {
        return std::make_shared<const Payload>(FRAME_TIME,
                                               MakeSurface(DECODED_SURFACE_ID));
    }

    static const VaSurface *SurfaceOf(const Payload &payload)
    {
        const auto *frame = std::get_if<VaSurface::SPtr>(&payload.GetContent());
        return frame != nullptr ? frame->get() : nullptr;
    }
};

TEST_F(PayloadTest, KeepsTimestampAndContent)
{
    auto surface = MakeSurface(DECODED_SURFACE_ID);
    const Payload payload(FRAME_TIME, surface);

    EXPECT_EQ(payload.Timestamp(), FRAME_TIME);
    EXPECT_EQ(SurfaceOf(payload), surface.get());
    EXPECT_EQ(SurfaceOf(payload)->fourcc, NV12_FOURCC);
    EXPECT_EQ(SurfaceOf(payload)->width, FRAME_WIDTH);
    EXPECT_EQ(SurfaceOf(payload)->height, FRAME_HEIGHT);
}

TEST_F(PayloadTest, ContentReadAsAnotherTypeIsNull)
{
    auto payload = MakeFramePayload();

    EXPECT_EQ(std::get_if<DmaBuf::SPtr>(&payload->GetContent()), nullptr);
}

TEST_F(PayloadTest, NewPayloadHasNoMetadata)
{
    auto payload = MakeFramePayload();

    EXPECT_EQ(payload->Find<DummyDetections>(), nullptr);
}

TEST_F(PayloadTest, AttachReturnsACopyWithTheMetadata)
{
    auto payload = MakeFramePayload();
    auto detected =
        payload->Attach(std::make_shared<DummyDetections>(PERSON_COUNT));

    // The copy has the metadata, and the same timestamp and content.
    ASSERT_NE(detected->Find<DummyDetections>(), nullptr);
    EXPECT_EQ(detected->Find<DummyDetections>()->count, PERSON_COUNT);
    EXPECT_EQ(detected->Timestamp(), FRAME_TIME);
    EXPECT_EQ(SurfaceOf(*detected), SurfaceOf(*payload));

    // The original is untouched: other nodes may still be reading it.
    EXPECT_EQ(payload->Find<DummyDetections>(), nullptr);
}

TEST_F(PayloadTest, MetadataAccumulatesAlongThePipeline)
{
    // detect adds DummyDetections, classify adds DummyClassifications.
    auto decoded = MakeFramePayload();
    auto detected =
        decoded->Attach(std::make_shared<DummyDetections>(PERSON_COUNT));
    auto classified =
        detected->Attach(std::make_shared<DummyClassifications>(ADULT_COUNT));

    ASSERT_NE(classified->Find<DummyDetections>(), nullptr);
    ASSERT_NE(classified->Find<DummyClassifications>(), nullptr);
    EXPECT_EQ(classified->Find<DummyDetections>()->count, PERSON_COUNT);
    EXPECT_EQ(classified->Find<DummyClassifications>()->count, ADULT_COUNT);
    EXPECT_EQ(detected->Find<DummyClassifications>(), nullptr);
}

TEST_F(PayloadTest, FindPicksTheFirstOfTheSameType)
{
    auto payload = MakeFramePayload()
                       ->Attach(std::make_shared<DummyDetections>(PERSON_COUNT))
                       ->Attach(std::make_shared<DummyDetections>(ADULT_COUNT));

    EXPECT_EQ(payload->Find<DummyDetections>()->count, PERSON_COUNT);
}

TEST_F(PayloadTest, ContentIsReleasedWithTheLastPayload)
{
    bool isReleased = false;
    auto payload = std::make_shared<const Payload>(
        FRAME_TIME, MakeSurface(DECODED_SURFACE_ID, &isReleased));
    auto detected =
        payload->Attach(std::make_shared<DummyDetections>(PERSON_COUNT));

    payload.reset();
    EXPECT_FALSE(isReleased);

    detected.reset();
    EXPECT_TRUE(isReleased);
}

TEST_F(PayloadTest, WithContentReturnsACopyWithTheNewContent)
{
    // decode, detect, then scale: the boxes must survive the scaling.
    auto decoded = MakeFramePayload();
    auto detected =
        decoded->Attach(std::make_shared<DummyDetections>(PERSON_COUNT));
    auto scaled = MakeSurface(SCALED_SURFACE_ID);
    auto rescaled = detected->WithContent(scaled);

    // The copy has the new content, and the same timestamp and metadata.
    EXPECT_EQ(SurfaceOf(*rescaled), scaled.get());
    EXPECT_EQ(rescaled->Timestamp(), FRAME_TIME);
    ASSERT_NE(rescaled->Find<DummyDetections>(), nullptr);
    EXPECT_EQ(rescaled->Find<DummyDetections>()->count, PERSON_COUNT);

    // The original still has the decoded surface.
    EXPECT_EQ(SurfaceOf(*detected)->surface, DECODED_SURFACE_ID);
}

TEST_F(PayloadTest, WithContentLetsTheOldContentGo)
{
    // Once the scaled frame exists and the decoded payload is dropped, the
    // decoded surface goes back to the decoder even while the scaled one
    // lives on.
    bool isReleased = false;
    auto decoded = std::make_shared<const Payload>(
        FRAME_TIME, MakeSurface(DECODED_SURFACE_ID, &isReleased));
    auto rescaled = decoded->WithContent(MakeSurface(SCALED_SURFACE_ID));

    decoded.reset();
    EXPECT_TRUE(isReleased);
    EXPECT_EQ(SurfaceOf(*rescaled)->surface, SCALED_SURFACE_ID);
}
