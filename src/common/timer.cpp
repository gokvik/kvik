/**
 * @file timer.cpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Timer for Kvik purposes
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <mutex>
#include <thread>

#include "kvik/timer.hpp"

namespace kvik
{
    Timer::Timer(const std::chrono::milliseconds interval,
                 std::function<void()> cb)
        : m_interval{interval},
          m_nextExec{std::chrono::steady_clock::now() + interval},
          m_run{true}, m_cb{cb}, m_thread{&Timer::handlerThread, this}
    {
    }

    Timer::~Timer()
    {
        {
            std::scoped_lock lock{m_mutex};
            m_run = false;
        }

        // Notify handler thread
        m_cv.notify_one();

        // Wait for thread's return
        m_thread.join();
    }

    void Timer::handlerThread()
    {
        while (true)
        {
            {
                // Wait for `m_interval` or destructor notification again
                std::unique_lock lock{m_mutex};
                if (m_cv.wait_until(lock, m_nextExec, [this]()
                                    { return !m_run; }))
                {
                    // Destructor has been called
                    break;
                }
            }

            // Call callback
            m_cb();

            m_nextExec += m_interval;
        }
    }
}
