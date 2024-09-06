/**
 * @file random.cpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @copyright Copyright (c) 2024
 */

#include <catch2/catch_test_macros.hpp>

#include "kvik/random.hpp"

using namespace kvik;

TEST_CASE("Simple random bytes", "[Random]")
{
    uint64_t n = 0;
    REQUIRE_NOTHROW(getRandomBytes(&n, sizeof(n)));
    REQUIRE(n != 0);
}

TEST_CASE("Zero length buffer", "[Random]")
{
    uint64_t n = 0;
    REQUIRE_NOTHROW(getRandomBytes(&n, 0));
    REQUIRE(n == 0);
}

TEST_CASE("Long buffer", "[Random]")
{
    size_t size = 5e6; // 5 MB

    // Allocate big buffer
    uint8_t *buf = (uint8_t *)calloc(size, 1);
    REQUIRE(buf != nullptr);

    // Sum up and check last 100 bytes
    uint16_t sum = 0;
    for (size_t i = 0; i < 100; i++)
    {
        sum += buf[size - i - 1];
    }
    REQUIRE(sum == 0);

    REQUIRE_NOTHROW(getRandomBytes(buf, size));

    // Sum up and check again
    for (size_t i = 0; i < 100; i++)
    {
        sum += buf[size - i - 1];
    }
    REQUIRE(sum != 0);
}
