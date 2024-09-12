/**
 * @file log_level.cpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Log level for Kvik
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "kvik/log_level.hpp"

namespace kvik
{
    // Kvik's local log level is ignored
    // Instead, use standard ESP-IDF log configuration
    LogLevel logLevel = LogLevel::DEBUG;
} // namespace kvik
