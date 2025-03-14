#ifndef _THERMISTOR_THERMISTOR_HPP_
#define _THERMISTOR_THERMISTOR_HPP_

#include "digipot.hpp"
#include "characteristic_curve.hpp"

#include <type_traits>

namespace thermistor {

  enum PotentiometerConfiguration {
    AW,
    WB
  };

  template <typename TDigipot, typename TCharacteristicCurve>
  class Thermistor {
    static_assert(std::is_base_of_v<TDigipot, digipot::Digipot>);
    static_assert(std::is_base_of_v<TCharacteristicCurve, CharacteristicCurve>);
    TDigipot m_digipot;
    TCharacteristicCurve m_characteristic_curve;
    PotentiometerConfiguration m_potentiometer_configuration;

  public:
    Thermistor(const TDigipot& digipot, const TCharacteristicCurve characteristic_curve, PotentiometerConfiguration potentiometer_configuration) :
      m_digipot(digipot), m_characteristic_curve(characteristic_curve), m_potentiometer_configuration(potentiometer_configuration) {}

    void set_temperature(float temperature) {
      uint32_t resistance = m_characteristic_curve.temperature_to_resistance(temperature);
      if(m_potentiometer_configuration == AW) {
        m_digipot.set_aw_resistance(resistance);
      } else {
        m_digipot.set_wb_resistance(resistance);
      }
    }

    float get_temperature() {
      uint32_t resistance;
      if(m_potentiometer_configuration == AW) {
        resistance = m_digipot.get_aw_resistance();
      } else {
        resistance = m_digipot.get_wb_resistance();
      }
      return m_characteristic_curve.resistance_to_temperature(resistance);
    }
  };
}

#endif