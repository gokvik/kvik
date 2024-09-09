/**
 * @file local_msg.cpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @copyright Copyright (c) 2024
 */

#include "kvik/local_msg.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace kvik;

TEST_CASE("Comparison", "[LocalMsg]")
{
    LocalMsg msg1;
    LocalMsg msg2;

    SECTION("Equality")
    {
        REQUIRE(msg1 == msg2);
    }

    SECTION("Different types")
    {
        msg2.type = LocalMsgType::FAIL;
        REQUIRE(msg1 != msg2);
    }

    SECTION("Different addresses")
    {
        msg2.addr.addr.push_back(0x01);
        REQUIRE(msg1 != msg2);
    }

    SECTION("Different relayed addresses")
    {
        msg2.relayedAddr.addr.push_back(0x01);
        REQUIRE(msg1 != msg2);
    }

    SECTION("Different topics")
    {
        msg2.topic = "1";
        REQUIRE(msg1 != msg2);
    }

    SECTION("Different payloads")
    {
        msg2.payload = "1";
        REQUIRE(msg1 != msg2);
    }

    SECTION("Different fail reasons")
    {
        // Just additional data, no difference in comparison
        msg2.failReason = LocalMsgFailReason::DUP_ID;
        REQUIRE(msg1 == msg2);
    }
}
