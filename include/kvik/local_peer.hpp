/**
 * @file local_peer.hpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Local layer peer info
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <algorithm>
#include <array>
#include <cstdint>

#include "kvik/local_addr.hpp"

namespace kvik
{
    /**
     * @brief Retained local layer peer info
     *
     * Special form of `LocalPeer` without any dynamic memory allocation.
     * Intended for storage in RTC memory of IoT microcontrollers (during deep
     * sleep). Not used for anything else.
     */
    struct RetainedLocalPeer
    {
        std::array<uint8_t, 32> addr = {};
        uint8_t addrLen = 0;
        uint16_t channel = 0;
    };

    /**
     * @brief Local layer peer info
     */
    struct LocalPeer
    {
        LocalAddr addr = {};  //!< Peer address
        uint16_t channel = 0; //!< Wireless channel (only for wireless local layers)

        bool operator==(const LocalPeer &other) const
        {
            return addr == other.addr;
        }

        bool operator!=(const LocalPeer &other) const
        {
            return !this->operator==(other);
        }

        inline bool empty() const
        {
            return addr.empty();
        }

        /**
         * @brief Converts internal representation into printable string
         * @return Printable string representation
         */
        inline std::string toString() const
        {
            return addr.toString();
        }

        /**
         * @brief Converts `LocalPeer` to `RetainedLocalPeer`
         *
         * See `RetainedLocalPeer`'s docstring for usage details.
         *
         * @return Retained local peer info
         */
        inline const RetainedLocalPeer retain() const
        {
            RetainedLocalPeer rlp;
            std::copy_n(addr.addr.begin(), rlp.addr.max_size(), rlp.addr.begin());
            rlp.addrLen = addr.addr.size();
            rlp.channel = channel;
            return rlp;
        }
    };
}

// Define hasher function
template <>
struct std::hash<kvik::LocalPeer>
{
    std::size_t operator()(kvik::LocalPeer const &peer) const noexcept
    {
        return std::hash<kvik::LocalAddr>{}(peer.addr);
    }
};
