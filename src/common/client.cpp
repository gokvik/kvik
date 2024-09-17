/**
 * @file client.cpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Client node type of Kvik
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <cinttypes>
#include <sys/time.h> // Unix and ESP

#include "kvik/client.hpp"
#include "kvik/client_config.hpp"
#include "kvik/errors.hpp"
#include "kvik/layers.hpp"
#include "kvik/logger.hpp"
#include "kvik/node.hpp"
#include "kvik/pub_sub_struct.hpp"
#include "kvik/timer.hpp"
#include "kvik/wildcard_trie.hpp"

// Log tag
static const char *KVIK_LOG_TAG = "Kvik/Client";

namespace kvik
{
    Client::Client(ClientConfig conf, ILocalLayer *ll,
                   ClientRetainedData retainedData)
        : INode{conf.nodeConf}, m_conf{conf}, m_ll{ll},
          m_subDB{conf.nodeConf.topicSep.levelSeparator,
                  conf.nodeConf.topicSep.singleLevelWildcard,
                  conf.nodeConf.topicSep.multiLevelWildcard},
          m_subDBTimer{conf.subDB.subLifetime,
                       std::bind(&Client::subDBTick, this)},
          m_timeSyncTimer{conf.timeSync.reprobeGatewayInterval,
                          std::bind(&Client::syncTime, this)}
    {
        if (m_ll == nullptr) {
            KVIK_THROW_EXC("Invalid local layer parameter");
        }

        // Set receive callback
        m_ll->setRecvCb(
            std::bind(&Client::recvLocal, this, std::placeholders::_1));

        m_ignoreInvalidMsgTs = true;

        if (retainedData.gw.addrLen > 0) {
            // Restore retained data
            m_gw = retainedData.gw.unretain();
            m_msgsFailCnt = retainedData.msgsFailCnt;
            m_timeSyncNoRespCnt = retainedData.timeSyncNoRespCnt;

            KVIK_LOGD("Using retained data");

            // Restore gateway's channel
            bool channelOk = true;
            if (m_gw.channel > 0) {
                KVIK_LOGD("Setting local layer channel to %u", m_gw.channel);
                if (m_ll->setChannel(m_gw.channel) != ErrCode::SUCCESS) {
                    KVIK_LOGW("Failed to set channel");
                    channelOk = false;
                }
            }

            // Attempt time sync
            if (channelOk && this->syncTime() == ErrCode::SUCCESS) {
                KVIK_LOGI("Time sync successful, GW: %s",
                          m_gw.toString().c_str());
                goto initialized;
            }

            KVIK_LOGW("Time sync failed, doing gateway discovery");
        }

        if (this->discoverGateway(m_conf.gwDscv.initialDscvFailThres) ==
            ErrCode::SUCCESS) {
            KVIK_LOGI("Gateway discovery successful, new GW: %s",
                      m_gw.toString().c_str());
            goto initialized;
        }

        KVIK_THROW_EXC("Gateway discovery failed");

    initialized:
        KVIK_LOGI("Initialized");
        m_ignoreInvalidMsgTs = false;

        // Spawn gateway watchdog
        m_gwWdThread = std::thread(&Client::gwWatchdogHandler, this);
    }

    Client::~Client()
    {
        {
            const std::scoped_lock lock(m_mutex);
            m_dscvLoopRun = false;
        }

        // Wait for cancellation of currently running gateway discovery
        KVIK_LOGD("Waiting for gateway discovery thread...");
        m_dscvLoopCv.notify_one();
        m_gwWdCv.notify_one();
        m_gwWdThread.join();

        // Unset receive callback
        m_ll->setRecvCb(nullptr);

        // Wait for all actions
        const std::scoped_lock lock(m_mutex, m_dscvSyncMutex);

        KVIK_LOGI("Deinitialized");
    }

    ErrCode Client::pubSubUnsubBulk(const std::vector<PubData> &pubs,
                                    const std::vector<SubReq> &subs,
                                    const std::vector<std::string> &unsubs)
    {
        if (pubs.size() == 0 && subs.size() == 0 && unsubs.size() == 0) {
            // Nothing to do
            return ErrCode::SUCCESS;
        }

        LocalMsg msg;
        msg.type = LocalMsgType::PUB_SUB_UNSUB;

        // Preallocate memory
        msg.pubs.reserve(pubs.size());
        msg.subs.reserve(subs.size());
        msg.unsubs.reserve(unsubs.size());

        // Copy items
        msg.pubs.insert(msg.pubs.end(), pubs.begin(), pubs.end());
        for (const auto &sub : subs) {
            msg.subs.push_back(sub.topic);
        }
        msg.unsubs.insert(msg.unsubs.end(), unsubs.begin(), unsubs.end());

        // Send the message
        LocalMsg respMsg;
        KVIK_RETURN_ERROR(this->sendLocal(msg, respMsg));
        if (respMsg.type != LocalMsgType::OK) {
            // Defensive check (already handled by `sendLocal()`)
            KVIK_LOGW("Received non-OK response");
            return ErrCode::MSG_PROCESSING_FAILED;
        }

        // Modify local data
        {
            const std::scoped_lock lock(m_mutex);

            // Remove subscriptions from database
            for (const auto &topic : unsubs) {
                if (!m_subDB.remove(topic)) {
                    // Not subscribed to this topic
                    KVIK_LOGD(
                        "Can't unsubscribe from not-subscribed topic '%s'",
                        topic.c_str());
                }
            }

            // Insert subscriptions into database
            for (const auto &sub : subs) {
                m_subDB.insert(sub.topic, sub.cb);
            }
        }

        return ErrCode::SUCCESS;
    }

    ErrCode Client::unsubscribeAll()
    {
        LocalMsg msg;
        msg.type = LocalMsgType::PUB_SUB_UNSUB;

        // Populate data
        {
            const std::scoped_lock lock(m_mutex);
            m_subDB.forEach([&msg](const std::string &topic, const SubCb &) {
                msg.unsubs.push_back(topic);
            });
        }

        if (msg.unsubs.size() == 0) {
            // Nothing to do
            return ErrCode::SUCCESS;
        }

        // Send the message
        LocalMsg respMsg;
        KVIK_RETURN_ERROR(this->sendLocal(msg, respMsg));
        if (respMsg.type != LocalMsgType::OK) {
            // Defensive check (already handled by `sendLocal()`)
            KVIK_LOGW("Received non-OK response");
            return ErrCode::MSG_PROCESSING_FAILED;
        }

        // Modify local data
        {
            const std::scoped_lock lock(m_mutex);
            m_subDB.clear();
        }

        return ErrCode::SUCCESS;
    }

    ErrCode Client::resubscribeAll()
    {
        LocalMsg msg;
        msg.type = LocalMsgType::PUB_SUB_UNSUB;

        // Populate data
        {
            const std::scoped_lock lock(m_mutex);
            m_subDB.forEach([&msg](const std::string &topic, const SubCb &) {
                msg.subs.push_back(topic);
            });
        }

        if (msg.subs.size() == 0) {
            // Nothing to do
            return ErrCode::SUCCESS;
        }

        // Send the message
        LocalMsg respMsg;
        KVIK_RETURN_ERROR(this->sendLocal(msg, respMsg));
        if (respMsg.type != LocalMsgType::OK) {
            // Defensive check (already handled by `sendLocal()`)
            KVIK_LOGW("Received non-OK response");
            return ErrCode::MSG_PROCESSING_FAILED;
        }

        return ErrCode::SUCCESS;
    }

    ErrCode Client::discoverGateway(size_t maxAttempts)
    {
        size_t attemptsCnt = 0;
        auto delay = m_conf.gwDscv.dscvMinDelay;
        auto respTimeout = m_conf.nodeConf.localDelivery.respTimeout;

        LocalPeer bestGw;
        LocalMsg msg;
        msg.type = LocalMsgType::PROBE_REQ;

        const auto &channels = m_ll->getChannels();

        KVIK_LOGD("Started, max attempts %zu", maxAttempts);

        while (attemptsCnt < maxAttempts || maxAttempts == 0) {
            KVIK_LOGD("Attempt %zu started", attemptsCnt + 1);

            {
                const std::scoped_lock dscvSyncLock(m_dscvSyncMutex);

                // Modified only by `discoverGateway` and constructor
                // `m_dscvSyncMutex` lock is enough.
                m_ignoreInvalidMsgTs = true;

                if (channels.empty()) {
                    // No channels on local layer, don't set it
                    KVIK_LOGD("Probing default channel");
                    this->processGatewayDiscoveryResponses(msg, bestGw, 0);
                } else {
                    // Iterate all possible channels and discover all possible
                    // gateways
                    for (const auto &ch : channels) {
                        if (m_ll->setChannel(ch) != ErrCode::SUCCESS) {
                            KVIK_LOGW("Can't set channel %u, skipping it", ch);
                            continue;
                        }
                        KVIK_LOGD("Probing channel %u", ch);
                        this->processGatewayDiscoveryResponses(msg, bestGw, ch);
                    }
                }

                m_ignoreInvalidMsgTs = false;

                if (!bestGw.empty()) {
                    // Found new best gateway
                    const std::scoped_lock lock(m_mutex);
                    if (!channels.empty()) {
                        m_ll->setChannel(bestGw.channel);
                    }
                    m_gw = bestGw;
                    m_msgsFailCnt = 0;
                    m_timeSyncNoRespCnt = 0;
                    KVIK_LOGI("Using new gateway: %s", m_gw.toString().c_str());
                    KVIK_LOGD("Attempt %zu successful", attemptsCnt + 1);
                    return ErrCode::SUCCESS;
                } else {
                    // Reset gateway
                    const std::scoped_lock lock(m_mutex);
                    m_gw = {};
                }
            }

            KVIK_LOGD("Attempt %zu failed", attemptsCnt + 1);

            {
                // Sleep with possible destructor interrupt
                std::unique_lock lock{m_mutex};
                if (m_dscvLoopCv.wait_for(lock, delay,
                                          [this]() { return !m_dscvLoopRun; })) {
                    // Destructor has been called
                    KVIK_LOGD("Cancelled by destructor call");
                    return ErrCode::SUCCESS;
                }
            }

            // Increase delay
            delay *= 2;
            if (delay > m_conf.gwDscv.dscvMaxDelay) {
                delay = m_conf.gwDscv.dscvMaxDelay;
            }

            attemptsCnt++;
        }

        KVIK_LOGW("Gateway discovery failed after %zu attempts",
                  attemptsCnt);

        return ErrCode::TOO_MANY_FAILED_ATTEMPTS;
    }

    void Client::processGatewayDiscoveryResponses(LocalMsg &msg,
                                                  LocalPeer &bestGw,
                                                  uint16_t channel)
    {
        // Broadcast probe
        LocalMsgVector responses;
        this->sendLocalUncheckedBroadcast(msg, responses);

        for (const auto &resp : responses) {
            if (resp.pref > bestGw.pref) {
                // Found better gateway
                bestGw.addr = resp.addr;
                bestGw.channel = channel;
                bestGw.pref = resp.pref;
                bestGw.tsDiff = resp.tsDiff;
            }
        }
    }

    void Client::gwWatchdogHandler()
    {
        {
            const std::scoped_lock lock(m_mutex);
            if (!m_dscvLoopRun) {
                KVIK_LOGD("Cancelled early by destructor call");
                return;
            }
        }

        while (true) {
            {
                std::unique_lock lock{m_mutex};
                m_gwWdCv.wait(lock);
                if (!m_dscvLoopRun) {
                    KVIK_LOGD("Cancelled by destructor call");
                    return;
                }
            }

            this->discoverGateway(0);
        }
    }

    ErrCode Client::syncTime()
    {
        const std::scoped_lock dscvSyncLock(m_dscvSyncMutex);

        ErrCode err;
        LocalMsg msg;
        msg.type = LocalMsgType::PROBE_REQ;

        KVIK_LOGD("Started");

        // Behind `m_dscvSyncMutex`, no need for `m_mutex` lock
        m_timeSyncTimer.setNextExec(
            std::chrono::steady_clock::now() +
            m_conf.timeSync.reprobeGatewayInterval);

        // Probe on local layer and wait for response
        LocalMsg respMsg;
        err = this->sendLocal(msg, respMsg);
        if (err != ErrCode::SUCCESS) {
            KVIK_LOGW("Send failed");
            goto fail;
        }
        if (respMsg.type != LocalMsgType::PROBE_RES) {
            // Defensive check (already handled by `sendLocal()`)
            KVIK_LOGW("Received invalid response");
            err = ErrCode::MSG_PROCESSING_FAILED;
            goto fail;
        }

        // Set system time
        if (m_conf.timeSync.syncSystemTime) {
            auto nowTs = std::chrono::steady_clock::now().time_since_epoch() +
                         m_gw.tsDiff;
            auto nowTsMs = std::chrono::duration_cast<
                std::chrono::milliseconds>(nowTs);
            int64_t nowTsMsInt = nowTsMs.count();

            struct timeval tv = {
                .tv_sec = static_cast<time_t>(nowTsMsInt / 1000),
                .tv_usec = static_cast<suseconds_t>((nowTsMsInt % 1000) * 1000),
            };

            if (settimeofday(&tv, nullptr) != 0) {
                KVIK_LOGE("Set system time failed");
            } else {
                KVIK_LOGI("Set current timestamp: " PRId64, nowTsMsInt);
            }
        }

        // Store the result
        {
            const std::scoped_lock lock(m_mutex);
            m_gw.tsDiff = respMsg.tsDiff;
            m_timeSyncNoRespCnt = 0;
            KVIK_LOGD("Successful (tsDiff=%zu ms)", m_gw.tsDiff.count());
        }

        return ErrCode::SUCCESS;

    fail:
        bool trigDscv;
        {
            const std::scoped_lock lock(m_mutex);
            m_timeSyncNoRespCnt++;
            trigDscv = m_conf.gwDscv.trigTimeSyncNoRespCnt == 0 ||
                       m_timeSyncNoRespCnt >=
                           m_conf.gwDscv.trigTimeSyncNoRespCnt;
        }

        // Check if failed count isn't too high
        if (trigDscv) {
            KVIK_LOGW("Too many failed time syncs, "
                      "triggering background gateway discovery");
            m_gwWdCv.notify_one();
        }

        return err;
    }

    ErrCode Client::sendLocal(LocalMsg &msg, LocalMsg &respMsg)
    {
        ErrCode err;

        // Send
        err = this->sendLocalUnchecked(msg, respMsg);
        if (err != ErrCode::SUCCESS) {
            goto fail;
        }

        // Check if response isn't FAIL
        if (respMsg.type == LocalMsgType::FAIL) {
            KVIK_LOGW("Message delivery failed with code %s",
                      localMsgFailReasonToStr(respMsg.failReason));
            err = ErrCode::MSG_PROCESSING_FAILED;
            goto fail;
        }

        {
            // Reset failed messages counter
            const std::scoped_lock lock(m_mutex);
            m_msgsFailCnt = 0;
        }
        return ErrCode::SUCCESS;

    fail:
        bool trigDscv;
        {
            const std::scoped_lock lock(m_mutex);
            m_msgsFailCnt++;
            trigDscv = m_conf.gwDscv.trigMsgsFailCnt == 0 ||
                       m_msgsFailCnt >= m_conf.gwDscv.trigMsgsFailCnt;
        }

        // Check if failed count isn't too high
        if (trigDscv) {
            KVIK_LOGW("Too many failed messages, "
                      "triggering background gateway discovery");
            m_gwWdCv.notify_one();
        }

        return err;
    }

    ErrCode Client::sendLocalUnchecked(LocalMsg &msg, LocalMsg &respMsg,
                                       bool noResp)
    {
        // Prepare
        std::future<void> respFuture;
        std::vector<LocalMsg> *responsesPtr;
        {
            const std::scoped_lock lock(m_mutex);
            this->prepareMsg(msg, false);
            if (msg.addr.empty()) {
                return ErrCode::NO_GATEWAY;
            }
            m_pendingMsgs.insert({msg.id, PendingMsg{msg, false}});
            respFuture = m_pendingMsgs.at(msg.id).respPromise.get_future();
            responsesPtr = &m_pendingMsgs.at(msg.id).resps;
        }

        KVIK_LOGD("Message (id=%u): %s", msg.id, msg.toString().c_str());

        // Send
        KVIK_RETURN_ERROR(m_ll->send(msg));

        if (noResp) {
            KVIK_LOGD("Not waiting for response");
            return ErrCode::SUCCESS;
        }

        // Wait for response
        if (respFuture.wait_for(m_conf.nodeConf.localDelivery.respTimeout) ==
            std::future_status::timeout) {
            const std::scoped_lock lock(m_mutex);
            m_pendingMsgs.erase(msg.id);
            KVIK_LOGW("Response timeout (id=%u) for: %s", msg.id,
                      msg.toString().c_str());
            return ErrCode::TIMEOUT;
        }

        // Get response, remove response promise and return
        {
            const std::scoped_lock lock(m_mutex);
            respMsg = (*responsesPtr)[0];
            m_pendingMsgs.erase(msg.id);
            KVIK_LOGD("Response (id=%u): %s", msg.id,
                      respMsg.toString().c_str());
            return ErrCode::SUCCESS;
        }
    }

    ErrCode Client::sendLocalUncheckedBroadcast(LocalMsg &msg,
                                                LocalMsgVector &resps)
    {
        // Prepare
        std::future<void> respFuture;
        std::vector<LocalMsg> *responsesPtr;
        {
            const std::scoped_lock lock(m_mutex);
            this->prepareMsg(msg, true);
            m_pendingMsgs.insert({msg.id, PendingMsg{msg, true}});
            respFuture = m_pendingMsgs.at(msg.id).respPromise.get_future();
            responsesPtr = &m_pendingMsgs.at(msg.id).resps;
        }

        KVIK_LOGD("Broadcast message (id=%u): %s", msg.id, msg.toString().c_str());

        // Send
        KVIK_RETURN_ERROR(m_ll->send(msg));

        std::this_thread::sleep_for(m_conf.nodeConf.localDelivery.respTimeout);

        // Get responses, remove response promise and return
        {
            const std::scoped_lock lock(m_mutex);
            resps = std::move(*responsesPtr);
            m_pendingMsgs.erase(msg.id);
            for (const auto &respMsg : resps) {
                KVIK_LOGD("Response (id=%u): %s", msg.id,
                          respMsg.toString().c_str());
            }
            return ErrCode::SUCCESS;
        }
    }

    ErrCode Client::recvLocal(LocalMsg msg)
    {
        // Check node type
        if (msg.nodeType != NodeType::GATEWAY &&
            msg.nodeType != NodeType::RELAY) {
            KVIK_LOGD("Received message from invalid node type: %s",
                      msg.toString().c_str());
            return ErrCode::INVALID_ARG;
        }

        switch (msg.type) {
        case LocalMsgType::OK:
        case LocalMsgType::FAIL:
        case LocalMsgType::PROBE_RES:
            KVIK_RETURN_ERROR(this->recvLocalResp(msg));
            break;
        case LocalMsgType::SUB_DATA:
            KVIK_RETURN_ERROR(this->recvLocalSubData(msg));
            break;
        default:
            KVIK_LOGW("Received unknown message: %s", msg.toString().c_str());
            return ErrCode::INVALID_ARG;
        }

        return ErrCode::SUCCESS;
    }

    ErrCode Client::recvLocalResp(const LocalMsg &msg)
    {
        const std::scoped_lock lock(m_mutex);

        // Validate message ID
        if (!this->validateMsgId(msg.addr, msg.id)) {
            KVIK_LOGD("Discarding response with duplicate ID: %s",
                      msg.toString().c_str());
            return ErrCode::MSG_DUP_ID;
        }

        // Validate timestamp
        if (!m_ignoreInvalidMsgTs &&
            !this->validateMsgTimestamp(msg.ts, m_gw.tsDiff)) {
            KVIK_LOGD("Discarding response with invalid timestamp: %s",
                      msg.toString().c_str());
            return ErrCode::MSG_INVALID_TS;
        }

        // Check existence of corresponding request
        if (m_pendingMsgs.find(msg.reqId) == m_pendingMsgs.end()) {
            KVIK_LOGD("Discarding response for non-existing request: %s",
                      msg.toString().c_str());
            return ErrCode::NOT_FOUND;
        }

        auto &pendingMsg = m_pendingMsgs.at(msg.reqId);
        auto pendingType = pendingMsg.req.type;

        // Validate sender address
        if (!pendingMsg.broadcast && pendingMsg.req.addr != msg.addr) {
            KVIK_LOGD("Discarding response from different address: %s",
                      msg.toString().c_str());
            return ErrCode::MSG_UNKNOWN_SENDER;
        }

        if ((msg.type == LocalMsgType::OK &&
             pendingType == LocalMsgType::PUB_SUB_UNSUB) ||
            (msg.type == LocalMsgType::FAIL &&
             pendingType == LocalMsgType::PROBE_REQ) ||
            (msg.type == LocalMsgType::FAIL &&
             pendingType == LocalMsgType::PUB_SUB_UNSUB) ||
            (msg.type == LocalMsgType::PROBE_RES &&
             pendingType == LocalMsgType::PROBE_REQ)) {
            // Valid response, notify waiting sender
            pendingMsg.resps.push_back(msg);
            if (!pendingMsg.broadcast) {
                pendingMsg.respPromise.set_value();
            }
            return ErrCode::SUCCESS;
        } else {
            KVIK_LOGD("Response of type %s is invalid for request of type %s",
                      localMsgTypeToStr(msg.type),
                      localMsgTypeToStr(pendingType));
            return ErrCode::INVALID_ARG;
        }
    }

    ErrCode Client::recvLocalSubData(const LocalMsg &msg)
    {
        KVIK_LOGD("Received subscriptions data: %s",
                  msg.toString().c_str());

        ErrCode err;

        // Validate message ID and timestamp
        bool msgIdValid, msgTsValid, senderValid;
        {
            const std::scoped_lock lock(m_mutex);
            msgIdValid = this->validateMsgId(msg.addr, msg.id);
            msgTsValid = this->validateMsgTimestamp(msg.ts, m_gw.tsDiff);
            senderValid = msg.addr == m_gw.addr;
        }

        if (!msgIdValid || !msgTsValid) {
            KVIK_LOGD("Message is invalid, discarding: %s",
                      msg.toString().c_str());
            return !msgIdValid ? ErrCode::MSG_DUP_ID
                               : ErrCode::MSG_INVALID_TS;
        }

        // Validate sender address
        if (!senderValid) {
            KVIK_LOGD("Discarding data from unknown sender: %s",
                      msg.toString().c_str());
            return ErrCode::MSG_UNKNOWN_SENDER;
        }

        // Notify sender about successful delivery
        LocalMsg respMsg;
        respMsg.type = LocalMsgType::OK;
        this->sendLocalUnchecked(respMsg, respMsg, true);

        // Iterate all subscriptions
        for (const auto &subData : msg.subsData) {
            std::unordered_map<std::string, const SubCb &> entries;
            {
                const std::scoped_lock lock(m_mutex);
                entries = m_subDB.find(subData.topic);
            }

            for (const auto &[topic, cb] : entries) {
                KVIK_LOGD("Calling user callback for topic '%s'",
                          topic.c_str());
                cb(subData);
            }
        }

        return ErrCode::SUCCESS;
    }

    void Client::prepareMsg(LocalMsg &msg, bool broadcast)
    {
        auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch());
        auto gwTs = nowMs + m_gw.tsDiff;

        msg.addr = broadcast ? LocalAddr{} : m_gw.addr;
        msg.id = this->getMsgId();
        msg.ts =
            static_cast<uint16_t>(gwTs / m_conf.nodeConf.msgIdCache.timeUnit);
        msg.nodeType = NodeType::CLIENT;
    }

    void Client::subDBTick()
    {
        KVIK_LOGD("Renewal running");

        LocalMsg msg, respMsg;
        msg.type = LocalMsgType::PUB_SUB_UNSUB;

        // Populate data
        {
            const std::scoped_lock lock(m_mutex);
            m_subDB.forEach([&msg](const std::string &topic, const SubCb &cb) {
                msg.subs.push_back(topic);
            });
        }

        if (msg.subs.size() == 0) {
            // Nothing to do
            KVIK_LOGD("Nothing to renew");
            return;
        }

        // Send the message
        auto err = this->sendLocal(msg, respMsg);
        if (err != ErrCode::SUCCESS) {
            KVIK_LOGW("Error while sending the message");
        }
        if (respMsg.type != LocalMsgType::OK) {
            KVIK_LOGW("Received non-OK response");
        }

        KVIK_LOGD("Renewal done");
    }

    const ClientRetainedData Client::retain()
    {
        const std::scoped_lock lock(m_mutex);

        return {
            .gw = m_gw.retain(),
            .msgsFailCnt = m_msgsFailCnt,
            .timeSyncNoRespCnt = m_timeSyncNoRespCnt,
        };
    }
} // namespace kvik
