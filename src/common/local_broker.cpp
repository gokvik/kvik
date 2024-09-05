/**
 * @file local_broker.cpp
 * @author Dávid Benko (davidbenko@davidbenko.dev)
 * @brief Local broker remote layer for Kvik
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <mutex>

#include "kvik/local_broker.hpp"
#include "kvik/logger.hpp"

// Log tag
static const char *KVIK_LOG_TAG = "Kvik/LocalBroker";

namespace kvik
{
    LocalBroker::LocalBroker()
    {
        KVIK_LOGD("Initialized");
    }

    LocalBroker::~LocalBroker()
    {
        KVIK_LOGD("Deinitialized");
    }

    ErrCode LocalBroker::publish(const RemoteMsg &msg)
    {
        KVIK_LOGD("Publishing %zu bytes to topic '%s'",
                  msg.payload.length(), msg.topic.c_str());

        // Check if node is subscribed to this topic
        bool subscribed;
        {
            const std::scoped_lock lock(m_mutex);
            subscribed = !m_subs.find(msg.topic).empty();
        }

        if (subscribed && m_recvCb != nullptr)
        {
            KVIK_LOGD("Subscription exists for published data, calling "
                      "callback on topic '%s'",
                      msg.topic.c_str());

            // Send data back as received
            KVIK_RETURN_ERROR(m_recvCb(msg));
        }

        return ErrCode::SUCCESS;
    }

    ErrCode LocalBroker::subscribe(const std::string &topic)
    {
        const std::scoped_lock lock(m_mutex);

        KVIK_LOGD("Subscribe to topic '%s'", topic.c_str());

        m_subs.insert(topic, true);
        return ErrCode::SUCCESS;
    }

    ErrCode LocalBroker::unsubscribe(const std::string &topic)
    {
        const std::scoped_lock lock(m_mutex);

        if (!m_subs.remove(topic))
        {
            KVIK_LOGD("Unsubscribe from topic '%s': subscription doesn't exist",
                      topic.c_str());
            return ErrCode::NOT_FOUND;
        }

        KVIK_LOGD("Unsubscribe from topic '%s': success", topic.c_str());
        return ErrCode::SUCCESS;
    }
} // namespace kvik
