/**
 * @file node_config.hpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Generic node config
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <chrono>
#include <cstdint>
#include <string>

namespace kvik
{
    /**
     * @brief Generic configuration for any node type
     */
    struct NodeConfig
    {
        struct LocalDelivery
        {
            /**
             * @brief Generic message response timeout
             *
             * Applies to `PROBE_RES`, `OK` and `FAIL` responses.
             */
            std::chrono::milliseconds respTimeout = std::chrono::milliseconds(500);
        };

        struct MsgIdCache
        {
            /**
             * @brief Interval of checking expiration times of message ID
             * cache entries
             *
             * Must be low enough to keep cache size low.
             * Must be high enough that standard time drifts (+ transmission
             * delays) don't cause false positive duplicates (see
             * `maxAge`).
             *
             * This is also used as unit for replay protection timestamps
             * inside messages. For this reason it has to be the SAME VALUE
             * FOR ALL COMMUNICATING NODES!
             */
            std::chrono::milliseconds timeUnit = std::chrono::milliseconds(500);

            /**
             * @brief Max age of cache entries as multiply of `timeUnit`
             *
             * Each entry in message ID cache has lifetime between
             * `(maxAge) * timeUnit` and `(maxAge + 1) * timeUnit`.
             * Product `(maxAge - 1) * timeUnit` represents  maximum accepted
             * time drift for a message.
             *
             * Thus, by default, message ID cache entry has lifetime of between
             * 1.5 and 2 seconds, and maximum time drift of received message is
             * 1 second.
             */
            uint8_t maxAge = 3;
        };

        struct Reporting
        {
            std::string baseTopic = "_report"; //!< Base topic for reporting purposes
            std::string rssiSubtopic = "rssi"; //!< Subtopic for RSSI reporting
        };

        struct TopicSeparators
        {
            std::string levelSeparator = "/";      //!< Separator used between topic parts
            std::string singleLevelWildcard = "+"; //!< Token used as single level wildcard
            std::string multiLevelWildcard = "#";  //!< Token used as multi level wildcard
        };

        LocalDelivery localDelivery;
        MsgIdCache msgIdCache;
        Reporting reporting;
        TopicSeparators topicSep;
    };
} // namespace kvik
