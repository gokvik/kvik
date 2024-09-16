/**
 * @file logger.hpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Logger for Kvik
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include "kvik/log_level.hpp"

namespace kvik
{
    /**
     * @brief Logging handler function
     * @param msgLevel Message log level
     * @param logTag Logging tag of this file
     * @param fmt Formatting string
     * @param ... Variadic arguments in `printf`-like style
     */
    void logFunc(LogLevel msgLevel, const char *logTag, const char *fmt, ...);

#define KVIK_LOGD(fmt, ...) logFunc(LogLevel::DEBUG, KVIK_LOG_TAG, "@%s: " fmt, __func__, ##__VA_ARGS__)
#define KVIK_LOGI(fmt, ...) logFunc(LogLevel::INFO, KVIK_LOG_TAG, "@%s: " fmt, __func__, ##__VA_ARGS__)
#define KVIK_LOGW(fmt, ...) logFunc(LogLevel::WARN, KVIK_LOG_TAG, "@%s: " fmt, __func__, ##__VA_ARGS__)
#define KVIK_LOGE(fmt, ...) logFunc(LogLevel::ERROR, KVIK_LOG_TAG, "@%s: " fmt, __func__, ##__VA_ARGS__)
} // namespace kvik
