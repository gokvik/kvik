/**
 * @file local_msg_prep.hpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Local message preparation helpers
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <chrono>

#include "kvik/local_msg.hpp"

namespace kvik
{
    /**
     * @brief Prepares local message
     *
     * Fills message ID and timestamp.
     *
     * @param msg Message (modified in-place)
     * @param tsDiff Timestamp difference between nodes
     * @param timeUnit Time unit (see `NodeConfig.MsgIdCache.timeUnit`)
     */
    void prepLocalMsg(LocalMsg &msg, std::chrono::milliseconds tsDiff,
                      std::chrono::milliseconds timeUnit);
} // namespace kvik
