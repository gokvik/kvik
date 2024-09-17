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
    const char *localMsgTypeToStr(LocalMsgType mt) noexcept
    {
        switch (mt) {
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
        case LocalMsgType::PUB_SUB_UNSUB:
            return "PUB_SUB_UNSUB";
        case LocalMsgType::SUB_DATA:
            return "SUB_DATA";
        default:
            return "???";
        }
    }

    const char *localMsgFailReasonToStr(LocalMsgFailReason fr) noexcept
    {
        switch (fr) {
        case LocalMsgFailReason::NONE:
            return "NONE";
        case LocalMsgFailReason::DUP_ID:
            return "DUP_ID";
        case LocalMsgFailReason::INVALID_TS:
            return "INVALID_TS";
        case LocalMsgFailReason::PROCESSING_FAILED:
            return "PROCESSING_FAILED";
        case LocalMsgFailReason::UNKNOWN_SENDER:
            return "UNKNOWN_SENDER";
        default:
            return "???";
        }
    }

    bool LocalMsg::operator==(const LocalMsg &other) const
    {
        return type == other.type &&
               addr == other.addr &&
               relayedAddr == other.relayedAddr &&
               pubs == other.pubs &&
               subs == other.subs &&
               unsubs == other.unsubs &&
               subsData == other.subsData;
    }

    bool LocalMsg::operator!=(const LocalMsg &other) const
    {
        return !this->operator==(other);
    }

    std::string LocalMsg::toString() const
    {
        std::string base = std::string{localMsgTypeToStr(type)} + " " +
                           (!addr.empty() ? addr.toString() : "(no addr)") +
                           (!relayedAddr.empty() ? " " + relayedAddr.toString() : "");

        switch (type) {
        case LocalMsgType::FAIL:
            return base + " | failed due to " +
                   localMsgFailReasonToStr(failReason);
        case LocalMsgType::PROBE_RES:
            return base + " | pref " + std::to_string(pref);
        case LocalMsgType::PUB_SUB_UNSUB:
            base += " | ";
            for (const auto &p : pubs) {
                base += "PUB " + p.toString() + ", ";
            }
            for (const auto &s : subs) {
                base += "SUB " + s + ", ";
            }
            for (const auto &u : unsubs) {
                base += "UNSUB " + u + ", ";
            }

            // Remove last ", "
            base.erase(base.size() - 2);

            return base;
        case LocalMsgType::SUB_DATA:
            base += " | ";
            for (const auto &d : subsData) {
                base += d.toString() + ", ";
            }

            // Remove last ", "
            base.erase(base.size() - 2);

            return base;
        default:
            return base;
        }
    }
} // namespace kvik
