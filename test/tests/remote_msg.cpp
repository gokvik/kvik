#include <catch2/catch_test_macros.hpp>

#include "kvik/remote_msg.hpp"

using namespace kvik;

TEST_CASE("Comparison", "[RemoteMsg]")
{
    RemoteMsg msg1;
    RemoteMsg msg2;

    SECTION("Equality")
    {
        REQUIRE(msg1 == msg2);
    }

    SECTION("Different topics")
    {
        msg2.topic = "1";
        REQUIRE(msg1 != msg2);
    }

    SECTION("Different payloads")
    {
        msg2.payload = "1";
        REQUIRE(msg1 != msg2);
    }
}
