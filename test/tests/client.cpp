/**
 * @file client.cpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @copyright Copyright (c) 2024
 */

#include <chrono>
#include <thread>

#include <catch2/catch_test_macros.hpp>

#include "kvik/client.hpp"
#include "kvik/client_config.hpp"
#include "kvik_testing/dummy_local_layer.hpp"

using namespace kvik;
using namespace std::chrono_literals;

using SentLog = DummyLocalLayer::SentLog;
using ChannelsLog = DummyLocalLayer::ChannelsLog;
using RespSuccLog = DummyLocalLayer::RespSuccLog;

// Without `do {} while (0)` due to scoping
#define DEFAULT_LL(var)   \
    DummyLocalLayer var;  \
    var.respTsDiff = 0ms; \
    var.respTimeUnit = 10ms;

// Testing data
static const ClientConfig CONF = {
    .nodeConf = {
        .localDelivery = {
            .respTimeout = 20ms,
        },
        .msgIdCache = {
            .timeUnit = 10ms,
            .maxAge = 2,
        },
    },
    .gwDscv = {
        .dscvMinDelay = 5ms,
        .dscvMaxDelay = 1s,
        .initialDscvFailThres = 3,
        .trigMsgsFailCnt = 5,
        .trigTimeSyncNoRespCnt = 2,
    },
    .reporting = {
        .rssiOnGwDscv = false,
    },
    .subDB = {
        .subLifetime = 1s,
    },
    .timeSync = {
        .syncSystemTime = false,
        .reprobeGatewayInterval = 1s,
    },
};

static std::string TOPIC1 = "abc";
static std::string TOPIC2 = "def";
static std::string PAYLOAD1 = "payload1";
static std::string PAYLOAD2 = "payload2";
static PubData PUB_DATA1 = {.topic = TOPIC1, .payload = PAYLOAD1};
static PubData PUB_DATA2 = {.topic = TOPIC2, .payload = PAYLOAD2};
static SubReq SUB_REQ1 = {.topic = TOPIC1, .cb = [](const SubData &data) {}};
static SubReq SUB_REQ2 = {.topic = TOPIC2, .cb = [](const SubData &data) {}};
static SubData SUB_DATA1 = {.topic = TOPIC1, .payload = PAYLOAD1};
static SubData SUB_DATA2 = {.topic = TOPIC2, .payload = PAYLOAD2};

// Peers
static LocalPeer PEER_GW1 = {
    .addr = {.addr = {1}},
    .channel = 1, // Not necessarily used
    .pref = 100,
};
static LocalPeer PEER_GW2 = {
    .addr = {.addr = {2, 1, 2, 3}},
    .channel = 2, // Not necessarily used
    .pref = 200,
};
static LocalPeer PEER_GW3 = {
    .addr = {.addr = {3, 11, 22, 33, 44}},
    .channel = 31, // Not necessarily used
    .pref = 300,
};
static LocalPeer PEER_GW4 = {
    .addr = {.addr = {4, 19, 90, 38, 19}},
    .channel = 99, // Not necessarily used
    .pref = 300,
};
static LocalPeer PEER_RELAY1 = {
    .addr = {.addr = {5, 93, 1, 29}},
    .channel = 42, // Not necessarily used
    .pref = 50,
};

// Probes
static LocalMsg MSG_PROBE_REQ = {
    .type = LocalMsgType::PROBE_REQ,
    .nodeType = NodeType::CLIENT,
};
static LocalMsg MSG_PROBE_REQ_GW1 = {
    .type = LocalMsgType::PROBE_REQ,
    .addr = PEER_GW1.addr,
    .nodeType = NodeType::CLIENT,
};
static LocalMsg MSG_PROBE_RES_GW1 = {
    .type = LocalMsgType::PROBE_RES,
    .addr = PEER_GW1.addr,
    .nodeType = NodeType::GATEWAY,
    .pref = 100,
    .tsDiff = 100ms,
};
static LocalMsg MSG_PROBE_REQ_GW2 = {
    .type = LocalMsgType::PROBE_REQ,
    .addr = PEER_GW2.addr,
    .nodeType = NodeType::CLIENT,
};
static LocalMsg MSG_PROBE_RES_GW2 = {
    .type = LocalMsgType::PROBE_RES,
    .addr = PEER_GW2.addr,
    .nodeType = NodeType::GATEWAY,
    .pref = 200,
};
static LocalMsg MSG_PROBE_RES_GW2_WITH_RSSI = {
    .type = LocalMsgType::PROBE_RES,
    .addr = PEER_GW2.addr,
    .nodeType = NodeType::GATEWAY,
    .rssi = -40,
    .pref = 200,
};
static PubData PUB_DATA_GW2_RSSI = {
    .topic = "_report/rssi/" + PEER_GW2.addr.toString(),
    .payload = "-40",
};
static LocalMsg MSG_PROBE_REQ_GW3 = {
    .type = LocalMsgType::PROBE_REQ,
    .addr = PEER_GW3.addr,
    .nodeType = NodeType::CLIENT,
};
static LocalMsg MSG_PROBE_RES_GW3 = {
    .type = LocalMsgType::PROBE_RES,
    .addr = PEER_GW3.addr,
    .nodeType = NodeType::GATEWAY,
    .pref = 300,
};
static LocalMsg MSG_PROBE_REQ_GW4 = {
    .type = LocalMsgType::PROBE_REQ,
    .addr = PEER_GW4.addr,
    .nodeType = NodeType::CLIENT,
};
static LocalMsg MSG_PROBE_RES_GW4 = {
    .type = LocalMsgType::PROBE_RES,
    .addr = PEER_GW4.addr,
    .nodeType = NodeType::GATEWAY,
    .pref = 250,
};
static LocalMsg MSG_PROBE_REQ_RELAY1 = {
    .type = LocalMsgType::PROBE_REQ,
    .addr = PEER_RELAY1.addr,
    .nodeType = NodeType::CLIENT,
};
static LocalMsg MSG_PROBE_RES_RELAY1 = {
    .type = LocalMsgType::PROBE_RES,
    .addr = PEER_RELAY1.addr,
    .nodeType = NodeType::RELAY,
    .pref = 50,
};
static LocalMsg MSG_PROBE_RES_RELAY1_WITH_RSSI = {
    .type = LocalMsgType::PROBE_RES,
    .addr = PEER_RELAY1.addr,
    .nodeType = NodeType::RELAY,
    .rssi = -74,
    .pref = 50,
};
static PubData PUB_DATA_RELAY1_RSSI = {
    .topic = "_report/rssi/" + PEER_RELAY1.addr.toString(),
    .payload = "-74",
};

// OK
static LocalMsg MSG_OK_GW2 = {
    .type = LocalMsgType::OK,
    .addr = PEER_GW2.addr,
    .nodeType = NodeType::GATEWAY,
};
static LocalMsg MSG_OK_GW2_FROM_CLIENT = {
    .type = LocalMsgType::OK,
    .addr = PEER_GW2.addr,
    .nodeType = NodeType::CLIENT,
};

// FAIL
static LocalMsg MSG_FAIL_GW2 = {
    .type = LocalMsgType::FAIL,
    .addr = PEER_GW2.addr,
    .nodeType = NodeType::GATEWAY,
    .failReason = LocalMsgFailReason::PROCESSING_FAILED,
};

// Pub/sub/unsub
static LocalMsg MSG_PUB_1_GW2 = {
    .type = LocalMsgType::PUB_SUB_UNSUB,
    .addr = PEER_GW2.addr,
    .pubs = {PUB_DATA1},
    .nodeType = NodeType::CLIENT,
};
static LocalMsg MSG_PUB_1_GW3 = {
    .type = LocalMsgType::PUB_SUB_UNSUB,
    .addr = PEER_GW3.addr,
    .pubs = {PUB_DATA1},
    .nodeType = NodeType::CLIENT,
};
static LocalMsg MSG_SUB_12_GW2 = {
    .type = LocalMsgType::PUB_SUB_UNSUB,
    .addr = PEER_GW2.addr,
    .subs = {TOPIC1, TOPIC2},
    .nodeType = NodeType::CLIENT,
};
static LocalMsg MSG_SUB_21_GW2 = {
    .type = LocalMsgType::PUB_SUB_UNSUB,
    .addr = PEER_GW2.addr,
    .subs = {TOPIC2, TOPIC1},
    .nodeType = NodeType::CLIENT,
};
static LocalMsg MSG_UNSUB_12_GW2 = {
    .type = LocalMsgType::PUB_SUB_UNSUB,
    .addr = PEER_GW2.addr,
    .unsubs = {TOPIC1, TOPIC2},
    .nodeType = NodeType::CLIENT,
};
static LocalMsg MSG_UNSUB_21_GW2 = {
    .type = LocalMsgType::PUB_SUB_UNSUB,
    .addr = PEER_GW2.addr,
    .unsubs = {TOPIC2, TOPIC1},
    .nodeType = NodeType::CLIENT,
};
static LocalMsg MSG_PUB_12_SUB_12_UNSUB_12_GW2 = {
    .type = LocalMsgType::PUB_SUB_UNSUB,
    .addr = PEER_GW2.addr,
    .pubs = {PUB_DATA1, PUB_DATA2},
    .subs = {SUB_REQ1.topic, SUB_REQ2.topic},
    .unsubs = {SUB_REQ1.topic, SUB_REQ2.topic},
    .nodeType = NodeType::CLIENT,
};

// Subscription data
static LocalMsg MSG_SUB_DATA_12_GW2 = {
    .type = LocalMsgType::SUB_DATA,
    .addr = PEER_GW2.addr,
    .subsData = {SUB_DATA1, SUB_DATA2},
    .nodeType = NodeType::GATEWAY,
};

TEST_CASE("Initialization without retained data", "[Client]")
{
    DEFAULT_LL(ll);

    SECTION("No gateways")
    {
        auto startTS = std::chrono::system_clock::now();

        CHECK_THROWS(Client(CONF, &ll));
        CHECK(ll.sentLog == SentLog{MSG_PROBE_REQ, MSG_PROBE_REQ, MSG_PROBE_REQ});
        CHECK(ll.respSuccLog == RespSuccLog{});
        CHECK(ll.channelsLog == ChannelsLog{});

        auto expectedDuration = CONF.gwDscv.dscvMinDelay * (1 + 2 + 4) +
                                CONF.nodeConf.localDelivery.respTimeout * 3;
        auto duration = std::chrono::system_clock::now() - startTS;
        CHECK(duration > expectedDuration - 5ms);
        CHECK(duration < expectedDuration + 5ms);
    }

    SECTION("Success")
    {
        ll.respTsDiff = 100ms;
        ll.responses.push(MSG_PROBE_RES_GW1);
        ll.responses.push(MSG_PROBE_RES_GW1);
        Client cl(CONF, &ll);
        cl.syncTime(); // Just to trigger message dispatch
        std::this_thread::sleep_for(10ms);
        CHECK(ll.sentLog == SentLog{MSG_PROBE_REQ, MSG_PROBE_REQ_GW1});
        CHECK(ll.respSuccLog == RespSuccLog{true, true});
        CHECK(ll.channelsLog == ChannelsLog{});
    }

    SECTION("Success with relay")
    {
        ll.responses.push(MSG_PROBE_RES_RELAY1);
        ll.responses.push(MSG_PROBE_RES_RELAY1);
        Client cl(CONF, &ll);
        cl.syncTime(); // Just to trigger message dispatch
        std::this_thread::sleep_for(10ms);
        CHECK(ll.sentLog == SentLog{MSG_PROBE_REQ, MSG_PROBE_REQ_RELAY1});
        CHECK(ll.respSuccLog == RespSuccLog{true, true});
        CHECK(ll.channelsLog == ChannelsLog{});
    }
}

TEST_CASE("Initialization with retained data", "[Client]")
{
    ClientRetainedData retained = {
        .gw = PEER_GW1.retain(),
        .msgsFailCnt = 0,
        .timeSyncNoRespCnt = 0,
    };
    DEFAULT_LL(ll);

    SECTION("Sync successful")
    {
        ll.respTsDiff = 100ms;
        ll.responses.push(MSG_PROBE_RES_GW1);
        ll.responses.push(MSG_PROBE_RES_GW1);
        Client cl(CONF, &ll, retained);
        cl.syncTime(); // Just to trigger message dispatch
        std::this_thread::sleep_for(10ms);
        CHECK(ll.sentLog == SentLog{MSG_PROBE_REQ_GW1, MSG_PROBE_REQ_GW1});
        CHECK(ll.respSuccLog == RespSuccLog{true, true});
        CHECK(ll.channelsLog == ChannelsLog{PEER_GW1.channel});
    }

    SECTION("Sync failed, discovery successful")
    {
        // Retained data contain channel even though local layer doesn't
        // support channels.
        // In this section, `ILocalLayer::setChannel()` doesn't fail, there's
        // just no valid response.
        ll.respTsDiff = 100ms;
        ll.responses.push({});
        ll.responses.push(MSG_PROBE_RES_GW1);
        ll.responses.push(MSG_PROBE_RES_GW1);

        Client cl(CONF, &ll, retained);
        cl.syncTime(); // Just to trigger message dispatch
        std::this_thread::sleep_for(10ms);
        CHECK(ll.sentLog == SentLog{MSG_PROBE_REQ_GW1, MSG_PROBE_REQ,
                                    MSG_PROBE_REQ_GW1});
        CHECK(ll.respSuccLog == RespSuccLog{false, true, true});
        CHECK(ll.channelsLog == ChannelsLog{PEER_GW1.channel});
    }

    SECTION("Sync failed due to set channel failure, discovery successful")
    {
        // Retained data contain channel even though local layer doesn't
        // support channels.
        // In this section, `ILocalLayer::setChannel()` fails.
        retained.gw.channel = 10;
        ll.respTsDiff = 100ms;
        ll.responses.push(MSG_PROBE_RES_GW1);
        ll.responses.push(MSG_PROBE_RES_GW1);
        ll.setChannelRet = ErrCode::GENERIC_FAILURE;

        Client cl(CONF, &ll, retained);
        cl.syncTime(); // Just to trigger message dispatch
        std::this_thread::sleep_for(10ms);
        CHECK(ll.sentLog == SentLog{MSG_PROBE_REQ, MSG_PROBE_REQ_GW1});
        CHECK(ll.respSuccLog == RespSuccLog{true, true});
        CHECK(ll.channelsLog == ChannelsLog{retained.gw.channel});
    }

    SECTION("Sync failed, discovery failed")
    {
        auto startTS = std::chrono::system_clock::now();

        CHECK_THROWS(Client(CONF, &ll, retained));
        CHECK(ll.sentLog == SentLog{MSG_PROBE_REQ_GW1, MSG_PROBE_REQ,
                                    MSG_PROBE_REQ, MSG_PROBE_REQ});
        CHECK(ll.respSuccLog == RespSuccLog{});
        CHECK(ll.channelsLog == ChannelsLog{PEER_GW1.channel});

        auto expectedDuration = CONF.gwDscv.dscvMinDelay * (1 + 2 + 4) +
                                CONF.nodeConf.localDelivery.respTimeout * 4;
        auto duration = std::chrono::system_clock::now() - startTS;
        CHECK(duration > expectedDuration - 5ms);
        CHECK(duration < expectedDuration + 5ms);
    }
}

TEST_CASE("Initialization with multiple gateways on multiple channels",
          "[Client]")
{
    DEFAULT_LL(ll);
    ll.channels = {74, 39, 88};
    ll.responses.push(MSG_PROBE_RES_GW1);
    ll.responses.push(MSG_PROBE_RES_GW3);
    ll.responses.push(MSG_PROBE_RES_GW2);
    ll.responses.push(MSG_PROBE_RES_GW3);

    Client cl(CONF, &ll);
    cl.syncTime(); // Just to trigger message dispatch
    std::this_thread::sleep_for(10ms);

    CHECK(ll.sentLog == SentLog{MSG_PROBE_REQ, MSG_PROBE_REQ, MSG_PROBE_REQ,
                                MSG_PROBE_REQ_GW3});
    CHECK(ll.respSuccLog == RespSuccLog{true, true, true, true});
    CHECK(ll.channelsLog == ChannelsLog{74, 39, 88, 39});
}

TEST_CASE("Initialization without local layer", "[Client]")
{
    REQUIRE_THROWS(Client(CONF, nullptr));
}

TEST_CASE("Periodic time sync", "[Client]")
{
    auto modifConf = CONF;
    modifConf.timeSync.reprobeGatewayInterval = 100ms;
    modifConf.gwDscv.trigTimeSyncNoRespCnt = 2;
    modifConf.gwDscv.dscvMinDelay = 500ms;

    DEFAULT_LL(ll);

    SECTION("Basic")
    {
        for (int i = 0; i < 4; i++) {
            ll.responses.push(MSG_PROBE_RES_GW2);
        }

        // Make client live for couple of cycles
        {
            Client cl(modifConf, &ll);
            std::this_thread::sleep_for(350ms);
        }

        CHECK(ll.sentLog == SentLog{MSG_PROBE_REQ, MSG_PROBE_REQ_GW2,
                                    MSG_PROBE_REQ_GW2, MSG_PROBE_REQ_GW2});
        CHECK(ll.respSuccLog == RespSuccLog{true, true, true, true});
    }

    SECTION("Failing (triggering gateway discovery)")
    {
        ll.responses.push(MSG_PROBE_RES_GW2);

        // Make client live for couple of cycles
        {
            Client cl(modifConf, &ll);
            std::this_thread::sleep_for(250ms);
        }

        CHECK(ll.sentLog == SentLog{MSG_PROBE_REQ, MSG_PROBE_REQ_GW2,
                                    MSG_PROBE_REQ_GW2, MSG_PROBE_REQ});
        CHECK(ll.respSuccLog == RespSuccLog{true});
    }
}

TEST_CASE("Publish, subscribe, unsubscribe without data", "[Client]")
{
    DEFAULT_LL(ll);
    ll.responses.push(MSG_PROBE_RES_GW2);

    Client cl(CONF, &ll);

    // Shouldn't send any message
    CHECK(cl.pubSubUnsubBulk({}, {}, {}) == ErrCode::SUCCESS);

    std::this_thread::sleep_for(10ms);
    CHECK(ll.sentLog == SentLog{MSG_PROBE_REQ});
    CHECK(ll.respSuccLog == RespSuccLog{true});
}

TEST_CASE("Publish, subscribe, unsubscribe", "[Client]")
{
    DEFAULT_LL(ll);
    ll.responses.push(MSG_PROBE_RES_GW2);

    SECTION("Success")
    {
        ll.responses.push(MSG_OK_GW2);
        Client cl(CONF, &ll);
        CHECK(cl.pubSubUnsubBulk({PUB_DATA1, PUB_DATA2}, {SUB_REQ1, SUB_REQ2},
                                 {SUB_REQ1.topic, SUB_REQ2.topic}) ==
              ErrCode::SUCCESS);
        std::this_thread::sleep_for(10ms);
        CHECK(ll.respSuccLog == RespSuccLog{true, true});
    }

    SECTION("Timeout")
    {
        Client cl(CONF, &ll);
        CHECK(cl.pubSubUnsubBulk({PUB_DATA1, PUB_DATA2}, {SUB_REQ1, SUB_REQ2},
                                 {SUB_REQ1.topic, SUB_REQ2.topic}) ==
              ErrCode::TIMEOUT);
        std::this_thread::sleep_for(10ms);
        CHECK(ll.respSuccLog == RespSuccLog{true});
    }

    SECTION("Explicit FAIL")
    {
        ll.responses.push(MSG_FAIL_GW2);
        Client cl(CONF, &ll);
        CHECK(cl.pubSubUnsubBulk({PUB_DATA1, PUB_DATA2}, {SUB_REQ1, SUB_REQ2},
                                 {SUB_REQ1.topic, SUB_REQ2.topic}) ==
              ErrCode::MSG_PROCESSING_FAILED);
        std::this_thread::sleep_for(10ms);
        CHECK(ll.respSuccLog == RespSuccLog{true, true});
    }

    // Requests are the same for all sections
    CHECK(ll.sentLog == SentLog{MSG_PROBE_REQ,
                                MSG_PUB_12_SUB_12_UNSUB_12_GW2});
}

TEST_CASE("Periodic subscriptions renewal with empty database", "[Client]")
{
    auto modifConf = CONF;
    modifConf.subDB.subLifetime = 100ms;

    DEFAULT_LL(ll);
    ll.responses.push(MSG_PROBE_RES_GW2);

    {
        Client cl(modifConf, &ll);
        std::this_thread::sleep_for(250ms);
    }
    CHECK(ll.sentLog == SentLog{MSG_PROBE_REQ});
    CHECK(ll.respSuccLog == RespSuccLog{true});
}

TEST_CASE("Periodic subscriptions renewal with populated database", "[Client]")
{
    auto modifConf = CONF;
    modifConf.subDB.subLifetime = 100ms;

    DEFAULT_LL(ll);
    ll.responses.push(MSG_PROBE_RES_GW2);
    ll.responses.push(MSG_OK_GW2);

    RespSuccLog correctRespSuccLog = {true, true};

    SECTION("Successful renewal")
    {
        for (int i = 0; i < 3; i++) {
            ll.responses.push(MSG_OK_GW2);
            correctRespSuccLog.push_back(true);
        }
    }

    SECTION("Failing renewal")
    {
        // No responses
    }

    {
        Client cl(modifConf, &ll);
        cl.subscribeBulk({SUB_REQ1, SUB_REQ2});

        std::this_thread::sleep_for(350ms);
    }

    CHECK((ll.sentLog == SentLog{MSG_PROBE_REQ, MSG_SUB_12_GW2, MSG_SUB_12_GW2,
                                 MSG_SUB_12_GW2, MSG_SUB_12_GW2} ||
           ll.sentLog == SentLog{MSG_PROBE_REQ, MSG_SUB_12_GW2, MSG_SUB_21_GW2,
                                 MSG_SUB_21_GW2, MSG_SUB_21_GW2}));
    CHECK(ll.respSuccLog == correctRespSuccLog);
}

TEST_CASE("Unsubscribe all with empty database", "[Client]")
{
    DEFAULT_LL(ll);
    ll.responses.push(MSG_PROBE_RES_GW2);

    // No UNSUB message sent
    Client cl(CONF, &ll);
    CHECK(cl.unsubscribeAll() == ErrCode::SUCCESS);
    CHECK(ll.sentLog == SentLog{MSG_PROBE_REQ});
    CHECK(ll.respSuccLog == RespSuccLog{true});
}

TEST_CASE("Unsubscribe all with populated database", "[Client]")
{
    DEFAULT_LL(ll);
    ll.responses.push(MSG_PROBE_RES_GW2);
    ll.responses.push(MSG_OK_GW2);

    SECTION("Success")
    {
        ll.responses.push(MSG_OK_GW2);
        Client cl(CONF, &ll);
        cl.subscribeBulk({SUB_REQ1, SUB_REQ2});
        CHECK(cl.unsubscribeAll() == ErrCode::SUCCESS);
        std::this_thread::sleep_for(10ms);
        CHECK(ll.respSuccLog == RespSuccLog{true, true, true});
    }

    SECTION("Timeout")
    {
        Client cl(CONF, &ll);
        cl.subscribeBulk({SUB_REQ1, SUB_REQ2});
        CHECK(cl.unsubscribeAll() == ErrCode::TIMEOUT);
        std::this_thread::sleep_for(10ms);
        CHECK(ll.respSuccLog == RespSuccLog{true, true});
    }

    SECTION("Explicit FAIL")
    {
        ll.responses.push(MSG_FAIL_GW2);
        Client cl(CONF, &ll);
        cl.subscribeBulk({SUB_REQ1, SUB_REQ2});
        CHECK(cl.unsubscribeAll() == ErrCode::MSG_PROCESSING_FAILED);
        std::this_thread::sleep_for(10ms);
        CHECK(ll.respSuccLog == RespSuccLog{true, true, true});
    }

    // Requests are the same for all sections
    CHECK((ll.sentLog == SentLog{MSG_PROBE_REQ, MSG_SUB_12_GW2,
                                 MSG_UNSUB_12_GW2} ||
           ll.sentLog == SentLog{MSG_PROBE_REQ, MSG_SUB_12_GW2,
                                 MSG_UNSUB_21_GW2}));
}

TEST_CASE("Resubscribe all with empty database", "[Client]")
{
    DEFAULT_LL(ll);
    ll.responses.push(MSG_PROBE_RES_GW2);

    // No SUB message sent
    Client cl(CONF, &ll);
    CHECK(cl.resubscribeAll() == ErrCode::SUCCESS);
    CHECK(ll.sentLog == SentLog{MSG_PROBE_REQ});
    CHECK(ll.respSuccLog == RespSuccLog{true});
}

TEST_CASE("Resubscribe all with populated database", "[Client]")
{
    DEFAULT_LL(ll);
    ll.responses.push(MSG_PROBE_RES_GW2);
    ll.responses.push(MSG_OK_GW2);

    SECTION("Success")
    {
        ll.responses.push(MSG_OK_GW2);
        Client cl(CONF, &ll);
        cl.subscribeBulk({SUB_REQ1, SUB_REQ2});
        CHECK(cl.resubscribeAll() == ErrCode::SUCCESS);
        std::this_thread::sleep_for(10ms);
        CHECK(ll.respSuccLog == RespSuccLog{true, true, true});
    }

    SECTION("Timeout")
    {
        Client cl(CONF, &ll);
        cl.subscribeBulk({SUB_REQ1, SUB_REQ2});
        CHECK(cl.resubscribeAll() == ErrCode::TIMEOUT);
        std::this_thread::sleep_for(10ms);
        CHECK(ll.respSuccLog == RespSuccLog{true, true});
    }

    SECTION("Explicit FAIL")
    {
        ll.responses.push(MSG_FAIL_GW2);
        Client cl(CONF, &ll);
        cl.subscribeBulk({SUB_REQ1, SUB_REQ2});
        CHECK(cl.resubscribeAll() == ErrCode::MSG_PROCESSING_FAILED);
        std::this_thread::sleep_for(10ms);
        CHECK(ll.respSuccLog == RespSuccLog{true, true, true});
    }

    // Requests are the same for all sections
    CHECK((ll.sentLog == SentLog{MSG_PROBE_REQ, MSG_SUB_12_GW2,
                                 MSG_SUB_12_GW2} ||
           ll.sentLog == SentLog{MSG_PROBE_REQ, MSG_SUB_12_GW2,
                                 MSG_SUB_21_GW2}));
}

TEST_CASE("Receive subscription data", "[Client]")
{
    DEFAULT_LL(ll);
    ll.responses.push(MSG_PROBE_RES_GW2);
    ll.responses.push(MSG_OK_GW2);

    int cnt = 0;
    SubData recvSubData;

    Client cl(CONF, &ll);
    cl.subscribe("aaa/bbb/#", [&cnt, &recvSubData](const SubData &data) {
        cnt++;
        recvSubData = data;
    });

    LocalMsg msg = {
        .type = LocalMsgType::SUB_DATA,
        .addr = PEER_GW2.addr,
        .nodeType = NodeType::GATEWAY,
    };

    SECTION("No topic match")
    {
        msg.subsData.push_back({"i/am/not/matching/anything", "payload"});
        prepLocalMsg(msg, ll.respTsDiff, ll.respTimeUnit);
        CHECK(ll.recv(msg) == ErrCode::SUCCESS);
        CHECK(cnt == 0);
    }

    SECTION("Single topic match")
    {
        msg.subsData.push_back({"aaa/bbb/123", "payload"});
        prepLocalMsg(msg, ll.respTsDiff, ll.respTimeUnit);
        CHECK(ll.recv(msg) == ErrCode::SUCCESS);
        CHECK(cnt == 1);
        CHECK(recvSubData.topic == "aaa/bbb/123");
        CHECK(recvSubData.payload == "payload");
    }

    SECTION("Multiple topic matches")
    {
        msg.subsData.push_back({"aaa/bbb/123", "payload1"});
        msg.subsData.push_back({"aaa/bbb/1/2", "payload2"});
        prepLocalMsg(msg, ll.respTsDiff, ll.respTimeUnit);
        CHECK(ll.recv(msg) == ErrCode::SUCCESS);
        CHECK(cnt == 2);
        CHECK(recvSubData.topic == "aaa/bbb/1/2");
        CHECK(recvSubData.payload == "payload2");
    }

    // Response should be successful in any case
    std::this_thread::sleep_for(10ms);
    CHECK(ll.respSuccLog == RespSuccLog{true, true});
}

TEST_CASE("Gateway discovery on local layer without channels", "[Client]")
{
    DEFAULT_LL(ll);
    ll.responses.push(MSG_PROBE_RES_GW3);

    Client cl(CONF, &ll);

    SECTION("Success")
    {
        ll.responses.push(MSG_PROBE_RES_GW3);
        ll.responses.push(MSG_PROBE_RES_GW3);

        CHECK(cl.discoverGateway() == ErrCode::SUCCESS);

        // Just to trigger message dispatch
        CHECK(cl.syncTime() == ErrCode::SUCCESS);
        std::this_thread::sleep_for(10ms);

        CHECK(ll.sentLog == SentLog{MSG_PROBE_REQ, MSG_PROBE_REQ,
                                    MSG_PROBE_REQ_GW3});
        CHECK(ll.respSuccLog == RespSuccLog{true, true, true});
        CHECK(ll.channelsLog == ChannelsLog{});
    }

    SECTION("No gateway")
    {
        auto startTS = std::chrono::system_clock::now();

        CHECK(cl.discoverGateway(4) == ErrCode::TOO_MANY_FAILED_ATTEMPTS);

        // Check timing
        auto expectedDuration = CONF.gwDscv.dscvMinDelay * (1 + 2 + 4 + 8) +
                                CONF.nodeConf.localDelivery.respTimeout * 4;
        auto duration = std::chrono::system_clock::now() - startTS;
        CHECK(duration > expectedDuration - 5ms);
        CHECK(duration < expectedDuration + 5ms);

        // Discovery failed, other messages should fail too
        CHECK(cl.syncTime() == ErrCode::NO_GATEWAY);
        std::this_thread::sleep_for(10ms);

        CHECK(ll.sentLog == SentLog{MSG_PROBE_REQ, MSG_PROBE_REQ,
                                    MSG_PROBE_REQ, MSG_PROBE_REQ,
                                    MSG_PROBE_REQ});
        CHECK(ll.respSuccLog == RespSuccLog{true});
        CHECK(ll.channelsLog == ChannelsLog{});
    }
}

TEST_CASE("Gateway discovery on local layer with channels", "[Client]")
{
    DEFAULT_LL(ll);
    ll.channels = {0, 1};
    ll.responses.push(MSG_PROBE_RES_GW3);
    ll.responses.push(MSG_PROBE_RES_RELAY1);

    Client cl(CONF, &ll);

    SECTION("Success")
    {
        ll.responses.push(MSG_PROBE_RES_RELAY1);
        ll.responses.push(MSG_PROBE_RES_GW3);
        ll.responses.push(MSG_PROBE_RES_GW3);

        CHECK(cl.discoverGateway() == ErrCode::SUCCESS);

        // Just to trigger message dispatch
        CHECK(cl.syncTime() == ErrCode::SUCCESS);
        std::this_thread::sleep_for(10ms);

        CHECK(ll.sentLog == SentLog{MSG_PROBE_REQ, MSG_PROBE_REQ,
                                    MSG_PROBE_REQ, MSG_PROBE_REQ,
                                    MSG_PROBE_REQ_GW3});
        CHECK(ll.respSuccLog == RespSuccLog(5, true));
        CHECK(ll.channelsLog ==
              ChannelsLog{
                  0, 1, // Discovery in constructor
                  0,    // Set channel of best GW in constructor
                  0, 1, // Discovery scan
                  1     // Set channel of best GW after scan
              });
    }

    SECTION("No gateway")
    {
        auto startTS = std::chrono::system_clock::now();

        CHECK(cl.discoverGateway(3) == ErrCode::TOO_MANY_FAILED_ATTEMPTS);

        // Check timing
        auto expectedDuration = CONF.gwDscv.dscvMinDelay * (1 + 2 + 4) +
                                CONF.nodeConf.localDelivery.respTimeout * 6;
        auto duration = std::chrono::system_clock::now() - startTS;
        CHECK(duration > expectedDuration - 5ms);
        CHECK(duration < expectedDuration + 5ms);

        // Discovery failed, other messages should fail too
        CHECK(cl.syncTime() == ErrCode::NO_GATEWAY);
        std::this_thread::sleep_for(10ms);

        CHECK(ll.sentLog == SentLog{8, MSG_PROBE_REQ});
        CHECK(ll.respSuccLog == RespSuccLog{true, true});
        CHECK(ll.channelsLog ==
              ChannelsLog{
                  0, 1, // Discovery in constructor
                  0,    // Set channel of best GW in constructor
                  0, 1, // Attempt 1 of discovery scan
                  0, 1, // Attempt 2 of discovery scan
                  0, 1, // Attempt 3 of discovery scan
              });
    }

    SECTION("ILocalLayer::setChannel() fails")
    {
        ll.setChannelRet = ErrCode::GENERIC_FAILURE;
        CHECK(cl.discoverGateway() == ErrCode::TOO_MANY_FAILED_ATTEMPTS);

        std::this_thread::sleep_for(10ms);
        CHECK(ll.sentLog == SentLog{MSG_PROBE_REQ, MSG_PROBE_REQ});
        CHECK(ll.respSuccLog == RespSuccLog{true, true});
        CHECK(ll.channelsLog ==
              ChannelsLog{
                  0, 1, // Discovery in constructor
                  0,    // Set channel of best GW in constructor
                  0, 1, // Discovery scan
                  // No best GW found (failed) -> no channel change
              });
    }
}

TEST_CASE("Parallel gateway discoveries", "[Client]")
{
    // Tests mutual exclusion in regard to channel switching

    struct ChannelAwareProbeLocalLayer : public DummyLocalLayer
    {
        ErrCode send(const LocalMsg &msg)
        {
            const std::scoped_lock lock{_mutex};
            sentLog.push_back(msg);

            if (channels.empty()) {
                // Don't respond on default channel
                return sendRet;
            }

            auto curCh = channelsLog.back();

            LocalMsg respMsg;
            switch (curCh) {
            case 10:
                respMsg = MSG_PROBE_RES_GW1;
                respMsg.pref = 100;
                break;
            case 20:
                respMsg = MSG_PROBE_RES_GW2;
                respMsg.pref = 150;
                break;
            case 30:
                respMsg = MSG_PROBE_RES_GW3;
                respMsg.pref = 300;
                break;
            case 40:
                respMsg = MSG_PROBE_RES_GW4;
                respMsg.pref = 250;
                break;
            default:
                return sendRet;
            }
            respMsg.reqId = msg.id;

            std::thread respThread(&DummyLocalLayer::simulateResponse,
                                   this, respMsg);
            respThread.detach();

            return sendRet;
        }
    };

    ClientConfig modifConf = CONF;
    modifConf.nodeConf.localDelivery.respTimeout = 50ms;

    ChannelAwareProbeLocalLayer ll;
    ll.respTsDiff = 0ms;
    ll.respTimeUnit = 10ms;
    ll.respDelay = 40ms;
    ll.channels = {10, 20, 30, 40};
    ll.responses.push(MSG_PROBE_RES_GW3);

    Client cl(modifConf, &ll);
    auto dscvWithCheck = [&cl]() {
        CHECK(cl.discoverGateway() == ErrCode::SUCCESS);
    };

    // Spawn three parallel discovery scans
    std::thread t1(dscvWithCheck);
    std::thread t2(dscvWithCheck);
    std::thread t3(dscvWithCheck);
    t1.join();
    t2.join();
    t3.join();

    // Just to trigger message dispatch
    CHECK(cl.syncTime() == ErrCode::SUCCESS);
    std::this_thread::sleep_for(10ms);

    SentLog correctSentLog(4 /*channels*/ * 4 /*scans*/, MSG_PROBE_REQ);
    correctSentLog.push_back(MSG_PROBE_REQ_GW3);

    CHECK(ll.sentLog == correctSentLog);
    CHECK(ll.respSuccLog ==
          RespSuccLog(4 /*channels*/ * 4 /*scans*/ + 1 /*time sync*/, true));
    CHECK(ll.channelsLog.size() ==
          (4 /*channels*/ + 1 /*best GW change*/) * 4 /*scans*/);
    CHECK(ll.channelsLog.back() == 30); // Highest preference gateway
}

TEST_CASE("Retain", "[Client]")
{
    DEFAULT_LL(ll);
    ll.responses.push(MSG_PROBE_RES_GW2);

    SECTION("Local layer without channels")
    {
        Client cl(CONF, &ll);

        // Trigger time sync failure to increment counters
        CHECK(cl.syncTime() == ErrCode::TIMEOUT);

        auto retained = cl.retain();

        for (size_t i = 0; i < PEER_GW2.addr.addr.size(); i++) {
            CHECK(retained.gw.addr[i] == PEER_GW2.addr.addr[i]);
        }
        CHECK(retained.gw.addrLen == PEER_GW2.addr.addr.size());
        CHECK(retained.gw.channel == 0);
        CHECK(retained.msgsFailCnt == 1);
        CHECK(retained.timeSyncNoRespCnt == 1);
    }

    SECTION("Local layer with channels")
    {
        ll.channels = {2};

        Client cl(CONF, &ll);

        // Trigger delivery failures to increment counter
        CHECK(cl.publish(TOPIC1, PAYLOAD1) == ErrCode::TIMEOUT);
        CHECK(cl.subscribe(TOPIC1, nullptr) == ErrCode::TIMEOUT);

        auto retained = cl.retain();

        for (size_t i = 0; i < PEER_GW2.addr.addr.size(); i++) {
            CHECK(retained.gw.addr[i] == PEER_GW2.addr.addr[i]);
        }
        CHECK(retained.gw.addrLen == PEER_GW2.addr.addr.size());
        CHECK(retained.gw.channel == 2);
        CHECK(retained.msgsFailCnt == 2);
        CHECK(retained.timeSyncNoRespCnt == 0);
    }
}

TEST_CASE("Gateway rediscovery after many failures", "[Client]")
{
    auto modifConf = CONF;
    modifConf.gwDscv.trigMsgsFailCnt = 3;
    modifConf.gwDscv.dscvMinDelay = 500ms;

    DEFAULT_LL(ll);
    ll.responses.push(MSG_PROBE_RES_GW2);
    ll.responses.push({});
    ll.responses.push({});
    ll.responses.push({});
    ll.responses.push(MSG_PROBE_RES_GW2);
    ll.responses.push(MSG_OK_GW2);

    {
        Client cl(modifConf, &ll);

        // Do 3 failed publications
        CHECK(cl.publish(TOPIC1, PAYLOAD1) == ErrCode::TIMEOUT);
        CHECK(cl.publish(TOPIC1, PAYLOAD1) == ErrCode::TIMEOUT);
        CHECK(cl.publish(TOPIC1, PAYLOAD1) == ErrCode::TIMEOUT);

        // Make client live for a bit, so discovery thread is spawned
        std::this_thread::sleep_for(100ms);

        // Publish successfully
        CHECK(cl.publish(TOPIC1, PAYLOAD1) == ErrCode::SUCCESS);
    }

    std::this_thread::sleep_for(10ms);
    CHECK(ll.sentLog == SentLog{
                            MSG_PROBE_REQ, // Initial gateway discovery
                            MSG_PUB_1_GW2, // Failed publication 1
                            MSG_PUB_1_GW2, // Failed publication 2
                            MSG_PUB_1_GW2, // Failed publication 3
                            MSG_PROBE_REQ, // Automatic gateway discovery
                            MSG_PUB_1_GW2, // Successful publication
                        });
    CHECK(ll.respSuccLog == RespSuccLog{true, false, false, false, true,
                                        true});
}

TEST_CASE("Replay protection for responses", "[Client]")
{
    auto modifConf = CONF;
    modifConf.nodeConf.localDelivery.respTimeout = 100ms;

    DEFAULT_LL(ll);
    ll.responses.push(MSG_PROBE_RES_GW2);

    SECTION("Duplicate ID")
    {
        Client cl(modifConf, &ll);

        std::thread t([&ll]() {
            // Sleep until publish sends out a message
            std::this_thread::sleep_for(10ms);

            CHECK(ll.sentLog.size() == 2);
            if (ll.sentLog.size() != 2) {
                return;
            }

            auto reqMsg = ll.sentLog.back();

            // Intentionally wrong response type
            auto msg = MSG_PROBE_RES_GW2;
            msg.reqId = reqMsg.id;
            prepLocalMsg(msg, ll.respTsDiff, ll.respTimeUnit);

            CHECK(ll.recv(msg) == ErrCode::INVALID_ARG);

            // Retransmissions should be detected and discarded
            CHECK(ll.recv(msg) == ErrCode::MSG_DUP_ID);
            CHECK(ll.recv(msg) == ErrCode::MSG_DUP_ID);
        });

        CHECK(cl.publish(TOPIC1, PAYLOAD1) == ErrCode::TIMEOUT);
        t.join();

        std::this_thread::sleep_for(10ms);
        CHECK(ll.sentLog == SentLog{MSG_PROBE_REQ, MSG_PUB_1_GW2});
        CHECK(ll.respSuccLog == RespSuccLog{true});
    }

    SECTION("Still valid timestamp")
    {
        // Make time difference too big to be considered valid
        ll.respTsDiff = -ll.respTimeUnit *
                        (modifConf.nodeConf.msgIdCache.maxAge - 1);
        ll.responses.push(MSG_OK_GW2);

        Client cl(modifConf, &ll);
        CHECK(cl.publish(TOPIC1, PAYLOAD1) == ErrCode::SUCCESS);

        std::this_thread::sleep_for(10ms);
        CHECK(ll.sentLog == SentLog{MSG_PROBE_REQ, MSG_PUB_1_GW2});
        CHECK(ll.respSuccLog == RespSuccLog{true, true});
    }

    SECTION("Invalid timestamp")
    {
        // Make time difference too big to be considered valid
        ll.respTsDiff = -ll.respTimeUnit *
                        (modifConf.nodeConf.msgIdCache.maxAge + 1);
        ll.responses.push(MSG_OK_GW2);

        Client cl(modifConf, &ll);
        CHECK(cl.publish(TOPIC1, PAYLOAD1) == ErrCode::TIMEOUT);

        std::this_thread::sleep_for(10ms);
        CHECK(ll.sentLog == SentLog{MSG_PROBE_REQ, MSG_PUB_1_GW2});
        CHECK(ll.respSuccLog == RespSuccLog{true, false});
    }
}

TEST_CASE("Replay protection for SUB_DATA", "[Client]")
{
    DEFAULT_LL(ll);
    ll.responses.push(MSG_PROBE_RES_GW2);

    Client cl(CONF, &ll);

    auto msg = MSG_SUB_DATA_12_GW2;
    prepLocalMsg(msg, ll.respTsDiff, ll.respTimeUnit);

    CHECK(ll.recv(msg) == ErrCode::SUCCESS);

    // Retransmissions should be detected and discarded
    CHECK(ll.recv(msg) == ErrCode::MSG_DUP_ID);
    CHECK(ll.recv(msg) == ErrCode::MSG_DUP_ID);

    // Wait until entry is removed from message ID cache
    std::this_thread::sleep_for(
        ll.respTimeUnit * (CONF.nodeConf.msgIdCache.maxAge + 1));

    // Timestamp too much in the past should be detected and discarded
    CHECK(ll.recv(msg) == ErrCode::MSG_INVALID_TS);

    CHECK(ll.sentLog == SentLog{MSG_PROBE_REQ, MSG_OK_GW2_FROM_CLIENT});
    CHECK(ll.respSuccLog == RespSuccLog{true});
}

TEST_CASE("Receive invalid message", "[Client]")
{
    DEFAULT_LL(ll);
    ll.responses.push(MSG_PROBE_RES_GW2);

    Client cl(CONF, &ll);
    LocalMsg msg;
    msg.addr = PEER_GW2.addr;
    msg.nodeType = NodeType::GATEWAY;
    prepLocalMsg(msg, ll.respTsDiff, ll.respTimeUnit);

    SECTION("No type")
    {
        msg.type = LocalMsgType::NONE;
        CHECK(ll.recv(msg) == ErrCode::INVALID_ARG);
    }

    SECTION("OK without corresponding request")
    {
        msg.type = LocalMsgType::OK;
        CHECK(ll.recv(msg) == ErrCode::NOT_FOUND);
    }

    SECTION("FAIL without corresponding request")
    {
        msg.type = LocalMsgType::FAIL;
        msg.failReason = LocalMsgFailReason::PROCESSING_FAILED;
        CHECK(ll.recv(msg) == ErrCode::NOT_FOUND);
    }

    SECTION("Probe request")
    {
        msg.type = LocalMsgType::PROBE_REQ;
        CHECK(ll.recv(msg) == ErrCode::INVALID_ARG);
    }

    SECTION("Probe response without corresponding request")
    {
        msg.type = LocalMsgType::PROBE_RES;
        CHECK(ll.recv(msg) == ErrCode::NOT_FOUND);
    }

    SECTION("Probe response without address")
    {
        msg.type = LocalMsgType::PROBE_RES;
        msg.addr = {};
        auto err = ll.recv(msg);
        CHECK((err == ErrCode::NOT_FOUND ||
               err == ErrCode::MSG_UNKNOWN_SENDER));
    }

    SECTION("Subscription data from different node")
    {
        msg.type = LocalMsgType::SUB_DATA;
        msg.addr = PEER_GW3.addr;
        msg.subsData.push_back(SUB_DATA1);
        CHECK(ll.recv(msg) == ErrCode::MSG_UNKNOWN_SENDER);
    }

    SECTION("Subscription data from invalid node type")
    {
        msg.type = LocalMsgType::SUB_DATA;
        msg.addr = PEER_GW3.addr;
        msg.subsData.push_back(SUB_DATA1);
        msg.nodeType = NodeType::CLIENT;
        CHECK(ll.recv(msg) == ErrCode::INVALID_ARG);
    }

    CHECK(ll.sentLog == SentLog{MSG_PROBE_REQ});
    CHECK(ll.respSuccLog == RespSuccLog{true});
}

TEST_CASE("Receive response from different node", "[Client]")
{
    DEFAULT_LL(ll);
    ll.responses.push(MSG_PROBE_RES_GW3);
    ll.responses.push(MSG_PROBE_RES_GW2);
    ll.responses.push(MSG_OK_GW2);

    Client cl(CONF, &ll);

    // Should timeout, because response comes from GW2 instead of GW3
    CHECK(cl.syncTime() == ErrCode::TIMEOUT);
    CHECK(cl.publish(TOPIC1, PAYLOAD1) == ErrCode::TIMEOUT);

    std::this_thread::sleep_for(10ms);
    CHECK(ll.sentLog == SentLog{MSG_PROBE_REQ, MSG_PROBE_REQ_GW3,
                                MSG_PUB_1_GW3});
    CHECK(ll.respSuccLog == RespSuccLog{true, false, false});
}

TEST_CASE("Receive invalid response type", "[Client]")
{
    DEFAULT_LL(ll);
    ll.responses.push(MSG_PROBE_RES_GW2);
    ll.responses.push(MSG_OK_GW2);
    ll.responses.push(MSG_PROBE_RES_GW2);

    Client cl(CONF, &ll);

    // Receives OK response (PROBE_RES expected)
    CHECK(cl.syncTime() == ErrCode::TIMEOUT);

    // Receives PROBE_RES response (OK expected)
    CHECK(cl.publish(TOPIC1, PAYLOAD1) == ErrCode::TIMEOUT);

    std::this_thread::sleep_for(10ms);
    CHECK(ll.sentLog == SentLog{MSG_PROBE_REQ, MSG_PROBE_REQ_GW2,
                                MSG_PUB_1_GW2});
    CHECK(ll.respSuccLog == RespSuccLog{true, false, false});
}

TEST_CASE("Receive response with various FAIL reasons", "[Client]")
{
    DEFAULT_LL(ll);
    ll.responses.push(MSG_PROBE_RES_GW2);

    auto msg = MSG_FAIL_GW2;
    ErrCode correctRetCode;

    SECTION("NONE")
    {
        msg.failReason = LocalMsgFailReason::NONE;
    }

    SECTION("DUP_ID")
    {
        msg.failReason = LocalMsgFailReason::DUP_ID;
    }

    SECTION("INVALID_TS")
    {
        msg.failReason = LocalMsgFailReason::INVALID_TS;
    }

    SECTION("PROCESSING_FAILED")
    {
        msg.failReason = LocalMsgFailReason::PROCESSING_FAILED;
    }

    SECTION("UNKNOWN_SENDER")
    {
        msg.failReason = LocalMsgFailReason::UNKNOWN_SENDER;
    }

    ll.responses.push(msg);
    Client cl(CONF, &ll);
    CHECK(cl.publish(TOPIC1, PAYLOAD1) == ErrCode::MSG_PROCESSING_FAILED);

    std::this_thread::sleep_for(10ms);
    CHECK(ll.sentLog == SentLog{MSG_PROBE_REQ, MSG_PUB_1_GW2});
    CHECK(ll.respSuccLog == RespSuccLog{true, true});
}

TEST_CASE("Two valid responses for single unicast request", "[Client]")
{
    auto modifConf = CONF;
    modifConf.nodeConf.localDelivery.respTimeout = 100ms;

    DEFAULT_LL(ll);
    ll.responses.push(MSG_PROBE_RES_GW2);

    Client cl(modifConf, &ll);

    std::thread t([&ll]() {
        // Sleep until publish sends out a message
        std::this_thread::sleep_for(10ms);

        CHECK(ll.sentLog.size() == 2);
        if (ll.sentLog.size() != 2) {
            return;
        }

        auto reqMsg = ll.sentLog.back();
        auto msg = MSG_OK_GW2;
        msg.reqId = reqMsg.id;

        // First response
        prepLocalMsg(msg, ll.respTsDiff, ll.respTimeUnit);
        CHECK(ll.recv(msg) == ErrCode::SUCCESS);

        // Second response
        prepLocalMsg(msg, ll.respTsDiff, ll.respTimeUnit);
        CHECK(ll.recv(msg) == ErrCode::NOT_FOUND);
    });

    CHECK(cl.publish(TOPIC1, PAYLOAD1) == ErrCode::SUCCESS);
    t.join();

    std::this_thread::sleep_for(10ms);
    CHECK(ll.sentLog == SentLog{MSG_PROBE_REQ, MSG_PUB_1_GW2});
    CHECK(ll.respSuccLog == RespSuccLog{true});
}

TEST_CASE("Destructor resets local layer receive callback", "[Client]")
{
    DEFAULT_LL(ll);
    ll.responses.push(MSG_PROBE_RES_GW2);

    // Construct, check callback and destruct
    {
        Client cl(CONF, &ll);
        CHECK(ll.recvCbSet());
    }

    CHECK_FALSE(ll.recvCbSet());
}

TEST_CASE("ILocalLayer::send() fails", "[Client]")
{
    DEFAULT_LL(ll);
    ll.responses.push(MSG_PROBE_RES_GW2);
    Client cl(CONF, &ll);

    // Check error from local layer is propagated correctly
    ll.sendRet = ErrCode::GENERIC_FAILURE;
    CHECK(cl.publish(TOPIC1, PAYLOAD1) == ErrCode::GENERIC_FAILURE);
}

TEST_CASE("Reporting of RSSI after gateway discovery", "[Client]")
{
    auto modifConf = CONF;
    modifConf.reporting.rssiOnGwDscv = true;

    DEFAULT_LL(ll);
    ll.channels = {0, 1};
    ll.responses.push(MSG_PROBE_RES_GW2);
    ll.responses.push(MSG_PROBE_RES_RELAY1);

    Client cl(modifConf, &ll);

    std::vector<PubData> correctReportPub = {PUB_DATA_GW2_RSSI,
                                             PUB_DATA_RELAY1_RSSI};
    std::vector<PubData> correctReportPubRev = {PUB_DATA_RELAY1_RSSI,
                                                PUB_DATA_GW2_RSSI};

    SECTION("Success without RSSI")
    {
        ll.responses.push(MSG_PROBE_RES_RELAY1);
        ll.responses.push(MSG_PROBE_RES_GW2);

        CHECK(cl.discoverGateway() == ErrCode::SUCCESS);

        std::this_thread::sleep_for(10ms);

        // No report should be made
        REQUIRE(ll.sentLog.size() == 2 + 2);
        CHECK(ll.respSuccLog == RespSuccLog(2 + 2, true));
    }

    SECTION("Success with RSSI, report successful")
    {
        ll.responses.push(MSG_PROBE_RES_RELAY1_WITH_RSSI);
        ll.responses.push(MSG_PROBE_RES_GW2_WITH_RSSI);
        ll.responses.push(MSG_OK_GW2);

        CHECK(cl.discoverGateway() == ErrCode::SUCCESS);

        std::this_thread::sleep_for(10ms);
        REQUIRE(ll.sentLog.size() == 2 + 2 + 1);
        CHECK((ll.sentLog.back().pubs == correctReportPub ||
               ll.sentLog.back().pubs == correctReportPubRev));
        CHECK(ll.respSuccLog == RespSuccLog(2 + 2 + 1, true));
    }

    SECTION("Success with RSSI, report failed")
    {
        ll.responses.push(MSG_PROBE_RES_RELAY1_WITH_RSSI);
        ll.responses.push(MSG_PROBE_RES_GW2_WITH_RSSI);

        CHECK(cl.discoverGateway() == ErrCode::SUCCESS);

        std::this_thread::sleep_for(10ms);
        REQUIRE(ll.sentLog.size() == 2 + 2 + 1);
        CHECK((ll.sentLog.back().pubs == correctReportPub ||
               ll.sentLog.back().pubs == correctReportPubRev));
        CHECK(ll.respSuccLog == RespSuccLog(2 + 2, true));
    }

    SECTION("Failure with RSSI")
    {
        CHECK(cl.discoverGateway() == ErrCode::TOO_MANY_FAILED_ATTEMPTS);

        std::this_thread::sleep_for(10ms);

        // No report should be made
        REQUIRE(ll.sentLog.size() == 2 + 2);
        CHECK(ll.respSuccLog == RespSuccLog(2, true));
    }
}
