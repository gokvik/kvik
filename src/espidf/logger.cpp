/**
 * @file logger.cpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Logger for Kvik
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <cstdarg>
#include <cstdio>

#include "esp_log.h"

#include "kvik/logger.hpp"

namespace kvik
{
    // Kvik's local log level is ignored
    // Instead, use standard ESP-IDF log configuration
    LogLevel logLevel = LogLevel::DEBUG;

    void logFunc(LogLevel msgLevel, const char *logTag, const char *fmt, ...)
    {
        esp_log_level_t espMsgLevel;

        switch (msgLevel)
        {
        case LogLevel::DEBUG:
            espMsgLevel = ESP_LOG_DEBUG;
            break;
        case LogLevel::INFO:
            espMsgLevel = ESP_LOG_INFO;
            break;
        case LogLevel::WARN:
            espMsgLevel = ESP_LOG_WARN;
            break;
        case LogLevel::ERROR:
            espMsgLevel = ESP_LOG_ERROR;
            break;
        default:
            espMsgLevel = ESP_LOG_NONE;
            break;
        }

        va_list args;
        va_start(args, fmt);
        esp_log_writev(espMsgLevel, logTag, fmt, args);
        va_end(args);
    }
} // namespace kvik
