/**
 * @file local_msg_id_cache.cpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Message ID cache
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <cstdint>
#include <functional>

#include "kvik/errors.hpp"
#include "kvik/local_addr.hpp"
#include "kvik/local_msg_id_cache.hpp"
#include "kvik/timer.hpp"

namespace kvik
{
    LocalMsgIdCache::LocalMsgIdCache(std::chrono::milliseconds timeUnit,
                                     uint8_t maxAge)
        : m_timeUnit{timeUnit}, m_maxAge{maxAge},
          m_timer{m_timeUnit, std::bind(&LocalMsgIdCache::tick, this)}
    {
    }

    bool LocalMsgIdCache::insert(const LocalAddr &addr, uint16_t id)
    {
        // Expiration
        auto expTickNum = m_tickNum + m_maxAge + 1;

        // Search in all address' sets for `id`
        for (const auto &[_, idSet] : m_cache[addr])
        {
            if (idSet.find(id) != idSet.end())
            {
                return false;
            }
        }

        m_cache[addr][expTickNum].insert(id);
        return true;
    }

    void LocalMsgIdCache::tick()
    {
        m_tickNum++;

        for (auto cacheIt = m_cache.begin(); cacheIt != m_cache.end();)
        {
            for (auto addrCacheIt = cacheIt->second.begin();
                 addrCacheIt != cacheIt->second.end();)
            {
                // If entry expired
                if (m_tickNum == addrCacheIt->first)
                    addrCacheIt = cacheIt->second.erase(addrCacheIt);
                else
                    addrCacheIt++;
            }

            // If no more timestamps for address
            if (cacheIt->second.empty())
                cacheIt = m_cache.erase(cacheIt);
            else
                cacheIt++;
        }
    }
} // namespace kvik
