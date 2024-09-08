/**
 * @file pub_sub_struct.hpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Publication/subscription structs and callbacks
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <functional>
#include <string>

namespace kvik
{
    /**
     * @brief Subscription data structure
     *
     * Contains topic, payload and in the future maybe more details of
     * received data for subscription.
     */
    struct SubData
    {
        std::string topic = "";   //!< Topic of message
        std::string payload = ""; //!< Payload of message

        bool operator==(const SubData &other) const;
        bool operator!=(const SubData &other) const;

        /**
         * @brief Converts `SubData` to printable string
         *
         * Primarily for logging purposes
         *
         * @return String representation of contained data
         */
        std::string toString() const;
    };

    /**
     * @brief Publication data structure
     *
     * Contains topic, payload and in the future maybe more settings.
     */
    struct PubData
    {
        std::string topic = "";   //!< Topic of message
        std::string payload = ""; //!< Payload of message

        bool operator==(const PubData &other) const;
        bool operator!=(const PubData &other) const;

        /**
         * @brief Converts `PubData` to printable string
         *
         * Primarily for logging purposes
         *
         * @return String representation of contained data
         */
        std::string toString() const;

        /**
         * @brief Converts `PubData` to `SubData`
         *
         * Useful when publication is immediatelly sent back as subscription.
         *
         * @return Subscription data
         */
        SubData toSubData() const;
    };

    /**
     * @brief Subscribe callback type
     */
    using SubCb = std::function<void(const SubData &data)>;

    /**
     * @brief Subscription request
     *
     * Used in node's `subscribe*()` calls.
     */
    struct SubReq
    {
        std::string topic = "";
        SubCb cb = nullptr;

        bool operator==(const SubReq &other) const;
        bool operator!=(const SubReq &other) const;
    };
} // namespace kvik

// Define hasher function
template <>
struct std::hash<kvik::PubData>
{
    std::size_t operator()(kvik::PubData const &data) const noexcept
    {
        return std::hash<std::string>{}(data.topic) +
               std::hash<std::string>{}(data.payload);
    }
};

// Define hasher function
template <>
struct std::hash<kvik::SubData>
{
    std::size_t operator()(kvik::SubData const &data) const noexcept
    {
        return std::hash<std::string>{}(data.topic) +
               std::hash<std::string>{}(data.payload);
    }
};

// Define hasher function
template <>
struct std::hash<kvik::SubReq>
{
    std::size_t operator()(kvik::SubReq const &sub) const noexcept
    {
        return std::hash<std::string>{}(sub.topic);
    }
};
