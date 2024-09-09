/**
 * @file node.cpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Node interface for Kvik
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "kvik/logger.hpp"
#include "kvik/node.hpp"
#include "kvik/node_config.hpp"
#include "kvik/random.hpp"
#include "kvik/version.hpp"

// Log tag
static const char *KVIK_LOG_TAG = "Kvik/Node";

namespace kvik
{
    INode::INode(const NodeConfig &conf)
        : m_nodeConf{conf},
          m_msgIdCache{conf.msgIdCache.timeUnit, conf.msgIdCache.maxAge}
    {
        // Init message ID
        getRandomBytes(&m_msgId, sizeof(m_msgId));

        if (!VERSION_UNKNOWN)
        {
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
} // namespace kvik
