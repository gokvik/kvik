/**
 * @file local_peer.hpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Local layer peer info
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <array>
#include <cstdint>

#include "kvik/local_addr.hpp"

namespace kvik
{
    // Forward declaration
    struct LocalPeer;

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

        /**
         * @brief Converts `RetainedLocalPeer` to `LocalPeer`
         *
         * @return Local peer info
         */
        const LocalPeer unretain() const;
    };

    /**
     * @brief Local layer peer info
     */
    struct LocalPeer
    {
        LocalAddr addr = {};  //!< Peer address
        uint16_t channel = 0; //!< Wireless channel (only for wireless local layers)

        /**
         * @brief Peer preference (weight)
         *
         * Used for gateway selection.
         * Local layer specific, thus not comparable between different local
         * layers.
         * Higher value means higher preference.
         */
        int16_t pref = 0;

        /**
         * @brief Gateway timestamp in milliseconds since epoch
         *
         * Used for time synchronization.
         */
        uint64_t ts = 0;

        bool operator==(const LocalPeer &other) const
        {
            return addr == other.addr;
        }

        bool operator!=(const LocalPeer &other) const
        {
            return !this->operator==(other);
        }

        // Based on preference
        bool operator<(const LocalPeer &other) const
        {
            return pref < other.pref;
        }

        // Based on preference
        bool operator>(const LocalPeer &other) const
        {
            return pref > other.pref;
        }

        bool empty() const
        {
            return addr.empty();
        }

        /**
         * @brief Converts internal representation into printable string
         * @return Printable string representation
         */
        std::string toString() const;

        /**
         * @brief Converts `LocalPeer` to `RetainedLocalPeer`
         *
         * See `RetainedLocalPeer`'s docstring for usage details.
         *
         * @return Retained local peer info
         */
        const RetainedLocalPeer retain() const;
    };
}
