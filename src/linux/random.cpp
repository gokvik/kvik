/**
 * @file random.cpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Random generator
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <sys/random.h>

#include "kvik/errors.hpp"
#include "kvik/random.hpp"

namespace kvik
{
    void getRandomBytes(void *buf, size_t len)
    {
        if (getrandom(buf, len, 0) != len) {
            throw Exception("Generation failed");
        }
    }
} // namespace kvik
