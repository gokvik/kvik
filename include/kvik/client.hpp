/**
 * @file client.hpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Client node type of Kvik
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <chrono>
#include <cstdint>
#include <future>
#include <unordered_map>

#include "kvik/client_config.hpp"
#include "kvik/errors.hpp"
#include "kvik/layers.hpp"
#include "kvik/local_peer.hpp"
#include "kvik/node.hpp"
#include "kvik/pub_sub_struct.hpp"
#include "kvik/timer.hpp"
#include "kvik/wildcard_trie.hpp"

namespace kvik
{
    /**
     * @brief Client retained data
     *
     * Used on IoT devices to retain state during deep sleep
     * and restore it after wake up.
     * Due to this, no dynamically allocated structures can be used.
     *
     * Gateway address has fixed maximum length (see `RetainedLocalPeer`
     * for details).
     */
    struct ClientRetainedData
    {
        RetainedLocalPeer gw;
        uint16_t msgsFailCnt;
        uint16_t timeSyncNoRespCnt;
    };

    /**
     * @brief Client node
     *
     * All public methods are multithread safe.
     */
    class Client : public INode
    {
    private:
        using LocalMsgVector = std::vector<LocalMsg>;

        /**
         * @brief Structure for sent messages pending for response
         */
        struct PendingMsg
        {
            const LocalMsg &req;            //!< Request (for validation of response)
            bool broadcast = false;         //!< Whether this message is broadcast
            std::promise<void> respPromise; //!< Response promise
            LocalMsgVector resps;           //!< Responses
        };

        std::mutex m_mutex;          //!< Mutex to prevent race conditions
        std::mutex m_dscvSyncMutex;  //!< Mutex for GW discovery/time sync
        ClientConfig m_conf;         //!< Configuration
        ILocalLayer *m_ll;           //!< Local layer
        WildcardTrie<SubCb> m_subDB; //!< Subscription database
        Timer m_subDBTimer;          //!< Sub DB timer
        Timer m_timeSyncTimer;       //!< Time synchronization timer
        LocalPeer m_gw;              //!< Gateway

        //! Messages pending for responses
        std::unordered_map<uint16_t, PendingMsg> m_pendingMsgs;

        //! Counter of recently failed messages (for rediscovery)
        uint16_t m_msgsFailCnt = 0;

        //! Counter of recently failed time sync attempts
        uint16_t m_timeSyncNoRespCnt = 0;

        /**
         * @brief Ignore invalid message timestamp
         *
         * Enabled temporarily during gateway discovery and initial time
         * synchronization.
         */
        bool m_ignoreInvalidMsgTs = false;

        //! Gateway discovery loop run flag
        bool m_dscvLoopRun = true;

        //! Gateway discovery loop conditional variable
        std::condition_variable m_dscvLoopCv;

        //! Gateway watchdog conditional variable
        std::condition_variable m_gwWdCv;

        //! Gateway watchdog thread
        std::thread m_gwWdThread;

    public:
        /**
         * @brief Constructs a new client node
         *
         * Performs gateway discovery.
         * If gateway address is passed in `retainedData`, it's address
         * isn't empty and responds to probe requests, only time
         * synchronization is performed.
         *
         * @param conf Configuration
         * @param ll Local layer (must be valid during whole `Client`'s
         * lifetime)
         * @param retainedData Retained data
         * @throw kvik::Exception Invalid parameters
         * @throw kvik::Exception Can't sync time or discover any gateway
         */
        Client(ClientConfig conf, ILocalLayer *ll,
               ClientRetainedData retainedData = {});

        /**
         * @brief Destroys the client node
         */
        ~Client();

        /**
         * @brief Publishes data, subscribes to and unsubscribes from topics in
         * bulk
         *
         * See `publish()`, `publishBulk()`, `subscribe()`, `subscribeBulk()`,
         * `unsubscribe()` and `unsubscribeBulk()` for simpler usage.
         *
         * @param pubs Vector of data to publish
         * @param subs Vector of subscription requests
         * @param subs Vector of unsubscription requests
         * @retval INVALID_SIZE Supplied data is too big for processing
         * @retval TIMEOUT Timeout while waiting for response
         * @retval MSG_PROCESSING_FAILED Gateway processing failed
         * @retval SUCCESS Successful action
         */
        ErrCode pubSubUnsubBulk(const std::vector<PubData> &pubs,
                                const std::vector<SubReq> &subs,
                                const std::vector<std::string> &unsubs);

        /**
         * @brief Unsubscribes from all topics
         * @retval INVALID_SIZE Supplied data is too big for processing
         * @retval TIMEOUT Timeout while waiting for response
         * @retval MSG_PROCESSING_FAILED Gateway processing failed
         * @retval SUCCESS Successful action
         */
        ErrCode unsubscribeAll();

        /**
         * @brief Resubscribes to all topics
         * @retval INVALID_SIZE Supplied data is too big for processing
         * @retval TIMEOUT Timeout while waiting for response
         * @retval MSG_PROCESSING_FAILED Gateway processing failed
         * @retval SUCCESS Successful action
         */
        ErrCode resubscribeAll();

        /**
         * @brief Tries to discover gateway and syncs time with chosen one
         *
         * Selection of best gateway is based on preference value calculated
         * by local layer protocol.
         *
         * @param maxAttempts Maximum number of attempts (value 0 means
         * infinity)
         * @retval TOO_MANY_FAILED_ATTEMPTS Discovery in specified number of
         * attempts failed
         * @retval SUCCESS Successful discovery
         */
        ErrCode discoverGateway(size_t maxAttempts = 1);

        /**
         * @brief Synchronizes time with gateway
         *
         * Also schedules upcoming background time sync to now +
         * `ClientConf::timeSync::reprobeGatewayInterval` (thus postponing
         * upcoming execution if called by user out of schedule).
         *
         * @retval TIMEOUT Timeout while waiting for probe response
         * @retval NO_GATEWAY Too many failed syncs with current gateway
         * @retval MSG_PROCESSING_FAILED Gateway processing failed
         * @retval SUCCESS Successful sync
         */
        ErrCode syncTime();

        /**
         * @brief Dumps itself in form of data to retain
         *
         * Next construction can be sped up by passing returned data structure
         * to the constructor.
         * Useful for IoT devices periodically waking up from deep sleep.
         *
         * @return Data to retain
         */
        const ClientRetainedData retain();

    protected:
        /**
         * @brief Sends local message and waits for the response
         *
         * Also handles gateway rediscovery in case of too many lost messages.
         *
         * @param msg Message to send (prepared in-place)
         * @param respMsg Response message (modified in-place)
         * @retval TIMEOUT Timeout while waiting for the response
         * @retval NO_GATEWAY No gateway
         * @retval MSG_PROCESSING_FAILED Gateway processing failed
         * @retval SUCCESS Successfully delivered
         */
        ErrCode sendLocal(LocalMsg &msg, LocalMsg &respMsg);

        /**
         * @brief Sends local message and waits for the response
         *
         * Unchecked version which does only:
         * - message preparation
         * - send
         * - waiting for the response (without FAIL handling)
         *
         * @param msg Message to send (prepared in-place)
         * @param respMsg Response message (modified in-place)
         * @param noResp Send without waiting for response
         * @retval TIMEOUT Timeout while waiting for the response
         * @retval NO_GATEWAY No gateway
         * @retval SUCCESS Successfully delivered
         */
        ErrCode sendLocalUnchecked(LocalMsg &msg, LocalMsg &respMsg,
                                   bool noResp = false);

        /**
         * @brief Sends local broadcast message and waits for any responses
         *
         * Unchecked version which does only:
         * - message preparation
         * - send
         * - waiting for the response (without FAIL handling)
         *
         * @param msg Message to send (prepared in-place)
         * @param respMsgs Response messages (modified in-place)
         * @retval SUCCESS Always
         */
        ErrCode sendLocalUncheckedBroadcast(LocalMsg &msg,
                                            LocalMsgVector &respMsgs);

        /**
         * @brief Sets all common message fields for transmission
         *
         * Handles message IDs, setting node type, gateway address,
         * timestamps,...
         * Not multithread safe.
         *
         * @param msg Message (modified in-place)
         * @param broadcast Whether to broadcast the message
         */
        void prepareMsg(LocalMsg &msg, bool broadcast);

        /**
         * @brief Tries to discover gateway on single channel
         *
         * Helper for `processGatewayDiscovery` member function.
         *
         * @param msg Broadcasted message
         * @param bestGw Best gateway storage
         * @param channel Current channel number
         */
        void processGatewayDiscoveryResponses(LocalMsg &msg, LocalPeer &bestGw,
                                              uint16_t channel);

    private:
        /**
         * @brief Subscription DB timer tick callback
         *
         * Checks subscription database lifetimes.
         * If any item expires, renews it.
         */
        void subDBTick();

        /**
         * @brief Gateway watchdog thread handler
         *
         * Waits for requests to rediscover gateway and then loops until a new
         * gateway is found.
         */
        void gwWatchdogHandler();

        /**
         * @brief Receives local message
         *
         * Used as callback for local layer.
         *
         * @param msg Message
         * @retval INVALID_ARG Invalid message/node type
         * @retval NOT_FOUND Corresponding request doesn't exist
         * @retval MSG_DUP_ID Duplicate message ID
         * @retval MSG_INVALID_TS Invalid timestamp
         * @retval MSG_UNKNOWN_SENDER Unknown sender
         * @retval SUCCESS Successfully processed
         */
        ErrCode recvLocal(LocalMsg msg);

        /**
         * @brief Receives local response
         *
         * @param msg Received response
         * @retval INVALID_ARG Invalid response type
         * @retval NOT_FOUND Corresponding request doesn't exist
         * @retval MSG_DUP_ID Duplicate message ID
         * @retval MSG_INVALID_TS Invalid timestamp
         * @retval MSG_UNKNOWN_SENDER Unknown sender
         * @retval SUCCESS Successfully processed
         */
        ErrCode recvLocalResp(const LocalMsg &msg);

        /**
         * @brief Receives local subscription data
         *
         * @param msg Received response
         * @retval MSG_DUP_ID Duplicate message ID
         * @retval MSG_INVALID_TS Invalid timestamp
         * @retval MSG_UNKNOWN_SENDER Unknown sender
         * @retval SUCCESS Successfully processed
         */
        ErrCode recvLocalSubData(const LocalMsg &msg);
    };
} // namespace kvik
