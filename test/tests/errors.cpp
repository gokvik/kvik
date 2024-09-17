/**
 * @file errors.cpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @copyright Copyright (c) 2024
 */

#include <catch2/catch_test_macros.hpp>

#include "kvik/errors.hpp"

using namespace kvik;

TEST_CASE("Exception::what()", "[LocalPeer]")
{
    Exception exc("abc");
    CHECK(std::string(exc.what()) == "abc");
}
