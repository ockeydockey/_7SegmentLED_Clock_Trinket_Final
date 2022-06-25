#pragma once

#include "arduino.h"

/// @brief Binary Single Stage IIR Filter
class BinaryIIRFilter
{
public:
    /// @brief Constructor
    /// @param attenuationFactor - Number of bits to shift data (used as an attenuation factor)
    /// @param initialValue - [optional] Seed value for the filter
    BinaryIIRFilter(uint8_t attenuationFactor, uint16_t initialValue = 0U)
    : m_attenuationFactor{attenuationFactor}, m_accumulator{initialValue << attenuationFactor}
    {}

    /// @brief Add new data sample
    /// @param value - New sample value
    /// @returns New filtered value
    uint16_t addSample(uint16_t value)
    {
        // m_accumulator = static_cast<uint16>(static_cast<uint32_t>(m_accumulator) + (static_cast<uint32_t>(value) - m_accumulator) >> m_attenuationFactor);
        // return m_accumulator;

        uint32_t out = getValue();
        m_accumulator = m_accumulator - out + value;
        return static_cast<uint16_t>(out);
    }

    /// @brief Get the filtered value
    /// @returns Present filtered value
    inline uint16_t getValue() const
    {
        return (m_accumulator >> m_attenuationFactor);
    }

private:
    uint8_t m_attenuationFactor;
    uint32_t m_accumulator;
};