/**
 * @file layers.hpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Local and remote layers for Kvik
 *
 * @copyright Copyright (c) 2023
 *
 */

#pragma once

#include <functional>

#include "kvik/errors.hpp"
#include "kvik/local_msg.hpp"
#include "kvik/remote_msg.hpp"

namespace kvik
{
    /**
     * @brief Interface for local layer
     */
    class ILocalLayer
    {
    public:
        using RecvCb = std::function<ErrCode(const LocalMsg &)>;

    protected:
        RecvCb m_recvCb = nullptr;

    public:
        /**
         * @brief Sends the message to given node
         *
         * Should be used by `INode` only!
         *
         * In the message, empty address means send to the bridge peer.
         *
         * @param msg Message
         * @return Error code
         */
        virtual ErrCode send(const LocalMsg &msg) = 0;

        /**
         * @brief Sets receive callback
         * @param cb Callback
         */
        void setRecvCb(RecvCb cb)
        {
            m_recvCb = cb;
        }
    };

    /**
     * @brief Interface for local layer that connects to bridge
     */
    class IClientLocalLayer : virtual public ILocalLayer
    {
    };

    /**
     * @brief Interface for local layer that serves clients
     */
    class IServingLocalLayer : virtual public ILocalLayer
    {
    };

    /**
     * @brief Interface for remote layer
     *
     */
    class IRemoteLayer
    {
    public:
        using RecvCb = std::function<ErrCode(const RemoteMsg &)>;
        using ReconnectCb = std::function<ErrCode()>;

    protected:
        RecvCb m_recvCb = nullptr;
        ReconnectCb m_reconnectCb = nullptr;

    public:
        /**
         * @brief Publishes message coming from node
         *
         * Should be used by `INode` only!
         *
         * @param msg Message to publish
         * @return Error code
         */
        virtual ErrCode publish(const RemoteMsg &msg) = 0;

        /**
         * @brief Subscribes to given topic
         *
         * Should be used by `INode` only!
         *
         * @param topic Topic
         * @return Error code
         */
        virtual ErrCode subscribe(const std::string &topic) = 0;

        /**
         * @brief Unsubscribes from given topic
         *
         * Should be used by `INode` only!
         *
         * @param topic Topic
         * @return Error code
         */
        virtual ErrCode unsubscribe(const std::string &topic) = 0;

        /**
         * @brief Sets receive callback
         * @param cb Callback
         */
        void setRecvCb(RecvCb cb)
        {
            m_recvCb = cb;
        }

        /**
         * @brief Sets reconnect callback
         * @param cb Callback
         */
        void setReconnectCb(ReconnectCb cb)
        {
            m_reconnectCb = cb;
        }
    };
} // namespace kvik
