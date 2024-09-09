/**
 * @file local_msg.hpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Local message classes
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <climits>
#include <cstdint>
#include <string>

#include "kvik/local_addr.hpp"
#include "kvik/node_types.hpp"

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
        PUB = 0x20,
        SUB_REQ = 0x30,
        SUB_DATA = 0x31,
        UNSUB = 0x32,
    };

    /**
     * @brief Local message FAIL reason
     */
    enum class LocalMsgFailReason : uint8_t
    {
        NONE = 0x00,              //!< No/unknown failure
        DUP_NONCE = 0x01,         //!< Duplicate nonce (i.e. replay attack protection)
        INVALID_TS = 0x02,        //!< Invalid timestamp (i.e. replay attack protection)
        MALFORMED = 0x03,         //!< Malformed message (invalid length, checksum, parameters, etc.)
        PROCESSING_FAILED = 0x04, //!< Processing (transmission over remote layer, relaying, etc.) has failed
    };

    /**
     * @brief Helper to convert `LocalMsgType` to string representation.
     *
     * @param mt Message type
     * @return String representation
     */
    constexpr const char *localMsgTypeToStr(LocalMsgType mt) noexcept;

    /**
     * @brief Helper to convert `LocalMsgFailReason` to string representation.
     *
     * @param fr Fail reason
     * @return String representation
     */
    constexpr const char *localMsgFailReasonToStr(LocalMsgFailReason fr) noexcept;

    /**
     * @brief Local message representation
     *
     * Used primarily for communication between `LocalLayer` and `Node` classes.
     */
    struct LocalMsg
    {
        LocalMsgType type = LocalMsgType::NONE; //!< Type of message
        LocalAddr addr = {};                    //!< Source/destination address
        LocalAddr relayedAddr = {};             //!< Relayed address (processed by relay node)
        std::string topic = "";                 //!< Topic of message
        std::string payload = "";               //!< Payload of message

        // Additional data
        NodeType nodeType = NodeType::UNKNOWN;                    //!< This node type
        LocalMsgFailReason failReason = LocalMsgFailReason::NONE; //!< Fail reason

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

        uint16_t ts = 0;    //!< Timestamp (in configured units)
        uint16_t nonce = 0; //!< Nonce

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

// Define hasher function
template <>
struct std::hash<kvik::LocalMsg>
{
    std::size_t operator()(kvik::LocalMsg const &msg) const noexcept
    {
        return std::hash<kvik::LocalMsgType>{}(msg.type) +
               std::hash<kvik::LocalAddr>{}(msg.addr) +
               std::hash<kvik::LocalAddr>{}(msg.relayedAddr) +
               std::hash<std::string>{}(msg.topic) +
               std::hash<std::string>{}(msg.payload);
    }
};
