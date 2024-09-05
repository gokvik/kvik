#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include "kvik/errors.hpp"
#include "kvik/local_broker.hpp"
#include "kvik/node.hpp"
#include "kvik/remote_msg.hpp"

using namespace kvik;
using namespace std::chrono_literals;

static const std::string TOPIC = "549b3d00da16ca2d/abc";
static const std::string TOPIC_FOR_WILDCARDS = "549b3d00da16ca2d/111/2223/abc";
static const std::string TOPIC_SINGLE_WILDCARD = "549b3d00da16ca2d/111/+/abc";
static const std::string TOPIC_MULTI_WILDCARD = "549b3d00da16ca2d/111/#";
static const RemoteMsg MSG_PUBLISH = {
    .topic = TOPIC,
    .payload = "123",
};
static const RemoteMsg MSG_PUBLISH_FOR_WILDCARD = {
    .topic = TOPIC_FOR_WILDCARDS,
    .payload = "123",
};

TEST_CASE("Check return values", "[LocalBroker]")
{
    LocalBroker lb;

    SECTION("Publish")
    {
        REQUIRE(lb.publish(MSG_PUBLISH) == ErrCode::SUCCESS);
    }

    SECTION("Subscribe")
    {
        REQUIRE(lb.subscribe(TOPIC) == ErrCode::SUCCESS);
    }

    SECTION("Unsubscribe without preceding subscribe")
    {
        REQUIRE(lb.unsubscribe(TOPIC) == ErrCode::NOT_FOUND);
    }

    SECTION("Unsubscribe without preceding subscribe - wildcard")
    {
        REQUIRE(lb.unsubscribe(TOPIC + "/#") == ErrCode::NOT_FOUND);
    }

    SECTION("Unsubscribe with preceding subscribe")
    {
        REQUIRE(lb.subscribe(TOPIC) == ErrCode::SUCCESS);
        REQUIRE(lb.unsubscribe(TOPIC) == ErrCode::SUCCESS);
    }

    SECTION("Unsubscribe with preceding subscribe - wildcard")
    {
        REQUIRE(lb.subscribe(TOPIC + "/#") == ErrCode::SUCCESS);
        REQUIRE(lb.unsubscribe(TOPIC + "/#") == ErrCode::SUCCESS);
    }
}

TEST_CASE("Receive subscription data", "[LocalBroker]")
{
    int calledCnt = 0;
    RemoteMsg recvMsg;

    LocalBroker lb;
    lb.setRecvCb([&calledCnt, &recvMsg](const RemoteMsg &msg) -> ErrCode
                 {
            calledCnt++;
            recvMsg = msg;
            return ErrCode::SUCCESS; });

    SECTION("Publish, don't receive")
    {
        REQUIRE(lb.publish(MSG_PUBLISH) == ErrCode::SUCCESS);
        CHECK(calledCnt == 0);
    }

    SECTION("Subscribe, publish, receive")
    {
        REQUIRE(lb.subscribe(MSG_PUBLISH.topic) == ErrCode::SUCCESS);
        REQUIRE(lb.publish(MSG_PUBLISH) == ErrCode::SUCCESS);
        CHECK(calledCnt == 1);
        CHECK(recvMsg == MSG_PUBLISH);
    }

    SECTION("Subscribe, publish, receive - single level wildcard")
    {
        REQUIRE(lb.subscribe(TOPIC_SINGLE_WILDCARD) == ErrCode::SUCCESS);
        REQUIRE(lb.publish(MSG_PUBLISH_FOR_WILDCARD) == ErrCode::SUCCESS);
        CHECK(calledCnt == 1);
        CHECK(recvMsg == MSG_PUBLISH_FOR_WILDCARD);
    }

    SECTION("Subscribe, publish, receive - multi level wildcard")
    {
        REQUIRE(lb.subscribe(TOPIC_MULTI_WILDCARD) == ErrCode::SUCCESS);
        REQUIRE(lb.publish(MSG_PUBLISH_FOR_WILDCARD) == ErrCode::SUCCESS);
        CHECK(calledCnt == 1);
        CHECK(recvMsg == MSG_PUBLISH_FOR_WILDCARD);
    }

    SECTION("Subscribe, unsubscribe, publish, don't receive")
    {
        REQUIRE(lb.subscribe(MSG_PUBLISH.topic) == ErrCode::SUCCESS);
        REQUIRE(lb.unsubscribe(MSG_PUBLISH.topic) == ErrCode::SUCCESS);
        REQUIRE(lb.publish(MSG_PUBLISH) == ErrCode::SUCCESS);
        CHECK(calledCnt == 0);
    }

    SECTION("Subscribe, unsubscribe, publish, don't receive - single level wildcard")
    {
        REQUIRE(lb.subscribe(TOPIC_SINGLE_WILDCARD) == ErrCode::SUCCESS);
        REQUIRE(lb.unsubscribe(TOPIC_SINGLE_WILDCARD) == ErrCode::SUCCESS);
        REQUIRE(lb.publish(MSG_PUBLISH_FOR_WILDCARD) == ErrCode::SUCCESS);
        CHECK(calledCnt == 0);
    }

    SECTION("Subscribe, unsubscribe, publish, don't receive - multi level wildcard")
    {
        REQUIRE(lb.subscribe(TOPIC_MULTI_WILDCARD) == ErrCode::SUCCESS);
        REQUIRE(lb.unsubscribe(TOPIC_MULTI_WILDCARD) == ErrCode::SUCCESS);
        REQUIRE(lb.publish(MSG_PUBLISH_FOR_WILDCARD) == ErrCode::SUCCESS);
        CHECK(calledCnt == 0);
    }

    SECTION("Subscribe, unsubscribe other, publish, receive")
    {
        REQUIRE(lb.subscribe(TOPIC_MULTI_WILDCARD) == ErrCode::SUCCESS);
        REQUIRE(lb.unsubscribe(TOPIC_SINGLE_WILDCARD) == ErrCode::NOT_FOUND);
        REQUIRE(lb.publish(MSG_PUBLISH_FOR_WILDCARD) == ErrCode::SUCCESS);
        CHECK(calledCnt == 1);
        CHECK(recvMsg == MSG_PUBLISH_FOR_WILDCARD);
    }

    SECTION("overlapping subscriptions")
    {
        REQUIRE(lb.subscribe(TOPIC_SINGLE_WILDCARD) == ErrCode::SUCCESS);
        REQUIRE(lb.subscribe(TOPIC_MULTI_WILDCARD) == ErrCode::SUCCESS);
        REQUIRE(lb.subscribe(TOPIC) == ErrCode::SUCCESS);
        REQUIRE(lb.publish(MSG_PUBLISH_FOR_WILDCARD) == ErrCode::SUCCESS);
        CHECK(calledCnt == 1);
        CHECK(recvMsg == MSG_PUBLISH_FOR_WILDCARD);
    }
}

TEST_CASE("Receive callback returns error", "[LocalBroker]")
{
    LocalBroker lb;
    lb.setRecvCb([](const RemoteMsg &msg) -> ErrCode
                 { return ErrCode::GENERIC_FAILURE; });

    SECTION("Publish, don't receive")
    {
        REQUIRE(lb.publish(MSG_PUBLISH) == ErrCode::SUCCESS);
    }

    SECTION("Subscribe, publish, receive")
    {
        REQUIRE(lb.subscribe(MSG_PUBLISH.topic) == ErrCode::SUCCESS);
        REQUIRE(lb.publish(MSG_PUBLISH) == ErrCode::GENERIC_FAILURE);
    }
}