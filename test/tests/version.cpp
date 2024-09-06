#include <string>

#include <catch2/catch_test_macros.hpp>

#include "kvik/version.hpp"
#include "kvik/logger.hpp"

using namespace kvik;

TEST_CASE("Version is not unknown", "[Version]")
{
    CHECK(!VERSION_UNKNOWN);
    CHECK(std::string(VERSION) != "unknown");
}
