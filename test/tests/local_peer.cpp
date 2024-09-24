/**
 * @file local_peer.cpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @copyright Copyright (c) 2024
 */

#include <chrono>

#include <catch2/catch_test_macros.hpp>

#include "kvik/local_peer.hpp"

using namespace kvik;
using namespace std::chrono_literals;

TEST_CASE("Comparison", "[LocalPeer]")
{
    LocalPeer peer1;
    LocalPeer peer2;

    SECTION("Equality")
    {
        CHECK(peer1 == peer2);
    }

    SECTION("Different address")
    {
        peer2.addr.addr.push_back(0x01);
        CHECK(peer1 != peer2);
    }

    SECTION("Different channel")
    {
        // Treated as additional data
        peer2.channel = 1;
        CHECK(peer1 == peer2);
    }

    SECTION("Different time difference")
    {
        // Treated as additional data
        peer2.tsDiff = 100ms;
        CHECK(peer1 == peer2);
    }

    SECTION("Different preference")
    {
        // Treated as additional data
        peer1.pref = 0;
        peer2.pref = 100;
        CHECK(peer1 == peer2);
    }

    SECTION("Different RSSI")
    {
        // Treated as additional data
        peer1.rssi = 0;
        peer2.rssi = 100;
        CHECK(peer1 == peer2);
    }
}

TEST_CASE("Empty", "[LocalPeer]")
{
    LocalPeer peer;

    SECTION("Initially empty")
    {
        REQUIRE(peer.empty());
    }

    SECTION("Address not empty")
    {
        peer.addr.addr.push_back(0x01);
        REQUIRE(!peer.addr.empty());
        REQUIRE(!peer.empty());
    }
}

TEST_CASE("Retain and unretain empty", "[LocalPeer]")
{
    LocalPeer peer;

    RetainedLocalPeer retainedPeer = peer.retain();
    REQUIRE(retainedPeer.addrLen == 0);
    REQUIRE(retainedPeer.channel == 0);

    LocalPeer unretainedPeer = retainedPeer.unretain();
    REQUIRE(unretainedPeer.addr.empty());
    REQUIRE(unretainedPeer.channel == 0);
}

TEST_CASE("Retain and unretain", "[LocalPeer]")
{
    LocalAddr addr({{0x10, 0x20, 0x30}});
    LocalPeer peer = {
        .addr = addr,
        .channel = 100,
    };

    RetainedLocalPeer retainedPeer = peer.retain();
    REQUIRE(retainedPeer.addr[0] == 0x10);
    REQUIRE(retainedPeer.addr[1] == 0x20);
    REQUIRE(retainedPeer.addr[2] == 0x30);
    REQUIRE(retainedPeer.addrLen == 3);
    REQUIRE(retainedPeer.channel == 100);

    LocalPeer unretainedPeer = retainedPeer.unretain();
    REQUIRE(unretainedPeer == peer);
    REQUIRE(unretainedPeer.addr == peer.addr);
    REQUIRE(unretainedPeer.channel == peer.channel);
}

TEST_CASE("Retain and unretain super long address", "[LocalPeer]")
{
    size_t maxRetainedAddrSize = RetainedLocalPeer().addr.max_size();

    LocalPeer peer = {
        .channel = 100,
    };

    for (size_t i = 0; i < 3 * maxRetainedAddrSize; i++) {
        peer.addr.addr.push_back(0x01);
    }

    RetainedLocalPeer retainedPeer = peer.retain();

    for (size_t i = 0; i < maxRetainedAddrSize; i++) {
        REQUIRE(retainedPeer.addr[i] == 0x01);
    }
    REQUIRE(retainedPeer.addrLen == maxRetainedAddrSize);
    REQUIRE(retainedPeer.channel == 100);

    LocalPeer unretainedPeer = retainedPeer.unretain();
    REQUIRE(unretainedPeer != peer);
    REQUIRE(unretainedPeer.addr.addr.size() == maxRetainedAddrSize);
    REQUIRE(unretainedPeer.channel == peer.channel);
}
