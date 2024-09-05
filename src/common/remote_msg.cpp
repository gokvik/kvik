/**
 * @file remote_msg.cpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Local message classes
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <string>

#include "kvik/remote_msg.hpp"

namespace kvik
{
    std::string RemoteMsg::toString() const
    {
        return (!topic.empty() ? topic : "(no topic)") + " " +
               "(" + std::to_string(payload.length()) + " B payload)";
    }

    bool RemoteMsg::operator==(const RemoteMsg &other) const
    {
        return topic == other.topic &&
               payload == other.payload;
    }

    bool RemoteMsg::operator!=(const RemoteMsg &other) const
    {
        return !this->operator==(other);
    }
} // namespace kvik
