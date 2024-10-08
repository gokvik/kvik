/**
 * @file version.hpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Current version of Kvik
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

namespace kvik
{
    /**
     * @brief Version of Kvik
     */
#ifdef GIT_VERSION
    constexpr const char* const VERSION = GIT_VERSION;
    constexpr const bool VERSION_UNKNOWN = false;
#else
    constexpr const char* const VERSION = "unknown";
    constexpr const bool VERSION_UNKNOWN = true;
#endif
} // namespace kvik
