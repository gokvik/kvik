/**
 * @file pub_sub_struct.cpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @copyright Copyright (c) 2024
 */

#include "kvik/pub_sub_struct.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace kvik;

TEST_CASE("Comparison", "[SubData]")
{
    SubData data1;
    SubData data2;

    SECTION("Equality")
    {
        REQUIRE(data1 == data2);
    }

    SECTION("Different topics")
    {
        data2.topic = "1";
        REQUIRE(data1 != data2);
    }

    SECTION("Different payloads")
    {
        data2.payload = "1";
        REQUIRE(data1 != data2);
    }
}

TEST_CASE("Comparison", "[PubData]")
{
    PubData data1;
    PubData data2;

    SECTION("Equality")
    {
        REQUIRE(data1 == data2);
    }

    SECTION("Different topics")
    {
        data2.topic = "1";
        REQUIRE(data1 != data2);
    }

    SECTION("Different payloads")
    {
        data2.payload = "1";
        REQUIRE(data1 != data2);
    }
}

TEST_CASE("Conversion of PubData to SubData", "[PubData]")
{
    PubData pubData = {
        .topic = "aaa",
        .payload = "123",
    };
    SubData subDataCorrect = {
        .topic = "aaa",
        .payload = "123",
    };
    REQUIRE(subDataCorrect == pubData.toSubData());
}
