#ifndef _THERMISTOR_CHARACTERISTICS_STEINHART_HART_HPP_
#define _THERMISTOR_CHARACTERISTICS_STEINHART_HART_HPP_

#include "../characteristic_curve.hpp"

namespace thermistor {
  class SteinhartHart : CharacteristicCurve {
    /**
     * 1/T = a + b * ln(R) + c (ln(R))^3
     */

    const float m_a, m_b, m_c;
  public:
    SteinhartHart(float a, float b, float c) :
    m_a(a), m_b(b), m_c(c) {}

    uint32_t temperature_to_resistance(float temperature) const override;

    float resistance_to_temperature(uint32_t resistance) const override;
  };
}

#endif