/**
 * @file dummy_node.hpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Dummy node for testing purposes
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <string>
#include <vector>

#include "kvik/node.hpp"

namespace kvik
{
    /**
     * @brief Dummy generic node
     *
     * Just logs all actions to local variables.
     */
    class DummyNode : public INode
    {
    public:
        using PubsLog = std::vector<PubData>;
        using SubsLog = std::vector<SubReq>;
        using UnsubsLog = std::vector<std::string>;

        PubsLog pubsLog;        //!< All publications
        SubsLog subsLog;        //!< All subscription
        UnsubsLog unsubsLog;    //!< All unsub requests
        size_t unsubAllCnt = 0; //!< Counter of unsub all requests
        size_t resubAllCnt = 0; //!< Counter of resub all requests

        using INode::getMsgId;
        using INode::INode;
        using INode::validateMsgId;
        using INode::validateMsgTimestamp;
        using INode::buildReportRssiTopic;

        ErrCode pubSubUnsubBulk(const std::vector<PubData> &newPubs,
                                const std::vector<SubReq> &newSubs,
                                const std::vector<std::string> &newUnsubs)
        {
            pubsLog.insert(pubsLog.end(), newPubs.begin(), newPubs.end());
            subsLog.insert(subsLog.end(), newSubs.begin(), newSubs.end());
            unsubsLog.insert(unsubsLog.end(), newUnsubs.begin(), newUnsubs.end());
            return ErrCode::SUCCESS;
        }

        ErrCode unsubscribeAll()
        {
            unsubAllCnt++;
            return ErrCode::SUCCESS;
        }

        ErrCode resubscribeAll()
        {
            resubAllCnt++;
            return ErrCode::SUCCESS;
        }
    };
} // namespace kvik
