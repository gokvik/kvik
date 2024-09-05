/**
 * @file local_broker.hpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Local broker remote layer for Kvik
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <mutex>
#include <string>

#include "kvik/errors.hpp"
#include "kvik/layers.hpp"
#include "kvik/wildcard_trie.hpp"

namespace kvik
{
    /**
     * @brief Local broker remote layer
     *
     * Acts as local MQTT server.
     */
    class LocalBroker : public IRemoteLayer
    {
        std::mutex m_mutex;
        kvik::WildcardTrie<bool> m_subs;  //!< Subscriptions
        std::string m_topicPrefix;        //!< Topic prefix for publishing

    public:
        /**
         * @brief Constructs a new local broker object
         */
        LocalBroker();

        /**
         * @brief Destroys local broker layer object
         */
        ~LocalBroker();

        /**
         * @brief Publishes message coming from node
         *
         * Should be used by `INode` only!
         *
         * If subscription for message's topic exists, immediately calls
         * receive callback (from current thread).
         *
         * @param msg Message to publish
         * @return ErrCode::SUCCESS If no receive callback called.
         * @return Any error code returned by receive callback.
         */
        ErrCode publish(const RemoteMsg &msg);

        /**
         * @brief Subscribes to given topic
         *
         * Should be used by `INode` only!
         *
         * @param topic Topic
         * @return ErrCode::SUCCESS Always
         */
        ErrCode subscribe(const std::string &topic);

        /**
         * @brief Unsubscribes from given topic
         *
         * Should be used by `INode` only!
         *
         * @param topic Topic
         * @return ErrCode::SUCCESS Entry removed
         * @return ErrCode::NOT_FOUND Entry doesn't exist
         */
        ErrCode unsubscribe(const std::string &topic);
    };
} // namespace kvik
