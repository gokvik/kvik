#include <catch2/catch_test_macros.hpp>

#include "kvik/local_addr_mac.hpp"

using namespace kvik;

TEST_CASE("Zeroed MAC is not empty", "[LocalAddrMAC]")
{
    REQUIRE(!LocalAddrMAC().empty());
}

TEST_CASE("Comparison", "[LocalAddrMAC]")
{
    uint8_t mac1[] = {0x00, 0x11, 0x23, 0x00, 0x55, 0xFF};
    uint8_t mac2[] = {0x00, 0x11, 0x23, 0x00, 0x55, 0xAA};
    uint8_t mac3[] = {0x00, 0x11, 0x23, 0x00, 0x55, 0xFF};
    uint8_t macZero[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t macBroadcast[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    SECTION("Equality")
    {
        REQUIRE(LocalAddrMAC(mac1) == LocalAddrMAC(mac3));
    }

    SECTION("Non-equality")
    {
        REQUIRE(LocalAddrMAC(mac1) != LocalAddrMAC(mac2));
    }

    SECTION("Default MAC is zeroed")
    {
        REQUIRE(LocalAddrMAC::zeroes() == LocalAddrMAC());
    }

    SECTION("Zeroed MAC equality")
    {
        REQUIRE(LocalAddrMAC::zeroes() == LocalAddrMAC(macZero));
    }

    SECTION("Broadcast MAC equality")
    {
        REQUIRE(LocalAddrMAC::broadcast() == LocalAddrMAC(macBroadcast));
    }
}

TEST_CASE("String representation", "[LocalAddrMAC]")
{
    uint8_t mac[] = {0x00, 0x11, 0x23, 0x00, 0x55, 0xFF};
    REQUIRE(LocalAddrMAC(mac).toString() == "0011230055ff");
}
