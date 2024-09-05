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
#include "kvik/version.hpp"

// Log tag
static const char *KVIK_LOG_TAG = "Kvik/Node";

namespace kvik
{
    INode::INode()
    {
        KVIK_LOGI("Kvik version: %s", kvik::VERSION);
    }
} // namespace kvik
