/**
 * @file node_types.hpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Node types
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <cstdint>

namespace kvik
{
    /**
     * @brief Node types enumeration
     *
     * Maximum length is 4 bits (16 options).
     */
    enum class NodeType : uint8_t
    {
        UNKNOWN = 0x00,
        CLIENT = 0x01,
        GATEWAY = 0x02,
        RELAY = 0x03,
    };
} // namespace kvik
