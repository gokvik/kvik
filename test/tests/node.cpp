/**
 * @file node.cpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @copyright Copyright (c) 2024
 */

#include <unordered_set>

#include <catch2/catch_test_macros.hpp>

#include "kvik/local_addr.hpp"
#include "kvik/node.hpp"
#include "kvik/node_config.hpp"
#include "kvik/pub_sub_struct.hpp"
#include "kvik_testing/dummy_node.hpp"

using namespace kvik;

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

TEST_CASE("Get nonce", "[Node]")
{
    constexpr size_t rounds = 50;

    DummyNode node(DEFAULT_CONFIG);
    std::unordered_set<uint16_t> nonces;

    for (size_t i = 0; i < rounds; i++)
    {
        nonces.insert(node.getNonce());
    }

    // At least one of 1 and 2 is not present
    // (Should detect missing seeding with 0.)
    CHECK((nonces.find(1) == nonces.end() || nonces.find(2) == nonces.end()));

    // One duplicate as reserve
    CHECK(nonces.size() >= rounds - 1);
}

TEST_CASE("Validate peer nonce", "[Node]")
{
    DummyNode node(DEFAULT_CONFIG);
    REQUIRE(node.validateNonce(LocalAddr(), 1));
    REQUIRE(node.validateNonce(LocalAddr(), 2));
    REQUIRE_FALSE(node.validateNonce(LocalAddr(), 1));
    REQUIRE(node.validateNonce(LocalAddr({{0x01}}), 1));
    REQUIRE_FALSE(node.validateNonce(LocalAddr({{0x01}}), 1));
}
