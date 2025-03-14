#ifndef _THERMISTOR_CHARACTERISTICS_FIRST_ORDER_HPP_
#define _THERMISTOR_CHARACTERISTICS_FIRST_ORDER_HPP_

#include "../characteristic_curve.hpp"

namespace thermistor {
  class FirstOrder : CharacteristicCurve {
    /**
     * R = R_0 + (k * (T - T_0))
     * T = T_0 + (R - R_0) / k
     */

    const float m_temperature_0;
    const uint32_t m_resistance_0;
    const float m_temperature_coefficient;
  public:
    FirstOrder(float temperature_0, uint32_t resistance_0, float temperature_coefficient) :
    m_temperature_0(temperature_0), m_resistance_0(resistance_0), m_temperature_coefficient(temperature_coefficient)
    {}

    uint32_t temperature_to_resistance(float temperature) const override;

    float resistance_to_temperature(uint32_t resistance) const override;
  };
}

#endif