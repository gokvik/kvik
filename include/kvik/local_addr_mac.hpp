/**
 * @file local_addr_mac.hpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Local layer address container for MAC address
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <string>
#include <vector>

#include "kvik/local_addr.hpp"

namespace kvik
{
    /**
     * @brief Local layer address container for MAC address
     */
    struct LocalAddrMAC : public LocalAddr
    {
        /**
         * @brief Constructs a new object
         *
         * @param mac MAC address (00:00:00:00:00:00 if `nullptr`)
         */
        LocalAddrMAC(const uint8_t *mac = nullptr);

        /**
         * @brief Constructs a new object from 00:00:00:00:00:00 MAC address
         *
         * @return New `LocalAddrMAC` object
         */
        static LocalAddrMAC zeroes();

        /**
         * @brief Constructs a new object from broadcast MAC address
         *
         * @return New `LocalAddrMAC` object
         */
        static LocalAddrMAC broadcast();

        /**
         * @brief Converts `LocalAddrMAC` to array of bytes
         *
         * @param mac MAC address storage pointer
         */
        void toBytes(uint8_t *mac) const;
    };
}

// Define hasher function
template <>
struct std::hash<kvik::LocalAddrMAC>
{
    std::size_t operator()(kvik::LocalAddrMAC const &addr) const noexcept
    {
        return std::hash<kvik::LocalAddr>{}(addr);
    }
};
