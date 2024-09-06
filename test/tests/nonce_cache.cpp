#include <chrono>
#include <thread>

#include <catch2/catch_test_macros.hpp>

#include "kvik/local_addr.hpp"
#include "kvik/nonce_cache.hpp"

using namespace kvik;
using namespace std::chrono_literals;

/**
 * @brief Transparent version of `NonceCache` for testing
 */
class TransparentNonceCache : public NonceCache
{
public:
    using NonceCache::NonceCache;
    using Cache = NonceCache::Cache;

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
using C = TransparentNonceCache::Cache;

uint8_t MAX_AGE = 2;
LocalAddr ADDR1{{0x01}};
LocalAddr ADDR2{{0x02, 0x03}};
LocalAddr ADDR3;

TEST_CASE("Empty ticks", "[NonceCache]")
{
    TransparentNonceCache nc{10ms, MAX_AGE};
    std::this_thread::sleep_for(25ms);
    CHECK(nc.getTickNum() == 2);
    CHECK(nc.getCache().empty());
}

TEST_CASE("Insert", "[NonceCache]")
{
    TransparentNonceCache nc{10ms, MAX_AGE};

    SECTION("Simple")
    {
        REQUIRE(nc.insert(ADDR1, 0));
        CHECK(nc.getCache() == C{{ADDR1, {{MAX_AGE + 1, {0}}}}});
    }

    SECTION("Multiple addresses")
    {
        REQUIRE(nc.insert(ADDR1, 0));
        REQUIRE(nc.insert(ADDR2, 1));
        REQUIRE(nc.insert(ADDR3, 2));
        CHECK(nc.getCache() == C{
            {ADDR1, {{MAX_AGE + 1, {0}}}},
            {ADDR2, {{MAX_AGE + 1, {1}}}},
            {ADDR3, {{MAX_AGE + 1, {2}}}},
        });
    }

    SECTION("Multiple addresses with same nonces")
    {
        REQUIRE(nc.insert(ADDR1, 10));
        REQUIRE(nc.insert(ADDR2, 10));
        REQUIRE(nc.insert(ADDR3, 10));
        CHECK(nc.getCache() == C{
            {ADDR1, {{MAX_AGE + 1, {10}}}},
            {ADDR2, {{MAX_AGE + 1, {10}}}},
            {ADDR3, {{MAX_AGE + 1, {10}}}},
        });
    }

    SECTION("Duplicate")
    {
        REQUIRE(nc.insert(ADDR1, 10));
        REQUIRE_FALSE(nc.insert(ADDR1, 10));
        CHECK(nc.getCache() == C{{ADDR1, {{MAX_AGE + 1, {10}}}}});
    }

    SECTION("Same address, different nonce")
    {
        REQUIRE(nc.insert(ADDR1, 10));
        REQUIRE(nc.insert(ADDR1, 20));
        CHECK(nc.getCache() == C{{ADDR1, {{MAX_AGE + 1, {10, 20}}}}});
    }

    CHECK(nc.getTickNum() == 0);
}

TEST_CASE("Basic expiration", "[NonceCache]")
{
    TransparentNonceCache nc{10ms, 5};

    REQUIRE(nc.insert(ADDR1, 0));
    REQUIRE(nc.getCache() == C{{ADDR1, {{6, {0}}}}});

    for (int i = 0; i < 5; i++) {
        std::this_thread::sleep_for(10ms);
        REQUIRE(nc.getCache() == C{{ADDR1, {{6, {0}}}}});
    }

    std::this_thread::sleep_for(10ms);
    REQUIRE(nc.getCache().empty());
}

TEST_CASE("Complex expiration", "[NonceCache]")
{
    TransparentNonceCache nc{10ms, 2};

    REQUIRE(nc.insert(ADDR1, 0));
    REQUIRE(nc.insert(ADDR2, 1));
    REQUIRE(nc.insert(ADDR3, 2));
    REQUIRE_FALSE(nc.insert(ADDR3, 2));
    REQUIRE(nc.insert(ADDR2, 10));
    REQUIRE(nc.getCache() == C{
        {ADDR1, {{3, {0}}}},
        {ADDR2, {{3, {1, 10}}}},
        {ADDR3, {{3, {2}}}},
    });
    REQUIRE(nc.getTickNum() == 0);

    std::this_thread::sleep_for(15ms);

    REQUIRE(nc.insert(ADDR1, 200));
    REQUIRE_FALSE(nc.insert(ADDR1, 200));
    REQUIRE_FALSE(nc.insert(ADDR3, 2));
    REQUIRE(nc.getCache() == C{
        {ADDR1, {{3, {0}}, {4, {200}}}},
        {ADDR2, {{3, {1, 10}}}},
        {ADDR3, {{3, {2}}}},
    });
    REQUIRE(nc.getTickNum() == 1);

    std::this_thread::sleep_for(10ms);

    REQUIRE(nc.insert(ADDR2, 100));
    REQUIRE_FALSE(nc.insert(ADDR3, 2));
    REQUIRE(nc.getCache() == C{
        {ADDR1, {{3, {0}}, {4, {200}}}},
        {ADDR2, {{3, {1, 10}}, {5, {100}}}},
        {ADDR3, {{3, {2}}}},
    });
    REQUIRE(nc.getTickNum() == 2);

    std::this_thread::sleep_for(10ms);

    REQUIRE(nc.getCache() == C{
        {ADDR1, {{4, {200}}}},
        {ADDR2, {{5, {100}}}},
    });
    REQUIRE(nc.getTickNum() == 3);

    std::this_thread::sleep_for(10ms);

    REQUIRE(nc.insert(ADDR1, 200));
    REQUIRE(nc.getCache() == C{
        {ADDR1, {{7, {200}}}},
        {ADDR2, {{5, {100}}}},
    });
    REQUIRE(nc.getTickNum() == 4);

    std::this_thread::sleep_for(10ms);

    REQUIRE(nc.getCache() == C{
        {ADDR1, {{7, {200}}}},
    });
    REQUIRE(nc.getTickNum() == 5);

    std::this_thread::sleep_for(20ms);

    REQUIRE(nc.getCache().empty());
    REQUIRE(nc.getTickNum() == 7);
}
