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
#include "kvik/pub_sub_struct.hpp"

namespace kvik
{
    /**
     * @brief Interface for local layer
     */
    class ILocalLayer
    {
    public:
        using RecvCb = std::function<ErrCode(LocalMsg)>;
        using Channels = std::vector<uint16_t>;

    protected:
        RecvCb m_recvCb = nullptr;

    public:
        /**
         * @brief Sends the message to given node
         *
         * Should be used by `INode` only!
         *
         * @param msg Message
         * @retval INVALID_SIZE Supplied data is too big for processing
         * @retval SUCCESS Successfully sent
         * @retval * Any other protocol-specific code
         */
        virtual ErrCode send(const LocalMsg &msg) = 0;

        /**
         * @brief Gives list of possible channels
         *
         * Used on client and relay nodes during gateway discovery.
         *
         * @return Vector of channels (can be empty, in which case
         * no `setChannel` call will be made unless retained client data
         * contain some garbage)
         */
        virtual const Channels &getChannels() = 0;

        /**
         * @brief Sets channel
         *
         * Channel 0 is treated as default one.
         *
         * @param ch Channel number
         * @retval NOT_SUPPORTED Channel change not supported
         *         (use only if `getChannels` returns empty vector)
         * @retval INVALID_ARG Invalid channel number
         * @retval SUCCESS Channel successfully changed
         */
        virtual ErrCode setChannel(uint16_t ch) = 0;

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
     * @brief Interface for remote layer
     *
     */
    class IRemoteLayer
    {
    public:
        using RecvCb = std::function<ErrCode(const SubData &)>;
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
         * @param data Message to publish
         * @return Error code
         */
        virtual ErrCode publish(const PubData &data) = 0;

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
