/**
 * @file main.cpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Testing main file
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <catch2/catch_session.hpp>

#include "kvik/log_level.hpp"

using namespace kvik;

int main(int argc, char *argv[])
{
    // Set highest possible logging verbosity
    logLevel = LogLevel::DEBUG;

    return Catch::Session().run(argc, argv);
}
