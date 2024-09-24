/**
 * @file limits.hpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Numerical limits
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <climits>
#include <cstdint>

namespace kvik
{
    //! Peer preference "unknown" value
    constexpr int16_t PREF_UNKNOWN = INT16_MIN;

    //! RSSI "unknown" value
    constexpr int16_t RSSI_UNKNOWN = INT16_MIN;
} // namespace kvik
