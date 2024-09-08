/**
 * @file local_addr.hpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Local layer address container
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace kvik
{
    /**
     * @brief Local layer address container
     *
     * Internal representation is decisive. Two addresses are the same if they
     * have the same internal representation.
     *
     * This is base for other local address types, like MAC.
     */
    struct LocalAddr
    {
        std::vector<uint8_t> addr; //!< Internal address representation

        bool operator==(const LocalAddr &other) const
        {
            return addr == other.addr;
        }

        bool operator!=(const LocalAddr &other) const
        {
            return !this->operator==(other);
        }

        /**
         * @brief Checks whether the address is empty
         *
         * @return true Local address is empty
         * @return false Local address is not empty
         */
        bool empty() const
        {
            return addr.empty();
        }

        /**
         * @brief Converts internal representation into printable string
         *
         * This is also used during comunication with remote layer protocols
         * (like MQTT).
         *
         * Most generic approach used by base class is to convert `addr` to
         * hexdump.
         *
         * @return Printable string representation
         */
        std::string toString() const
        {
            std::string str;
            char buf[3];
            for (const auto ch : addr)
            {
                sprintf(buf, "%02x", ch);
                str += buf;
            }
            return str;
        }
    };
}

// Define hasher function
template <>
struct std::hash<kvik::LocalAddr>
{
    std::size_t operator()(kvik::LocalAddr const &addr) const noexcept
    {
        // Convert internal representation to string
        std::string addrStr(addr.addr.begin(), addr.addr.end());

        // Hash it
        return std::hash<std::string>{}(addrStr);
    }
};
