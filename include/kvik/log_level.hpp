/**
 * @file log_level.hpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Log level for Kvik
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <cstdint>

namespace kvik
{
    enum class LogLevel : uint_fast8_t
    {
        DEBUG = 0,
        INFO = 1,
        WARN = 2,
        ERROR = 3,
        OFF = 255,
    };

    /**
     * @brief Global log level
     *
     * On ESP-IDF it's ignored (logging is handled by standard ESP-IDF logger).
     */
    extern LogLevel logLevel;
} // namespace kvik
