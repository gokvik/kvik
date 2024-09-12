/**
 * @file local_peer.cpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Local layer peer info
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <algorithm>

#include "kvik/local_peer.hpp"

namespace kvik
{
    std::string LocalPeer::toString() const
    {
        return addr.toString() +
               (channel != 0 ? " (channel " + std::to_string(channel) + ")" : "") +
               (pref != 0 ? " (pref " + std::to_string(pref) + ")" : "");
    }

    const RetainedLocalPeer LocalPeer::retain() const
    {
        RetainedLocalPeer rlp;

        // Copy address (at most 32 bytes of address)
        uint8_t sizeToCopy = std::min(addr.addr.size(), rlp.addr.max_size());
        std::copy_n(addr.addr.begin(), sizeToCopy, rlp.addr.begin());

        rlp.addrLen = sizeToCopy;
        rlp.channel = channel;
        return rlp;
    }

    const LocalPeer RetainedLocalPeer::unretain() const
    {
        return {
            .addr = {.addr = {
                         addr.begin(),
                         std::next(addr.begin(), addrLen)}},
            .channel = channel,
        };
    }
}
