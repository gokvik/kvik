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

#include "kvik/logger.hpp"

namespace kvik
{
    // Set default log level
    LogLevel logLevel = LogLevel::INFO;

    void logFunc(LogLevel msgLevel, const char *logTag, const char *fmt, ...)
    {
        if (logLevel > msgLevel)
        {
            // Skip too verbose messages
            return;
        }

#if KVIK_LOG_NO_COLORS
        switch (msgLevel)
        {
        case LogLevel::DEBUG:
            fprintf(stderr, "[D] %s: ", logTag);
            break;
        case LogLevel::INFO:
            fprintf(stderr, "[I] %s: ", logTag);
            break;
        case LogLevel::WARN:
            fprintf(stderr, "[W] %s: ", logTag);
            break;
        case LogLevel::ERROR:
            fprintf(stderr, "[E] %s: ", logTag);
            break;
        default:
            fprintf(stderr, "[?] %s: ", logTag);
            break;
        }
#else
        switch (msgLevel)
        {
        case LogLevel::DEBUG:
            fprintf(stderr, "\033[0;34m[D] %s: ", logTag);
            break;
        case LogLevel::INFO:
            fprintf(stderr, "\033[0;36m[I] %s: ", logTag);
            break;
        case LogLevel::WARN:
            fprintf(stderr, "\033[0;33m[W] %s: ", logTag);
            break;
        case LogLevel::ERROR:
            fprintf(stderr, "\033[0;31m[E] %s: ", logTag);
            break;
        default:
            fprintf(stderr, "\033[0m[?] %s: ", logTag);
            break;
        }
#endif

        va_list args;
        va_start(args, fmt);
        vfprintf(stderr, fmt, args);
        va_end(args);

#if KVIK_LOG_NO_COLORS
        fprintf(stderr, "\n");
#else
        fprintf(stderr, "\033[0m\n");
#endif
    }
} // namespace kvik
