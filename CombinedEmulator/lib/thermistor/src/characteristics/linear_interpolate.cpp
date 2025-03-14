#include "src/characteristics/linear_interpolate.hpp"

namespace thermistor {

  uint32_t LinearInterpolate::temperature_to_resistance(float temperature) const {
    return lookup(temperature);
  }

  float LinearInterpolate::resistance_to_temperature(uint32_t resistance) const {
    return lookup<1,0>(resistance);
  }
}