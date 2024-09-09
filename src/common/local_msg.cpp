/**
 * @file local_msg.cpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Local message classes
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <string>

#include "kvik/local_msg.hpp"

namespace kvik
{
    constexpr const char *localMsgTypeToStr(LocalMsgType mt) noexcept
    {
        switch (mt)
        {
        case LocalMsgType::NONE:
            return "NONE";
        case LocalMsgType::OK:
            return "OK";
        case LocalMsgType::FAIL:
            return "FAIL";
        case LocalMsgType::PROBE_REQ:
            return "PROBE_REQ";
        case LocalMsgType::PROBE_RES:
            return "PROBE_RES";
        case LocalMsgType::PUB:
            return "PUB";
        case LocalMsgType::SUB_REQ:
            return "SUB_REQ";
        case LocalMsgType::SUB_DATA:
            return "SUB_DATA";
        case LocalMsgType::UNSUB:
            return "UNSUB";
        default:
            return "???";
        }
    }

    constexpr const char *localMsgFailReasonToStr(LocalMsgFailReason fr) noexcept
    {
        switch (fr)
        {
        case LocalMsgFailReason::NONE:
            return "NONE";
        case LocalMsgFailReason::DUP_NONCE:
            return "DUP_NONCE";
        case LocalMsgFailReason::INVALID_TS:
            return "INVALID_TS";
        case LocalMsgFailReason::MALFORMED:
            return "MALFORMED";
        case LocalMsgFailReason::PROCESSING_FAILED:
            return "PROCESSING_FAILED";
        default:
            return "???";
        }
    }

    std::string LocalMsg::toString() const
    {
        return std::string{localMsgTypeToStr(type)} + " " +
               (type == LocalMsgType::FAIL
                    ? (
                          std::string("(failed due to ") +
                          localMsgFailReasonToStr(failReason) +
                          ") ")
                    : "") +
               (!addr.empty() ? addr.toString() : "(no addr)") + " " +
               (!relayedAddr.empty() ? relayedAddr.toString() : "(not relayed)") + " " +
               (!topic.empty() ? topic : "(no topic)") + " " +
               "(" + std::to_string(payload.length()) + " B payload)";
    }

    bool LocalMsg::operator==(const LocalMsg &other) const
    {
        return type == other.type &&
               addr == other.addr &&
               relayedAddr == other.relayedAddr &&
               topic == other.topic &&
               payload == other.payload;
    }

    bool LocalMsg::operator!=(const LocalMsg &other) const
    {
        return !this->operator==(other);
    }
} // namespace kvik
