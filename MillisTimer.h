#pragma once

#include "arduino.h"

/// @brief Simple Arduino timer based on the \c millis() system
class MillisTimer
{
public:
    /// @brief Constructor
    /// @param period_ms - Timer period (in milliseconds)
    /// @param isPeriodic - Whether the timer is periodic (auto-repeating)
    MillisTimer(uint32_t period_ms, bool isPeriodic)
    : m_period_ms{period_ms}, m_isPeriodic{isPeriodic}
    {
    }

    /// @brief Starts this timer if not running, restarts this timer if running
    void start()
    {
        m_nextExpiration_ms = millis() + m_period_ms;
        m_isRunning = true;
    }

    /// @brief Stops this timer if running
    void stop()
    {
        m_isRunning = false;
    }

    /// @brief Process the time that has elapsed for this timer
    /// @note This method needs to be called very regularly since it is the only time that timer expiration is evaluated.  It also need to be called at intervals shorter than the period of the timer.
    /// @returns Whether the timer just expired
    bool process()
    {
        bool didExpire = false;
        if (m_isRunning)
        {
            uint32_t now = millis();
            if (static_cast<int32_t>(now - m_nextExpiration_ms) >= 0)
            {
                expire();
                didExpire = true;
            }
        }

        return didExpire;
    }

    /// @brief Accessor for this timer's running status
    /// @returns Whether the timer is presently running
    bool isRunning() const
    {
        return m_isRunning;
    }

private:
    /// Period of the timer (in milliseconds)
    uint32_t m_period_ms;
    /// Whether this timer is periodic (auto-repeating)
    bool m_isPeriodic;
    /// Whether this timer is presently running
    bool m_isRunning = false;
    /// @brief Millis() timestamp of the next time this timer will expire
    /// @details This timestamp is based on the millis() timer, which is the number of milliseconds since the CPU powered on.
    uint32_t m_nextExpiration_ms;

    /// @brief Expire this timer (takes into account whether timer is periodic)
    void expire()
    {
        if (m_isPeriodic)
        {
            start();
        }
        else
        {
            stop();
        }
    }
};
