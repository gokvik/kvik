/**
 * @file local_msg_id_cache.cpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @copyright Copyright (c) 2024
 */

#include <chrono>
#include <thread>

#include <catch2/catch_test_macros.hpp>

#include "kvik/local_addr.hpp"

// Access modifiers hack for testing
#define private public
#include "kvik/local_msg_id_cache.hpp"
#undef private

using namespace kvik;
using namespace std::chrono_literals;

/**
 * @brief Transparent version of `LocalMsgIdCache` for testing
 */
class TransparentLocalMsgIdCache : public LocalMsgIdCache
{
public:
    using LocalMsgIdCache::LocalMsgIdCache;

    const Cache &getCache() const
    {
        return m_cache;
    }

    uint16_t getTickNum() const
    {
        return m_tickNum;
    }
};

// Cache container
using C = TransparentLocalMsgIdCache::Cache;

uint8_t MAX_AGE = 2;
LocalAddr ADDR1{{0x01}};
LocalAddr ADDR2{{0x02, 0x03}};
LocalAddr ADDR3;

TEST_CASE("Empty ticks", "[LocalMsgIdCache]")
{
    TransparentLocalMsgIdCache ic{10ms, MAX_AGE};
    std::this_thread::sleep_for(25ms);
    CHECK(ic.getTickNum() == 2);
    CHECK(ic.getCache().empty());
}

TEST_CASE("Insert", "[LocalMsgIdCache]")
{
    TransparentLocalMsgIdCache ic{10ms, MAX_AGE};

    SECTION("Simple")
    {
        REQUIRE(ic.insert(ADDR1, 0));
        CHECK(ic.getCache() == C{{ADDR1, {{MAX_AGE + 1, {0}}}}});
    }

    SECTION("Multiple addresses")
    {
        REQUIRE(ic.insert(ADDR1, 0));
        REQUIRE(ic.insert(ADDR2, 1));
        REQUIRE(ic.insert(ADDR3, 2));
        CHECK(ic.getCache() == C{
            {ADDR1, {{MAX_AGE + 1, {0}}}},
            {ADDR2, {{MAX_AGE + 1, {1}}}},
            {ADDR3, {{MAX_AGE + 1, {2}}}},
        });
    }

    SECTION("Multiple addresses with same IDs")
    {
        REQUIRE(ic.insert(ADDR1, 10));
        REQUIRE(ic.insert(ADDR2, 10));
        REQUIRE(ic.insert(ADDR3, 10));
        CHECK(ic.getCache() == C{
            {ADDR1, {{MAX_AGE + 1, {10}}}},
            {ADDR2, {{MAX_AGE + 1, {10}}}},
            {ADDR3, {{MAX_AGE + 1, {10}}}},
        });
    }

    SECTION("Duplicate")
    {
        REQUIRE(ic.insert(ADDR1, 10));
        REQUIRE_FALSE(ic.insert(ADDR1, 10));
        CHECK(ic.getCache() == C{{ADDR1, {{MAX_AGE + 1, {10}}}}});
    }

    SECTION("Same address, different ID")
    {
        REQUIRE(ic.insert(ADDR1, 10));
        REQUIRE(ic.insert(ADDR1, 20));
        CHECK(ic.getCache() == C{{ADDR1, {{MAX_AGE + 1, {10, 20}}}}});
    }

    CHECK(ic.getTickNum() == 0);
}

TEST_CASE("Basic expiration", "[LocalMsgIdCache]")
{
    TransparentLocalMsgIdCache ic{10ms, 5};

    REQUIRE(ic.insert(ADDR1, 0));
    REQUIRE(ic.getCache() == C{{ADDR1, {{6, {0}}}}});

    for (int i = 0; i < 5; i++) {
        std::this_thread::sleep_for(10ms);
        REQUIRE(ic.getCache() == C{{ADDR1, {{6, {0}}}}});
    }

    std::this_thread::sleep_for(10ms);
    REQUIRE(ic.getCache().empty());
}

TEST_CASE("Complex expiration", "[LocalMsgIdCache]")
{
    TransparentLocalMsgIdCache ic{10ms, 2};

    REQUIRE(ic.insert(ADDR1, 0));
    REQUIRE(ic.insert(ADDR2, 1));
    REQUIRE(ic.insert(ADDR3, 2));
    REQUIRE_FALSE(ic.insert(ADDR3, 2));
    REQUIRE(ic.insert(ADDR2, 10));
    REQUIRE(ic.getCache() == C{
        {ADDR1, {{3, {0}}}},
        {ADDR2, {{3, {1, 10}}}},
        {ADDR3, {{3, {2}}}},
    });
    REQUIRE(ic.getTickNum() == 0);

    std::this_thread::sleep_for(15ms);

    REQUIRE(ic.insert(ADDR1, 200));
    REQUIRE_FALSE(ic.insert(ADDR1, 200));
    REQUIRE_FALSE(ic.insert(ADDR3, 2));
    REQUIRE(ic.getCache() == C{
        {ADDR1, {{3, {0}}, {4, {200}}}},
        {ADDR2, {{3, {1, 10}}}},
        {ADDR3, {{3, {2}}}},
    });
    REQUIRE(ic.getTickNum() == 1);

    std::this_thread::sleep_for(10ms);

    REQUIRE(ic.insert(ADDR2, 100));
    REQUIRE_FALSE(ic.insert(ADDR3, 2));
    REQUIRE(ic.getCache() == C{
        {ADDR1, {{3, {0}}, {4, {200}}}},
        {ADDR2, {{3, {1, 10}}, {5, {100}}}},
        {ADDR3, {{3, {2}}}},
    });
    REQUIRE(ic.getTickNum() == 2);

    std::this_thread::sleep_for(10ms);

    REQUIRE(ic.getCache() == C{
        {ADDR1, {{4, {200}}}},
        {ADDR2, {{5, {100}}}},
    });
    REQUIRE(ic.getTickNum() == 3);

    std::this_thread::sleep_for(10ms);

    REQUIRE(ic.insert(ADDR1, 200));
    REQUIRE(ic.getCache() == C{
        {ADDR1, {{7, {200}}}},
        {ADDR2, {{5, {100}}}},
    });
    REQUIRE(ic.getTickNum() == 4);

    std::this_thread::sleep_for(10ms);

    REQUIRE(ic.getCache() == C{
        {ADDR1, {{7, {200}}}},
    });
    REQUIRE(ic.getTickNum() == 5);

    std::this_thread::sleep_for(20ms);

    REQUIRE(ic.getCache().empty());
    REQUIRE(ic.getTickNum() == 7);
}
