#ifndef _DIGIPOT_DEVICE_HPP_
#define _DIGIPOT_DEVICE_HPP_

#include <cstdint>
#include <type_traits>
#include <algorithm>

namespace digipot {
  class Digipot {
  public:
    virtual inline uint32_t nominal_resistance() const = 0;
    virtual inline uint32_t wiper_resolution() const = 0;

    virtual void set_wiper_position(uint32_t position) = 0;
    virtual uint32_t get_wiper_position() = 0;

    virtual inline void set_wb_resistance(uint32_t r_ohms) {
      r_ohms = std::min(nominal_resistance(), r_ohms);

    }
    virtual inline void set_aw_resistance(uint32_t r_ohms) {
      set_wb_resistance(nominal_resistance() - r_ohms);
    }

    virtual inline uint32_t get_wb_resistance() {
      return get_wiper_position() * resistance_resolution();
    }
    virtual inline uint32_t get_aw_resistance() {
      return nominal_resistance() - get_wb_resistance();
    }
  protected:
    virtual uint32_t resistance_resolution() const {
      return nominal_resistance() / wiper_resolution();
    }
  };
}

#endif