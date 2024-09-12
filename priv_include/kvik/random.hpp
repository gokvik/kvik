/**
 * @file random.hpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Random generator
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <cstdlib>

namespace kvik
{
    /**
     * @brief Generates `len` truly random bytes in `buf`
     *
     * Implementation is plaform dependent.
     *
     * @param buf Buffer
     * @param len Length
     * @throw kvik::Exception Generation failed
     */
    void getRandomBytes(void *buf, size_t len);
} // namespace kvik
