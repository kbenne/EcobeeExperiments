#include "ad529x.hpp"

#include <SPI.h>

namespace digipot
{
  enum CommandType : uint8_t
  {
    NOP = 0,
    WRITE_RDAC = 1,
    READ_RDAC = 2,
    STORE_RDAC = 3,
    RESET = 4,
    READ_MEMORY = 5,
    WRITE_CONTROL = 6,
    READ_CONTROL = 7,
    SHUTDOWN = 8
  };
  SPISettings spi_settings(50000000U, MSBFIRST, SPI_MODE1);

  uint16_t pack_packet(CommandType command, uint16_t data)
  {
    return command << 10 | (data & 0x01FF);
  }

  bool wait_for_ready(ad529x::ConnectionSettings settings, uint32_t timeout_millis)
  {
    auto start_time = millis();
    while (millis() - start_time < timeout_millis)
    {
      if (digitalRead(settings.ready_pin) == HIGH)
      {
        return true;
      }
    }
    return false;
  }

  uint16_t send(ad529x::ConnectionSettings settings, uint16_t packet, uint8_t device_index, bool read = false)
  {
    auto spi = settings.hardware_spi;
    pinMode(settings.sync_pin, OUTPUT);
    digitalWrite(settings.sync_pin, HIGH);
    pinMode(settings.ready_pin, INPUT_PULLDOWN);
    if (!wait_for_ready(settings, 10))
    {
      return 0;
    }
    digitalWrite(settings.sync_pin, LOW);

    spi->begin();
    spi->beginTransaction(spi_settings);
    uint16_t return_value = 0;

    spi->transfer16(packet);

    int i = 1;
    for (; i < device_index; i++)
    {
      spi->transfer16(0);
    }
    digitalWrite(settings.sync_pin, HIGH);

    if (read)
    {

      if (!wait_for_ready(settings, 10))
      {
        spi->endTransaction();
        spi->end();
        return 0;
      }
      digitalWrite(settings.sync_pin, LOW);

      for (; i < settings.device_count; i++)
      {
        return_value = spi->transfer16(0);
      }
      digitalWrite(settings.sync_pin, HIGH);
    }

    spi->endTransaction();
    spi->end();
    return return_value;
  }

  inline uint32_t AD529X::nominal_resistance() const {
    return m_nominal_resistance;
  }

  inline uint32_t AD529X::wiper_resolution() const {
    if(m_type == ad529x::AD5291) {
      return 256;
    }
    return 1024;
  }

  void AD529X::set_wiper_position(uint32_t position)
  {
    send(m_settings, pack_packet(WRITE_RDAC, position), m_device_index);
  }

  uint32_t AD529X::get_wiper_position()
  {
    return send(m_settings, pack_packet(READ_RDAC, 0), m_device_index, true);
  }

  void AD529X::write_control_register(bool memory_enable, bool rdac_write_protect, bool calibration_enable, bool memory_program_success)
  {
    send(m_settings, pack_packet(WRITE_CONTROL, (memory_enable << 0 | rdac_write_protect << 1 | calibration_enable << 2 | memory_program_success << 3)), m_device_index);
  }

  uint16_t AD529X::read_control_register() {
    return send(m_settings, pack_packet(READ_CONTROL, 0), m_device_index, true);
  }
}