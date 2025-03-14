#ifndef _CHARACTERISTIC_CURVE_TABLE_HPP_
#define _CHARACTERISTIC_CURVE_TABLE_HPP_
#include <cstdint>

namespace thermistor {

  class CharacteristicCurve {
  public:
    virtual uint32_t temperature_to_resistance(float temperature) const = 0;

    virtual float resistance_to_temperature(uint32_t resistance) const = 0;
  };

}

#endif