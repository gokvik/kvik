/**
 * @file node.cpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @copyright Copyright (c) 2024
 */

#include <chrono>
#include <unordered_set>

#include <catch2/catch_test_macros.hpp>

#include "kvik/local_addr.hpp"
#include "kvik/node.hpp"
#include "kvik/node_config.hpp"
#include "kvik/pub_sub_struct.hpp"
#include "kvik_testing/dummy_node.hpp"

using namespace kvik;
using namespace std::chrono_literals;

using PubsLog = DummyNode::PubsLog;
using SubsLog = DummyNode::SubsLog;
using UnsubsLog = DummyNode::UnsubsLog;

// Testing data
static const NodeConfig DEFAULT_CONFIG = {};
static std::string TOPIC1 = "abc";
static std::string TOPIC2 = "def";
static std::string PAYLOAD1 = "payload1";
static std::string PAYLOAD2 = "payload2";
static PubData PUB_DATA1 = {.topic = TOPIC1, .payload = PAYLOAD1};
static PubData PUB_DATA2 = {.topic = TOPIC2, .payload = PAYLOAD2};
static SubReq SUB_REQ1 = {.topic = TOPIC1, .cb = [](const SubData &data) {}};
static SubReq SUB_REQ2 = {.topic = TOPIC2, .cb = [](const SubData &data) {}};

// Publish, subscribe, unsubscribe and resubscribe tests only test requests
// correctly arrive at `pubSubBulk`, `unsubscribeBulk`, `unsubscribeAll` and
// `resubscribeAll` methods that are implemented by various node types (for
// testing purposes by `DummyNode`).

TEST_CASE("Publish", "[Node]")
{
    DummyNode node(DEFAULT_CONFIG);

    SECTION("Single")
    {
        REQUIRE(node.publish(PUB_DATA1.topic, PUB_DATA1.payload) == ErrCode::SUCCESS);
        REQUIRE(node.pubsLog == PubsLog{PUB_DATA1});
    }

    SECTION("Bulk")
    {
        REQUIRE(node.publishBulk({PUB_DATA1, PUB_DATA2}) == ErrCode::SUCCESS);
        REQUIRE(node.pubsLog == PubsLog{PUB_DATA1, PUB_DATA2});
    }
}

TEST_CASE("Subscribe", "[Node]")
{
    DummyNode node(DEFAULT_CONFIG);

    SECTION("Single")
    {
        REQUIRE(node.subscribe(SUB_REQ1.topic, SUB_REQ1.cb) == ErrCode::SUCCESS);
        REQUIRE(node.subsLog == SubsLog{SUB_REQ1});
    }

    SECTION("Bulk")
    {
        REQUIRE(node.subscribeBulk({SUB_REQ1, SUB_REQ2}) == ErrCode::SUCCESS);
        REQUIRE(node.subsLog == SubsLog{SUB_REQ1, SUB_REQ2});
    }
}

TEST_CASE("Unsubscribe", "[Node]")
{
    DummyNode node(DEFAULT_CONFIG);

    SECTION("Single")
    {
        REQUIRE(node.unsubscribe(TOPIC1) == ErrCode::SUCCESS);
        REQUIRE(node.unsubsLog == UnsubsLog{TOPIC1});
    }

    SECTION("Bulk")
    {
        REQUIRE(node.unsubscribeBulk({TOPIC1, TOPIC2}) == ErrCode::SUCCESS);
        REQUIRE(node.unsubsLog == UnsubsLog{TOPIC1, TOPIC2});
    }

    SECTION("All")
    {
        REQUIRE(node.unsubscribeAll() == ErrCode::SUCCESS);
        REQUIRE(node.unsubAllCnt == 1);
    }
}

TEST_CASE("Resubscribe all", "[Node]")
{
    DummyNode node(DEFAULT_CONFIG);
    REQUIRE(node.resubscribeAll() == ErrCode::SUCCESS);
    REQUIRE(node.resubAllCnt == 1);
}

TEST_CASE("Get message ID", "[Node]")
{
    constexpr size_t rounds = 50;

    DummyNode node(DEFAULT_CONFIG);
    std::unordered_set<uint16_t> msgIds;

    for (size_t i = 0; i < rounds; i++) {
        msgIds.insert(node.getMsgId());
    }

    // At least one of 1 and 2 is not present
    // (Should detect missing seeding with 0.)
    CHECK((msgIds.find(1) == msgIds.end() || msgIds.find(2) == msgIds.end()));

    // One duplicate as reserve
    CHECK(msgIds.size() >= rounds - 1);
}

TEST_CASE("Validate peer message ID", "[Node]")
{
    DummyNode node(DEFAULT_CONFIG);
    REQUIRE(node.validateMsgId(LocalAddr(), 1));
    REQUIRE(node.validateMsgId(LocalAddr(), 2));
    REQUIRE_FALSE(node.validateMsgId(LocalAddr(), 1));
    REQUIRE(node.validateMsgId(LocalAddr({{0x01}}), 1));
    REQUIRE_FALSE(node.validateMsgId(LocalAddr({{0x01}}), 1));
}

TEST_CASE("Invalid config", "[Node]")
{
    auto conf = DEFAULT_CONFIG;

    SECTION("Invalid msgIdCache.maxAge")
    {
        conf.msgIdCache.maxAge = 0;
    }

    REQUIRE_THROWS(DummyNode(conf));
}

TEST_CASE("Validate message timestamp", "[Node]")
{
    // Correct calculation
    auto calcMsgTs = [](std::chrono::milliseconds timeUnit,
                        std::chrono::milliseconds tsDiff) -> uint16_t {
        return (std::chrono::steady_clock::now().time_since_epoch() +
                tsDiff) /
               timeUnit;
    };

    NodeConfig conf;
    uint8_t &maxAge = conf.msgIdCache.maxAge;
    std::chrono::milliseconds &timeUnit = conf.msgIdCache.timeUnit;
    std::chrono::milliseconds tsDiff;

    SECTION("Unit 1s, maxAge 3")
    {
        timeUnit = 1s;
        maxAge = 3;

        SECTION("No time difference")
        {
            tsDiff = 0ms;
        }

        SECTION("Sub unit time difference")
        {
            tsDiff = 100ms;
        }

        SECTION("Multi unit negative time difference")
        {
            tsDiff = -3s;
        }

        auto msgTsNow = calcMsgTs(timeUnit, tsDiff);
        DummyNode node(conf);

        // Future timestamp is rejected
        CHECK_FALSE(node.validateMsgTimestamp(msgTsNow + 2, tsDiff));
        CHECK_FALSE(node.validateMsgTimestamp(msgTsNow + 1, tsDiff));

        // Timestamp in range [now - (maxAge - 1), now] is accepted
        CHECK(node.validateMsgTimestamp(msgTsNow, tsDiff));
        CHECK(node.validateMsgTimestamp(msgTsNow - 1, tsDiff));
        CHECK(node.validateMsgTimestamp(msgTsNow - 2, tsDiff));

        // Older timestamp is rejected
        CHECK_FALSE(node.validateMsgTimestamp(msgTsNow - 3, tsDiff));
        CHECK_FALSE(node.validateMsgTimestamp(msgTsNow - 4, tsDiff));
    }

    SECTION("Unit 10ms, maxAge 1, tsDiff 0ms")
    {
        timeUnit = 10ms;
        maxAge = 1;
        tsDiff = 0ms;

        auto msgTsNow = calcMsgTs(timeUnit, tsDiff);
        DummyNode node(conf);

        // Future timestamp is rejected
        CHECK_FALSE(node.validateMsgTimestamp(msgTsNow + 2, tsDiff));
        CHECK_FALSE(node.validateMsgTimestamp(msgTsNow + 1, tsDiff));

        // Only current timestamp is accepted (as maxAge is 1)
        CHECK(node.validateMsgTimestamp(msgTsNow, tsDiff));

        // Older timestamp is rejected
        CHECK_FALSE(node.validateMsgTimestamp(msgTsNow - 1, tsDiff));
        CHECK_FALSE(node.validateMsgTimestamp(msgTsNow - 2, tsDiff));
    }
}
