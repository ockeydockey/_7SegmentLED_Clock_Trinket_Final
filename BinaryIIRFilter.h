#pragma once

#include "arduino.h"

/// @brief Binary Single Stage IIR Filter
/// @details Based on https://stackoverflow.com/questions/38918530/simple-low-pass-filter-in-fixed-point
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