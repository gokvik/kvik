/**
 * @file node.cpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Node interface for Kvik
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "kvik/node.hpp"
#include "kvik/errors.hpp"
#include "kvik/logger.hpp"
#include "kvik/node_config.hpp"
#include "kvik/random.hpp"
#include "kvik/version.hpp"

// Log tag
static const char *KVIK_LOG_TAG = "Kvik/Node";

namespace kvik
{
    INode::INode(NodeConfig conf)
        : m_nodeConf{conf},
          m_msgIdCache{conf.msgIdCache.timeUnit, conf.msgIdCache.maxAge}
    {
        // Init message ID
        getRandomBytes(&m_msgId, sizeof(m_msgId));

        if (m_nodeConf.msgIdCache.maxAge == 0) {
            KVIK_THROW_EXC("NodeConfig.msgIdCache.maxAge can't be 0!");
        }

        if (!VERSION_UNKNOWN) {
            KVIK_LOGI("Kvik version: %s", VERSION);
        }
    }

    INode::~INode()
    {
    }

    uint16_t INode::getMsgId()
    {
        return m_msgId++;
    }

    bool INode::validateMsgId(const LocalAddr &addr, uint16_t id)
    {
        return m_msgIdCache.insert(addr, id);
    }

    bool INode::validateMsgTimestamp(uint16_t msgTsUnits,
                                     std::chrono::milliseconds tsDiff)
    {
        auto maxDriftUnits = m_nodeConf.msgIdCache.maxAge - 1;
        auto nowTs = std::chrono::steady_clock::now().time_since_epoch() +
                     tsDiff;
        auto nowUnits64 = nowTs / m_nodeConf.msgIdCache.timeUnit;
        uint16_t nowUnits = nowUnits64;

        if (nowUnits - maxDriftUnits > nowUnits) {
            // Underflow
            nowUnits += maxDriftUnits;
            msgTsUnits += maxDriftUnits;
        }

        return msgTsUnits <= nowUnits &&
               msgTsUnits >= nowUnits - maxDriftUnits;
    }

    std::string INode::buildReportRssiTopic(const LocalAddr &peer) const
    {
        return m_nodeConf.reporting.baseTopic +
               m_nodeConf.topicSep.levelSeparator +
               m_nodeConf.reporting.rssiSubtopic +
               m_nodeConf.topicSep.levelSeparator +
               peer.toString();
    }
} // namespace kvik
