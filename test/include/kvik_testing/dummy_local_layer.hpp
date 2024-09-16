/**
 * @file dummy_local_layer.hpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Dummy local layer for testing purposes
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <chrono>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "kvik/layers.hpp"
#include "kvik_testing/local_msg_prep.hpp"

namespace kvik
{
    /**
     * @brief Dummy local layer
     *
     * Just logs all actions to local variables.
     */
    class DummyLocalLayer : public ILocalLayer
    {
    protected:
        std::mutex _mutex;

    public:
        using SentLog = std::vector<LocalMsg>;
        using ChannelsLog = std::vector<uint16_t>;
        using RespSuccLog = std::vector<bool>;

        ErrCode sendRet = ErrCode::SUCCESS;       //!< Return code of `send`
        ErrCode setChannelRet = ErrCode::SUCCESS; //!< Return code of `setChannel`
        Channels channels;                        //!< List of channels returned by `getChannels`
        std::queue<LocalMsg> responses;           //!< Responses for received messages

        //! Delay before sending back response
        std::chrono::milliseconds respDelay = std::chrono::milliseconds(0);

        //! Time difference of response messages
        std::chrono::milliseconds respTsDiff = std::chrono::milliseconds(0);

        //! Time unit of response messages
        std::chrono::milliseconds respTimeUnit = std::chrono::seconds(1);

        SentLog sentLog;         //!< All sent messages
        ChannelsLog channelsLog; //!< All set channels

        //! All return codes for `responses` (true for success, false for error)
        RespSuccLog respSuccLog;

        ErrCode send(const LocalMsg &msg)
        {
            const std::scoped_lock lock{_mutex};
            sentLog.push_back(msg);

            if (!responses.empty()) {
                auto &respMsg = responses.front();
                respMsg.reqId = msg.id;

                std::thread respThread(&DummyLocalLayer::simulateResponse,
                                       this, respMsg);
                respThread.detach();
                responses.pop();
            }

            return sendRet;
        }

        const Channels &getChannels()
        {
            const std::scoped_lock lock{_mutex};
            return channels;
        }

        ErrCode setChannel(uint16_t ch)
        {
            const std::scoped_lock lock{_mutex};
            channelsLog.push_back(ch);
            return setChannelRet;
        }

        /**
         * @brief Simulates message reception
         *
         * Calls local layer's receive callback, if has been set.
         *
         * @param msg Received message
         * @return Error code returned by callback, SUCCESS if none set
         */
        ErrCode recv(const LocalMsg &msg)
        {
            if (m_recvCb != nullptr) {
                return m_recvCb(msg);
            }
            return ErrCode::SUCCESS;
        }

        /**
         * @brief Simulates response
         *
         * Used in `send()` method.
         *
         * @param respMsg Response to send
         */
        void simulateResponse(LocalMsg respMsg)
        {
            std::this_thread::sleep_for(respDelay);

            if (m_recvCb != nullptr) {
                prepLocalMsg(respMsg, respTsDiff, respTimeUnit);
                ErrCode err = m_recvCb(respMsg);

                {
                    const std::scoped_lock lock{_mutex};
                    respSuccLog.push_back(err == ErrCode::SUCCESS);
                }
            }
        }

        /**
         * @brief Checks whether receive callback is set
         * @retval true Callback set
         * @retval false Callback not set
         */
        bool recvCbSet() { return m_recvCb != nullptr; }
    };
} // namespace kvik
