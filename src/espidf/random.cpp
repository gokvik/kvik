/**
 * @file random.cpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Random generator
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "esp_random.h"

#include "kvik/random.hpp"

namespace kvik
{
    void getRandomBytes(void *buf, size_t len)
    {
        esp_fill_random(buf, len);
    }
} // namespace kvik
