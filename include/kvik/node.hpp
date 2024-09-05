/**
 * @file node.hpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Node interface for Kvik
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <functional>

#include "kvik/errors.hpp"
#include "kvik/pub_sub_struct.hpp"

namespace kvik
{
    /**
     * @brief Generic configuration for any node type
     */
    struct GenericNodeConfig
    {
        struct Reporting
        {
            std::string baseTopic = "_report";                  //!< Base topic for reporting purposes
            std::string rssiSubtopic = "rssi";                  //!< Subtopic for RSSI reporting
            std::string probePayloadSubtopic = "probe_payload"; //!< Subtopic for probe payload reporting
        };

        struct TopicSeparators
        {
            std::string levelSeparator = "/";      //!< Separator used between topic parts
            std::string singleLevelWildcard = "+"; //!< Token used as single level wildcard
            std::string multiLevelWildcard = "#";  //!< Token used as multi level wildcard
        };

        Reporting reporting;
        TopicSeparators topicSep;
    };

    /**
     * @brief Interface for generic node type of Kvik
     */
    class INode
    {
    public:
        /**
         * @brief Constructs a new generic node
         */
        INode();

        /**
         * @brief Publishes payload to topic
         *
         * This is primary endpoint for publishing data locally on all node types.
         *
         * @param topic Topic
         * @param payload Payload
         * @return Error code
         */
        inline ErrCode publish(const std::string &topic,
                               const std::string &payload)
        {
            return INode::publish({
                .topic = topic,
                .payload = payload,
            });
        }

        /**
         * @brief Publishes payload to topic
         *
         * More generic version of `publish(topic, payload)`.
         *
         * @param data Data to publish
         * @return Error code
         */
        virtual ErrCode publish(const PubData &data) = 0;

        /**
         * @brief Subscribes to topic
         *
         * This is primary endpoint for subscribing locally on all node types.
         *
         * @param topic Topic
         * @param cb Callback function
         * @return Error code
         */
        virtual ErrCode subscribe(const std::string &topic, SubCb cb) = 0;

        /**
         * @brief Unsubscribes from topic
         *
         * This is primary endpoint for unsubscribing locally on all node types.
         *
         * @param topic Topic
         * @return Error code
         */
        virtual ErrCode unsubscribe(const std::string &topic) = 0;

        /**
         * @brief Resubscribes to all topics
         * @return Error code
         */
        virtual ErrCode resubscribeAll() = 0;
    };
} // namespace kvik
