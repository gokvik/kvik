/**
 * @file pub_sub_struct.cpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Publication/subscription structs and callbacks
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <string>

#include "kvik/pub_sub_struct.hpp"

namespace kvik
{
    bool PubData::operator==(const PubData &other) const
    {
        return topic == other.topic &&
               payload == other.payload;
    }

    bool PubData::operator!=(const PubData &other) const
    {
        return !this->operator==(other);
    }

    std::string PubData::toString() const
    {
        return (!topic.empty() ? topic : "(no topic)") + " " +
               "(" + std::to_string(payload.length()) + " B payload)";
    }

    SubData PubData::toSubData() const
    {
        return {
            .topic = topic,
            .payload = payload,
        };
    }

    bool SubData::operator==(const SubData &other) const
    {
        return topic == other.topic &&
               payload == other.payload;
    }

    bool SubData::operator!=(const SubData &other) const
    {
        return !this->operator==(other);
    }

    std::string SubData::toString() const
    {
        return (!topic.empty() ? topic : "(no topic)") + " " +
               "(" + std::to_string(payload.length()) + " B payload)";
    }
} // namespace kvik
