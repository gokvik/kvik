/**
 * @file local_msg.hpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Local message classes
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <chrono>
#include <climits>
#include <cstdint>
#include <string>
#include <vector>

#include "kvik/local_addr.hpp"
#include "kvik/node_types.hpp"
#include "kvik/pub_sub_struct.hpp"

namespace kvik
{
    //! RSSI "unknown" value
    constexpr int16_t MSG_RSSI_UNKNOWN = INT16_MIN;

    /**
     * @brief Local message types
     */
    enum class LocalMsgType : uint8_t
    {
        NONE = 0x00,
        OK = 0x01,
        FAIL = 0x02,
        PROBE_REQ = 0x10,
        PROBE_RES = 0x11,
        PUB_SUB_UNSUB = 0x20,
        SUB_DATA = 0x21,
    };

    /**
     * @brief Local message FAIL reason
     */
    enum class LocalMsgFailReason : uint8_t
    {
        //! No/unknown failure
        NONE = 0x00,

        /**
         * @brief Duplicate message ID (i.e. replay attack protection,
         * deduplication)
         *
         * Currently unused in FAIL messages as an attacker could DoS the node
         * just by sending duplicates in loop.
         */
        DUP_ID = 0x01,

        /**
         * @brief Invalid timestamp (i.e. replay attack protection)
         *
         * Currently unused in FAIL messages as an attacker could DoS the node
         * just by sending duplicates in loop.
         */
        INVALID_TS = 0x02,

        /**
         * @brief Processing has failed
         *
         * E.g. transmission over remote layer, relaying, etc.
         */
        PROCESSING_FAILED = 0x03,

        /**
         * @brief Unknown message sender
         *
         * Currently unused in FAIL messages.
         */
        UNKNOWN_SENDER = 0x04,
    };

    /**
     * @brief Helper to convert `LocalMsgType` to string representation.
     *
     * @param mt Message type
     * @return String representation
     */
    const char *localMsgTypeToStr(LocalMsgType mt) noexcept;

    /**
     * @brief Helper to convert `LocalMsgFailReason` to string representation.
     *
     * @param fr Fail reason
     * @return String representation
     */
    const char *localMsgFailReasonToStr(LocalMsgFailReason fr) noexcept;

    /**
     * @brief Local message representation
     *
     * Used primarily for communication between `LocalLayer` and `Node` classes.
     *
     * TODO: optimize structure size
     */
    struct LocalMsg
    {
        LocalMsgType type = LocalMsgType::NONE; //!< Type of message
        LocalAddr addr = {};                    //!< Source/destination address
        LocalAddr relayedAddr = {};             //!< Relayed address (processed by relay node)
        std::vector<PubData> pubs;              //!< Publications (PUB_SUB_UNSUB only)
        std::vector<std::string> subs;          //!< Topics of subscriptions (PUB_SUB_UNSUB only)
        std::vector<std::string> unsubs;        //!< Topics of unsubscriptions (PUB_SUB_UNSUB only)
        std::vector<SubData> subsData;          //!< Subscriptions data (SUB_DATA only)

        // Additional data
        uint16_t id = 0;                                          //!< Message ID
        uint16_t ts = 0;                                          //!< Timestamp (in configured units)
        uint16_t reqId = 0;                                       //!< Message ID of corresponding request message (OK, FAIL, PROBE_RES only)
        NodeType nodeType = NodeType::UNKNOWN;                    //!< This node type
        LocalMsgFailReason failReason = LocalMsgFailReason::NONE; //!< Fail reason (FAIL only)

        /**
         * @brief RSSI corresponding to the message
         *
         * Only relevant for RF local layers.
         *
         * In case of received message contains it's RSSI,
         * in case of to be sent message should contain RSSI of corresponding
         * received message so that correct data rate can be chosen.
         */
        int16_t rssi = MSG_RSSI_UNKNOWN;

        /**
         * @brief Peer preference (weight)
         *
         * Used for gateway selection.
         * Local layer specific, thus not comparable between different local
         * layers.
         * Higher value means higher preference.
         *
         * PROBE_RES only.
         */
        int16_t pref = INT16_MIN;

        /**
         * @brief Gateway time difference
         *
         * Used for time synchronization.
         *
         * Calculated as gateway's timestamp in PROBE_RES minus local steady
         * clock time.
         *
         * PROBE_RES only.
         */
        std::chrono::milliseconds tsDiff = std::chrono::milliseconds(0);

        bool operator==(const LocalMsg &other) const;
        bool operator!=(const LocalMsg &other) const;

        /**
         * @brief Converts `LocalMsg` to printable string
         *
         * Primarily for logging purposes
         *
         * @return String representation of contained data
         */
        std::string toString() const;
    };
} // namespace kvik
