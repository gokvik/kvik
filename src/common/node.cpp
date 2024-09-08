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
          m_nonceCache{conf.replayProtection.nonceCacheTimeUnit,
                       conf.replayProtection.nonceCacheMaxAge}
    {
        // Init nonce
        getRandomBytes(&m_nonce, sizeof(m_nonce));

        if (!VERSION_UNKNOWN)
        {
            KVIK_LOGI("Kvik version: %s", VERSION);
        }
    }

    INode::~INode()
    {
    }

    uint16_t INode::getNonce()
    {
        return m_nonce++;
    }

    bool INode::validateNonce(const LocalAddr &addr, uint16_t nonce)
    {
        return m_nonceCache.insert(addr, nonce);
    }
} // namespace kvik
