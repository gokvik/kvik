/**
 * @file local_addr_mac.cpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Local layer address container for MAC address
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <cstring>
#include <memory>

#include "kvik/local_addr_mac.hpp"

namespace kvik
{
    /**
     * @brief Length of MAC address in bytes
     */
    static constexpr size_t MAC_LEN = 6;

    LocalAddrMAC::LocalAddrMAC(const uint8_t *mac)
    {
        uint8_t macZeroes[MAC_LEN] = {};
        if (mac == nullptr)
        {
            mac = macZeroes;
        }

        // Internal representation
        addr = std::vector<uint8_t>(mac, mac + MAC_LEN);
    }

    LocalAddrMAC LocalAddrMAC::zeroes()
    {
        return LocalAddrMAC();
    }

    LocalAddrMAC LocalAddrMAC::broadcast()
    {
        uint8_t mac[MAC_LEN];
        memset(mac, 0xFF, MAC_LEN);
        return LocalAddrMAC(mac);
    }

    void LocalAddrMAC::toMAC(uint8_t *mac) const
    {
        for (unsigned i = 0; i < addr.size(); i++)
        {
            mac[i] = addr[i];
        }
    }
}
