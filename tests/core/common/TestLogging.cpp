// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#include "common/Logging.hpp"

#include <string>
#include <utility>

#include <gtest/gtest.h>

using namespace Pento;

// Makes the constructor and Fail() callable from the test.
class TestLogging : public Logging
{
  public:
    explicit TestLogging(std::string name) : Logging(std::move(name))
    {
    }

    using Logging::Fail;
};

TEST(LoggingTest, FailReturnsStatus)
{
    TestLogging object("decode");

    EXPECT_EQ(object.Name(), "decode");
    EXPECT_EQ(object.Fail(Status::FileNotFound, "input.mp4"),
              Status::FileNotFound);
}

TEST(LoggingTest, NameIsSetOnlyIfEmpty)
{
    TestLogging named("decode");
    TestLogging unnamed("");

    named.SetNameIfEmpty("VplDecode-0");
    unnamed.SetNameIfEmpty("VplDecode-1");

    EXPECT_EQ(named.Name(), "decode");
    EXPECT_EQ(unnamed.Name(), "VplDecode-1");
}
