#include "kvik/local_addr.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace kvik;

TEST_CASE("Empty", "[LocalAddr]")
{
    REQUIRE(LocalAddr().empty());
}

TEST_CASE("Non-empty", "[LocalAddr]")
{
    REQUIRE(!LocalAddr({{0x00}}).empty());
}

TEST_CASE("Comparison", "[LocalAddr]")
{
    auto addr1 = LocalAddr({{0x00}});
    auto addr2 = LocalAddr({{0x01}});
    auto addr3 = LocalAddr({{0x00}});
    auto addr4 = LocalAddr({{0x00, 0x01}});

    SECTION("Equality")
    {
        REQUIRE(addr1 == addr3);
    }

    SECTION("Non-equality")
    {
        REQUIRE(addr1 != addr2);
    }

    SECTION("Non-equality of with common prefix")
    {
        REQUIRE(addr1 != addr4);
    }

    SECTION("Non-equality of with common suffix")
    {
        REQUIRE(addr2 != addr4);
    }
}

TEST_CASE("String representation", "[LocalAddr]")
{
    REQUIRE(LocalAddr({{0x00, 0x11, 0xAB}}).toString() == "0011ab");
}
