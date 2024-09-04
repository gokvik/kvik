/**
 * @file logger.hpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Logger for Kvik
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

    /**
     * @brief Logging handler function
     * @param msgLevel Message log level
     * @param logTag Logging tag of this file
     * @param fmt Formatting string
     * @param ... Variadic arguments in `printf`-like style
     */
    void logFunc(LogLevel msgLevel, const char *logTag, const char *fmt, ...);

#define KVIK_LOGD(fmt, ...) logFunc(LogLevel::DEBUG, KVIK_LOG_TAG, fmt, ##__VA_ARGS__)
#define KVIK_LOGI(fmt, ...) logFunc(LogLevel::INFO, KVIK_LOG_TAG, fmt, ##__VA_ARGS__)
#define KVIK_LOGW(fmt, ...) logFunc(LogLevel::WARN, KVIK_LOG_TAG, fmt, ##__VA_ARGS__)
#define KVIK_LOGE(fmt, ...) logFunc(LogLevel::ERROR, KVIK_LOG_TAG, fmt, ##__VA_ARGS__)
} // namespace kvik
