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
#include "kvik/node_config.hpp"
#include "kvik/nonce_cache.hpp"
#include "kvik/pub_sub_struct.hpp"

namespace kvik
{
    /**
     * @brief Interface for generic node type of Kvik
     */
    class INode
    {
        const NodeConfig &m_nodeConf;
        uint16_t m_nonce;
        NonceCache m_nonceCache;

    public:
        /**
         * @brief Constructs a new generic node
         * @param conf Generic configuration
         */
        INode(const NodeConfig &conf);

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
            return this->pubSubBulk(pubs, {});
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
            return this->pubSubBulk({}, subs);
        }

        /**
         * @brief Publishes data and subscribes to topics in bulk
         *
         * Advantage of bulk publish/subscribe is all data are put together
         * possibly into single big packet. In case of transmission over radio,
         * this saves a lot of time compared to multiple smaller chunks.
         *
         * @param pubs Vector of data to publish
         * @param subs Vector of subscription requests
         * @return Error code (node-specific)
         */
        virtual ErrCode pubSubBulk(const std::vector<PubData> &pubs,
                                   const std::vector<SubReq> &subs) = 0;

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
        virtual ErrCode unsubscribeBulk(const std::vector<std::string> &topics) = 0;

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
         * @brief Generates new nonce for a local message transmission
         *
         * Implemented as random initial value and incrementation on each call.
         *
         * Not multithread safe.
         *
         * @return Nonce
         */
        uint16_t getNonce();

        /**
         * @brief Validates received nonce
         *
         * Not multithread safe.
         *
         * @param addr Source peer address
         * @param nonce Nonce
         * @return true Nonce is valid (not duplicate)
         * @return false Nonce is invalid (duplicate)
         */
        bool validateNonce(const LocalAddr &addr, uint16_t nonce);
    };
} // namespace kvik
