/**
 * @file errors.hpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Base of Kvik error handling
 *
 * @copyright Copyright (c) 2023
 *
 */

#pragma once

#include <cstdint>
#include <string>

#if __cpp_exceptions
#include <exception>
#endif

namespace kvik
{
    /**
     * Kvik error code
     *
     * Anything else than `KvikErr::SUCCESS` is considered as failure.
     * The list of errors will expand with time.
     */
    enum class ErrCode : uint_fast16_t
    {
        SUCCESS = 0x0,
        GENERIC_FAILURE = 0x1,

        INVALID_ARG = 0x10,
        INVALID_SIZE = 0x11,
        NOT_FOUND = 0x12,
        NOT_SUPPORTED = 0x13,
        TIMEOUT = 0x14,
        TOO_MANY_FAILED_ATTEMPTS = 0x15,
        NO_GATEWAY = 0x16,

        // Error codes corresponding to `LocalMsgFailReason`
        MSG_DUP_ID = 0x101,
        MSG_INVALID_TS = 0x102,
        MSG_PROCESSING_FAILED = 0x103,
        MSG_UNKNOWN_SENDER = 0x104,
    };

#define KVIK_STRINGIZE_INNER(x) #x
#define KVIK_STRINGIZE(x) KVIK_STRINGIZE_INNER(x)

#if __cpp_exceptions
    /**
     * @brief Base Kvik exception
     *
     * Defined only if C++ exceptions are enabled.
     */
    class Exception : public std::exception
    {
        std::string m_msg;

    public:
        /**
         * @brief Constructs a new exception
         *
         * @param msg Message
         */
        explicit Exception(const std::string &msg)
            : m_msg{msg} {}

        /**
         * @brief Returns exception's message
         *
         * @return Message
         */
        virtual const char *what() const noexcept
        {
            return m_msg.c_str();
        }
    };

#define KVIK_THROW_EXC(msg) (throw Exception(__FILE__ ":" KVIK_STRINGIZE(__LINE__) ": " msg))
#else // __cpp_exceptions
#pragma message "KVIK: exceptions are disabled"
#define KVIK_THROW_EXC(msg) (abort())
#endif // __cpp_exceptions

#define KVIK_RETURN_ERROR(x)           \
    do {                               \
        ErrCode err = (x);             \
        if (err != ErrCode::SUCCESS) { \
            return err;                \
        }                              \
    } while (0)

#define KVIK_RETURN_ON_ERROR(x)        \
    do {                               \
        ErrCode err = (x);             \
        if (err != ErrCode::SUCCESS) { \
            return;                    \
        }                              \
    } while (0)

#define KVIK_THROW_EXC_ON_ERROR(x, msg) \
    do {                                \
        ErrCode err = (x);              \
        if (err != ErrCode::SUCCESS) {  \
            KVIK_THROW_EXC(msg);        \
        }                               \
    } while (0)
} // namespace kvik
