/**
 * @file nonce_cache.hpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Nonce cache
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <cstdint>
#include <functional>
#include <unordered_map>
#include <unordered_set>

#include "kvik/local_addr.hpp"
#include "kvik/timer.hpp"

namespace kvik
{
    /**
     * @brief Associative nonce cache
     *
     * Tracks recent nonces from all nodes and detects duplicates.
     */
    class NonceCache
    {
    protected:
        using NonceSet = std::unordered_set<uint16_t>;
        using AddrTsCache = std::unordered_map<uint16_t, NonceSet>;
        using Cache = std::unordered_map<LocalAddr, AddrTsCache>;

        // Config
        std::chrono::milliseconds m_timeUnit;
        uint8_t m_maxAge;

        /**
         * @brief Cache
         *
         * Implemented as mapping address -> timestamp -> set of nonces.
         */
        Cache m_cache;

        Timer m_timer;
        uint16_t m_tickNum = 0; //!< Tick counter (used instead of timestamps)

    public:
        /**
         * @brief Constructs nonce cache object
         * @param timeUnit Time unit (see details in `GenericNodeConfig`)
         * @param maxAge Maximum entry age (see details in `GenericNodeConfig`)
         */
        NonceCache(std::chrono::milliseconds timeUnit, uint8_t maxAge);

        /**
         * @brief Inserts new entry if not already present
         * @param addr Message peer address
         * @param nonce Message nonce
         * @return true Entry inserted
         * @return false Entry already present (duplicate)
         */
        bool insert(const LocalAddr &addr, uint16_t nonce);

    private:
        /**
         * @brief Timer tick callback
         *
         * Cleans all expired entries.
         */
        void tick();
    };
} // namespace kvik
