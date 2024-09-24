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
#include <chrono>
#include <cstdint>

#include "kvik/limits.hpp"
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
        /**
         * @brief Peer address
         */
        LocalAddr addr = {};

        /**
         * @brief Communication channel
         *
         * Primarily for wireless local layers, but can be used generally.
         * Value 0 represents default channel.
         */
        uint16_t channel = 0;

        /**
         * @brief Peer preference (weight)
         *
         * Used for gateway selection.
         * Local layer specific, thus not comparable between different local
         * layers.
         * Higher value means higher preference.
         */
        int16_t pref = PREF_UNKNOWN;

        /**
         * @brief RSSI of probe response from the peer
         *
         * Only relevant for RF local layers.
         */
        int16_t rssi = RSSI_UNKNOWN;

        /**
         * @brief Gateway time difference
         *
         * Used for time synchronization.
         *
         * Calculated as gateway's timestamp in PROBE_RES minus local steady
         * clock time.
         */
        std::chrono::milliseconds tsDiff = std::chrono::milliseconds(0);

        bool operator==(const LocalPeer &other) const
        {
            return addr == other.addr;
        }

        bool operator!=(const LocalPeer &other) const
        {
            return !this->operator==(other);
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

// Define hasher function
template <>
struct std::hash<kvik::LocalPeer>
{
    std::size_t operator()(kvik::LocalPeer const &peer) const noexcept
    {
        return std::hash<kvik::LocalAddr>{}(peer.addr) +
               std::hash<uint16_t>{}(peer.channel);
    }
};
