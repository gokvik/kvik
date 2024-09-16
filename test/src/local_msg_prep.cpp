/**
 * @file local_msg_prep.cpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Local message preparation helpers
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <chrono>
#include <mutex>

#include "kvik_testing/local_msg_prep.hpp"

using namespace std::chrono_literals;

static std::mutex mutex;
static uint16_t msgId = 0;

namespace kvik
{
    void prepLocalMsg(LocalMsg &msg, std::chrono::milliseconds tsDiff,
                      std::chrono::milliseconds timeUnit)
    {
        const std::scoped_lock lock{mutex};
        msg.id = msgId++;
        msg.ts = (std::chrono::steady_clock::now().time_since_epoch() +
                  tsDiff) /
                 timeUnit;
    }
} // namespace kvik
