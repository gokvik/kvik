/**
 * @file remote_msg.hpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Remote message classes
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <string>

namespace kvik
{
    /**
     * @brief Remote message representation
     *
     * Used primarily for communication between `RemoteLayer` and `Node` classes.
     */
    struct RemoteMsg
    {
        std::string topic = "";   //!< Topic of message
        std::string payload = ""; //!< Payload of message

        bool operator==(const RemoteMsg &other) const;
        bool operator!=(const RemoteMsg &other) const;

        /**
         * @brief Converts `RemoteMsg` to printable string
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
struct std::hash<kvik::RemoteMsg>
{
    std::size_t operator()(kvik::RemoteMsg const &msg) const noexcept
    {
        return std::hash<std::string>{}(msg.topic) +
               std::hash<std::string>{}(msg.payload);
    }
};
