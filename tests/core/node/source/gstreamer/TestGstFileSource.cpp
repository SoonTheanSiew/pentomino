// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#include "graph/Edge.hpp"
#include "node/source/gstreamer/GstFileSource.hpp"
#include "payload/Payload.hpp"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <variant>
#include <vector>

#include <gtest/gtest.h>

using namespace Pento;

class GstFileSourceTest : public ::testing::Test
{
  protected:
    static constexpr std::size_t OUTPUT_INDEX_0 = 0;
    static constexpr std::size_t CHUNK_SIZE = 4096;
    static constexpr std::size_t FILE_SIZE = 10000;
    static constexpr std::size_t CHUNK_COUNT = 3;
    static constexpr std::size_t LAST_CHUNK_SIZE =
        FILE_SIZE - CHUNK_SIZE * (CHUNK_COUNT - 1);
    // Bytes follow i % 251, so every chunk holds different bytes and a
    // repeated, skipped or reordered chunk changes what is received.
    static constexpr std::size_t BYTE_PERIOD = 251;

    void SetUp() override
    {
        m_bytes.resize(FILE_SIZE);

        for (std::size_t i = 0; i < m_bytes.size(); ++i)
            m_bytes[i] = static_cast<std::uint8_t>(i % BYTE_PERIOD);

        std::ofstream file(m_path, std::ios::binary);
        file.write(reinterpret_cast<const char *>(m_bytes.data()),
                   static_cast<std::streamsize>(m_bytes.size()));
    }

    void TearDown() override
    {
        std::filesystem::remove(m_path);
    }

    std::filesystem::path m_path =
        std::filesystem::temp_directory_path() / "PentoGstFileSourceTest.bin";
    std::vector<std::uint8_t> m_bytes;
};

TEST_F(GstFileSourceTest, ReadsWholeFileInChunks)
{
    Edge output("output");
    GstFileSource source("source",
                         {.path = m_path.string(), .chunkSize = CHUNK_SIZE});

    Status status = source.ConnectOutput(OUTPUT_INDEX_0, output);
    ASSERT_EQ(status, Status::Ok);

    status = source.Init();
    ASSERT_EQ(status, Status::Ok);

    std::vector<std::uint8_t> received;
    std::vector<std::size_t> chunkSizes;

    while ((status = source.Step()) == Status::Ok)
    {
        std::shared_ptr<const Payload> payload;
        status = output.Pop(payload);
        ASSERT_EQ(status, Status::Ok);

        const auto &bitstream =
            std::get<Bitstream::SPtr>(payload->GetContent());
        chunkSizes.push_back(bitstream->bytes.size());
        received.insert(received.end(), bitstream->bytes.begin(),
                        bitstream->bytes.end());
    }

    source.Terminate();

    const std::vector<std::size_t> expectedSizes = {CHUNK_SIZE, CHUNK_SIZE,
                                                    LAST_CHUNK_SIZE};
    EXPECT_EQ(status, Status::EndOfStream);
    EXPECT_EQ(received, m_bytes);
    EXPECT_EQ(chunkSizes, expectedSizes);
}

TEST_F(GstFileSourceTest, MissingFileFails)
{
    GstFileSource source("source", {.path = m_path.string() + ".missing"});

    const Status status = source.Init();

    EXPECT_EQ(status, Status::FileNotFound);
}
