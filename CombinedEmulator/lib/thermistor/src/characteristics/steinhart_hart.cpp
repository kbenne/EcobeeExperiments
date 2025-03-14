#include "steinhart_hart.hpp"

#include <cmath>

namespace thermistor {
  uint32_t SteinhartHart::temperature_to_resistance(float temperature) const {
    temperature = temperature + 273.15;
    float y = (1.0 / (2 * m_c)) * (m_a - (1.0/temperature));
    float x = y + sqrtf(powf(m_b / (3 * m_c), 3) + pow(y, 2));
    float resistance = expf((m_b / (3 * m_c * powf(x, 1.0/3.0))) - powf(x, 1.0/3.0));
    return resistance;
  }
  float SteinhartHart::resistance_to_temperature(uint32_t resistance) const {
    float log_resistance = logf(resistance);
    float temperature = 1.0 / (m_a + (m_b * log_resistance) + m_c * powf(log_resistance, 3.0));
    return temperature - 273.15;
  }
}