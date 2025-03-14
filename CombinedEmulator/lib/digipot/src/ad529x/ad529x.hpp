#ifndef _AD529X_HPP_
#define _AD529X_HPP_

#include "../digipot.hpp"
#include <SPI.h>

namespace digipot {
  namespace ad529x {
    struct ConnectionSettings {
      arduino::HardwareSPI * hardware_spi;
      uint8_t device_count;
      pin_size_t sync_pin;
      pin_size_t ready_pin;
    };
  
    enum DeviceType {
      AD5291,
      AD5292
    };
  }
  class AD529X : Digipot
  {
    const ad529x::ConnectionSettings m_settings;
    const ad529x::DeviceType m_type;
    const uint8_t m_device_index;
    const uint32_t m_nominal_resistance;
    const uint16_t m_wiper_steps = (m_type == AD5291) ? 256 : 1024;
    const uint16_t m_wiper_resolution = m_nominal_resistance / m_wiper_steps;

  public:
    AD529X(ad529x::ConnectionSettings settings, ad529x::DeviceType type, uint8_t device_index, uint32_t nominal_resistance) : m_settings(settings), m_type(type), m_device_index(device_index), m_nominal_resistance(nominal_resistance) {}

    inline uint32_t nominal_resistance() const override;
    inline uint32_t wiper_resolution() const override;

    void set_wiper_position(uint32_t position) override;

    uint32_t get_wiper_position() override;

    void write_control_register(bool memory_enable = false, bool rdac_write_protect = false, bool calibration_enable = false, bool memory_program_success = false);

    uint16_t read_control_register();
  };

}

#endif