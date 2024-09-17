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

    SECTION("Different publications")
    {
        msg2.pubs.push_back({.topic = "1", .payload = "2"});
        REQUIRE(msg1 != msg2);
    }

    SECTION("Different subscriptions")
    {
        msg2.subs.push_back("1");
        REQUIRE(msg1 != msg2);
    }

    SECTION("Different unsubscriptions")
    {
        msg2.unsubs.push_back("1");
        REQUIRE(msg1 != msg2);
    }

    SECTION("Different subscriptions data")
    {
        msg2.subsData.push_back({.topic = "1", .payload = "2"});
        REQUIRE(msg1 != msg2);
    }

    SECTION("Different IDs")
    {
        // Just additional data, no difference in comparison
        msg2.id = 1;
        REQUIRE(msg1 == msg2);
    }

    SECTION("Different fail reasons")
    {
        // Just additional data, no difference in comparison
        msg2.failReason = LocalMsgFailReason::DUP_ID;
        REQUIRE(msg1 == msg2);
    }
}

TEST_CASE("String representation", "[LocalMsgType]")
{
    auto strEq = [](LocalMsgType type,
                    std::string str) -> bool {
        return std::string(localMsgTypeToStr(type)) == str;
    };

    CHECK(strEq(LocalMsgType::NONE, "NONE"));
    CHECK(strEq(LocalMsgType::OK, "OK"));
    CHECK(strEq(LocalMsgType::FAIL, "FAIL"));
    CHECK(strEq(LocalMsgType::PROBE_REQ, "PROBE_REQ"));
    CHECK(strEq(LocalMsgType::PROBE_RES, "PROBE_RES"));
    CHECK(strEq(LocalMsgType::PUB_SUB_UNSUB, "PUB_SUB_UNSUB"));
    CHECK(strEq(LocalMsgType::SUB_DATA, "SUB_DATA"));

    // Invalid value
    CHECK(strEq(static_cast<LocalMsgType>(-1), "???"));
}

TEST_CASE("String representation", "[LocalMsgFailReason]")
{
    auto strEq = [](LocalMsgFailReason reason, std::string str) -> bool {
        return std::string(localMsgFailReasonToStr(reason)) == str;
    };

    CHECK(strEq(LocalMsgFailReason::NONE, "NONE"));
    CHECK(strEq(LocalMsgFailReason::DUP_ID, "DUP_ID"));
    CHECK(strEq(LocalMsgFailReason::INVALID_TS, "INVALID_TS"));
    CHECK(strEq(LocalMsgFailReason::PROCESSING_FAILED, "PROCESSING_FAILED"));
    CHECK(strEq(LocalMsgFailReason::UNKNOWN_SENDER, "UNKNOWN_SENDER"));

    // Invalid value
    CHECK(strEq(static_cast<LocalMsgFailReason>(-1), "???"));
}
