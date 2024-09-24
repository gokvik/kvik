/**
 * @file client_config.hpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Client node type's configuration
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <chrono>
#include <cstdint>

#include "kvik/node.hpp"
#include "kvik/node_config.hpp"

namespace kvik
{
    /**
     * @brief Client configuration
     */
    struct ClientConfig
    {
        struct GatewayDiscovery
        {
            /**
             * @brief Minimum delay after failed discovery attempt
             *
             * How long to cool down after first failed gateway discovery
             * attempt.
             * Actual delay starts at `dscvMinDelay` and multiplies by 2 on
             * each further failure, until it gets capped at `dscvMaxDelay`.
             * After successful discovery, the delay is reset.
             */
            std::chrono::milliseconds dscvMinDelay = std::chrono::seconds(1);

            /**
             * @brief Maximum delay after failed discovery attempt
             *
             * How long to cool down after nth failed gateway discovery
             * attempt.
             * Actual delay starts at `dscvMinDelay` and multiplies by 2 on
             * each further failure, until it gets capped at `dscvMaxDelay`.
             * After successful discovery, the delay is reset.
             */
            std::chrono::milliseconds dscvMaxDelay = std::chrono::minutes(2);

            /**
             * @brief Failure threshold of initial discovery
             *
             * Consider this many failed gateway discovery attempts as
             * unrecoverable and throw an error.
             *
             * Value 0 means try indefinitely many times.
             */
            uint16_t initialDscvFailThres = 5;

            /**
             * @brief Discovery trigger threshold
             *
             * After how many failed or unresponded messages from current
             * gateway in a row to trigger gateway rediscovery.
             *
             * Values 0 and 1 are equivalent (no loss is permitted).
             */
            uint16_t trigMsgsFailCnt = 5;

            /**
             * @brief Discovery trigger threshold for time synchronization
             *
             * After how many probes (during time synchronization process)
             * without response from current gateway to trigger gateway
             * rediscovery.
             *
             * Values 0 and 1 are equivalent (no loss is permitted).
             */
            uint16_t trigTimeSyncNoRespCnt = 2;
        };

        struct Reporting
        {
            /**
             * @brief Report RSSI during gateway discovery
             *
             * Whether to report RSSI value (if used by local layer protocol)
             * of all PROBE_RES messages received during time synchronization.
             * In other words, client reports signal strength to all available
             * gateways.
             *
             * Only one message is generated containing all publications.
             */
            bool rssiOnGwDscv = true;
        };

        struct SubDB
        {
            /**
             * @brief Lifetime of subscription from client
             *
             * Client will automatically renew the subscription after this
             * timeout.
             *
             * Gateway's subscription lifetime must be higher
             * (default is 15 minutes).
             */
            std::chrono::milliseconds subLifetime = std::chrono::minutes(10);
        };

        struct TimeSync
        {
            /**
             * @brief Sync system time
             *
             * Whether to modify system time after successful time
             * synchronization with gateway.
             * As gateways usually sync their time with SNTP servers, their
             * time is fairly precise. On IoT devices, this option can be
             * safely turned on. On Linux systems, however, this isn't usually
             * desired, as system time is synchronized already by other
             * means.
             *
             * When set to `false` gateway time is synchronized only locally
             * with `Client` object and used only for message validation.
             */
            bool syncSystemTime = false;

            /**
             * @brief How often to probe gateway
             *
             * This ensures correct time synchronization which is necessary
             * for replay attack protection.
             *
             * If set to 0, reprobing is disabled. You have to reprobe from
             * your code. (If your device is periodically waking up and doing
             * full initialization afterwards, this can be safely disabled.)
             */
            std::chrono::milliseconds reprobeGatewayInterval =
                std::chrono::minutes(60);
        };

        NodeConfig nodeConf;
        GatewayDiscovery gwDscv;
        Reporting reporting;
        SubDB subDB;
        TimeSync timeSync;
    };
} // namespace kvik
