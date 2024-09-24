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
#include <vector>

#include "kvik/errors.hpp"
#include "kvik/local_addr.hpp"
#include "kvik/local_msg_id_cache.hpp"
#include "kvik/node_config.hpp"
#include "kvik/pub_sub_struct.hpp"

namespace kvik
{
    /**
     * @brief Interface for generic node type of Kvik
     */
    class INode
    {
        NodeConfig m_nodeConf;
        uint16_t m_msgId;
        LocalMsgIdCache m_msgIdCache;

    public:
        /**
         * @brief Constructs a new generic node
         * @param conf Generic configuration
         * @throw kvik::Exception Invalid configuration
         */
        INode(NodeConfig conf);

        /**
         * @brief Destroys the generic node
         */
        virtual ~INode();

        /**
         * @brief Publishes payload to topic
         *
         * This is primary endpoint for publishing data locally on all node types.
         *
         * @param topic Topic
         * @param payload Payload
         * @return Error code (node-specific)
         */
        ErrCode publish(const std::string &topic,
                        const std::string &payload)
        {
            return this->publish({
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
         * @return Error code (node-specific)
         */
        ErrCode publish(const PubData &data)
        {
            return this->publishBulk({data});
        }

        /**
         * @brief Publishes data in bulk
         * @param pubs Vector of data to publish
         * @return Error code (node-specific)
         */
        ErrCode publishBulk(const std::vector<PubData> &pubs)
        {
            return this->pubSubUnsubBulk(pubs, {}, {});
        }

        /**
         * @brief Subscribes to topic
         *
         * This is primary endpoint for subscribing locally on all node types.
         *
         * @param topic Topic
         * @param cb Callback function
         * @return Error code (node-specific)
         */
        ErrCode subscribe(const std::string &topic, SubCb cb)
        {
            return this->subscribeBulk({{topic, cb}});
        }

        /**
         * @brief Subscribes to topics in bulk
         * @param subs Vector of subscription requests
         * @return Error code (node-specific)
         */
        ErrCode subscribeBulk(const std::vector<SubReq> &subs)
        {
            return this->pubSubUnsubBulk({}, subs, {});
        }

        /**
         * @brief Unsubscribes from topic
         *
         * This is primary endpoint for unsubscribing locally on all node types.
         *
         * @param topic Topic
         * @return Error code (node-specific)
         */
        ErrCode unsubscribe(const std::string &topic)
        {
            return this->unsubscribeBulk({topic});
        }

        /**
         * @brief Unsubscribes from topics in bulk
         * @param topics Topics
         * @return Error code (node-specific)
         */
        ErrCode unsubscribeBulk(const std::vector<std::string> &topics)
        {
            return this->pubSubUnsubBulk({}, {}, topics);
        }

        /**
         * @brief Publishes data, subscribes to and unsubscribes from topics in bulk
         *
         * Advantage of bulk publish/subscribe/unsubscribe is all data are put
         * together possibly into single big packet. In case of transmission
         * over radio, this saves a lot of time compared to multiple smaller
         * chunks.
         *
         * @param pubs Vector of data to publish
         * @param subs Vector of subscription requests
         * @param unsubs Vector of unsubscription requests
         * @return Error code (node-specific)
         */
        virtual ErrCode pubSubUnsubBulk(const std::vector<PubData> &pubs,
                                        const std::vector<SubReq> &subs,
                                        const std::vector<std::string> &unsubs) = 0;

        /**
         * @brief Unsubscribes from all topics
         * @return Error code (node-specific)
         */
        virtual ErrCode unsubscribeAll() = 0;

        /**
         * @brief Resubscribes to all topics
         * @return Error code (node-specific)
         */
        virtual ErrCode resubscribeAll() = 0;

    protected:
        /**
         * @brief Generates new message ID for a local message transmission
         *
         * Implemented as random initial value and incrementation on each call.
         *
         * Not multithread safe.
         *
         * @return Message ID
         */
        uint16_t getMsgId();

        /**
         * @brief Validates received message ID
         *
         * Not multithread safe.
         *
         * @param addr Source peer address
         * @param id Message ID
         * @retval true Message ID is valid (not duplicate)
         * @retval false Message ID is invalid (duplicate)
         */
        bool validateMsgId(const LocalAddr &addr, uint16_t id);

        /**
         * @brief Validates received message timestamp
         *
         * Not multithread safe.
         *
         * @param ts Timestamp
         * @param tsDiff Timestamp difference between nodes
         * @retval true Timestamp is valid
         * @retval false Timestamp is invalid
         */
        bool validateMsgTimestamp(
            uint16_t ts,
            std::chrono::milliseconds tsDiff = std::chrono::milliseconds(0));
        
        /**
         * @brief Builds RSSI report topic
         * @param addr Peer address
         * @return RSSI report topic
         */
        std::string buildReportRssiTopic(const LocalAddr &peer) const;
    };
} // namespace kvik
