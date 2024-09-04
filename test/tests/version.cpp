#include <catch2/catch_test_macros.hpp>

#include "kvik/version.hpp"
#include "kvik/logger.hpp"

using namespace kvik;

TEST_CASE("Version is not unknown", "[Version]")
{
    REQUIRE(VERSION != "unknown");
}
