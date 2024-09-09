/**
 * @file timer.cpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @copyright Copyright (c) 2024
 */

#include <chrono>
#include <future>

#include <catch2/catch_test_macros.hpp>

#include "kvik/timer.hpp"

using namespace kvik;
using namespace std::chrono_literals;

TEST_CASE("10 timer ticks", "[Timer]")
{
    std::promise<void> promise;

    // Store start timestamp
    auto startTS = std::chrono::system_clock::now();

    // Instantiate timer
    Timer timer(10ms, [&promise]()
                { promise.set_value(); });

    for (size_t i = 1; i <= 10; i++)
    {
        // Wait for first tick
        promise.get_future().wait();

        // Reset promise
        promise = std::promise<void>();

        auto duration = std::chrono::system_clock::now() - startTS;
        CHECK(duration > 9ms * i);
        CHECK(duration < 11ms * i);
    }
}

TEST_CASE("Next exec modification", "[Timer]")
{
    std::promise<void> promise;

    // Store start timestamp
    auto startTS = std::chrono::system_clock::now();

    // Instantiate timer
    Timer timer(10ms, [&promise, &timer]() {
        promise.set_value();
        timer.setNextExec(std::chrono::steady_clock::now() + 20ms);
    });

    for (size_t i = 1; i <= 10; i++)
    {
        // Wait for first tick
        promise.get_future().wait();

        // Reset promise
        promise = std::promise<void>();

        auto duration = std::chrono::system_clock::now() - startTS;

        // Calls should be at `startTs` +10ms, +30ms, +50ms,...
        CHECK(duration > 19ms * i - 10ms);
        CHECK(duration < 21ms * i - 10ms);
    }
}

TEST_CASE("Next exec modification - one-time change", "[Timer]")
{
    std::promise<void> promise;

    // Store start timestamp
    auto startTS = std::chrono::system_clock::now();
    int cnt = 0;

    // Instantiate timer
    Timer timer(10ms, [&promise, &timer, &cnt]() {
        if (cnt == 0)
        {
            timer.setNextExec(std::chrono::steady_clock::now() + 30ms);
        }
        if (cnt == 2)
        {
            promise.set_value();
        }
        cnt++;
    });

    promise.get_future().wait();

    auto duration = std::chrono::system_clock::now() - startTS;
    CHECK(duration > 49ms);
    CHECK(duration < 51ms);
}

TEST_CASE("Next exec modification - external one-time change", "[Timer]")
{
    std::promise<void> promise;

    // Store start timestamp
    auto startTS = std::chrono::system_clock::now();

    // Instantiate timer
    Timer timer(10ms, [&promise]()
                { promise.set_value(); });

    timer.setNextExec(std::chrono::steady_clock::now() + 30ms);

    promise.get_future().wait();
    promise = std::promise<void>();

    // Check first duration
    auto duration = std::chrono::system_clock::now() - startTS;
    CHECK(duration > 29ms);
    CHECK(duration < 31ms);

    for (size_t i = 1; i <= 5; i++)
    {
        promise.get_future().wait();
        promise = std::promise<void>();

        // Check more durations
        duration = std::chrono::system_clock::now() - startTS;
        CHECK(duration > 9ms * i + 30ms);
        CHECK(duration < 11ms * i + 30ms);
    }
}
