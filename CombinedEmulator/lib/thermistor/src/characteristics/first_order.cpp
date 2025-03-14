#include "first_order.hpp"

namespace thermistor {
  uint32_t FirstOrder::temperature_to_resistance(float temperature) const {
    uint32_t resistance = m_resistance_0 + (m_temperature_coefficient * (temperature - m_temperature_0));
    return resistance;
  }
  float FirstOrder::resistance_to_temperature(uint32_t resistance) const {
    float temperature = m_temperature_0 + ((resistance - m_resistance_0) / m_temperature_coefficient);
    return temperature;
  }
}