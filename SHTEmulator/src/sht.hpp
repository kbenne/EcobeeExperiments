#ifndef SHT_INCLUDED
#define SHT_INCLUDED

#include <Arduino.h>

namespace sht {

static int16_t Command;
static int16_t TemperatureRegister;
static int16_t HumidityRegister;

constexpr static byte SHT4x_DEFAULT_ADDR = 0x44;
constexpr static byte SHT4x_NOHEAT_HIGHPRECISION = 0xFD;
constexpr static byte SHT4x_NOHEAT_MEDPRECISION = 0xF6;
constexpr static byte SHT4x_NOHEAT_LOWPRECISION = 0xE0;
constexpr static byte SHT4x_HIGHHEAT_1S = 0x39;
constexpr static byte SHT4x_HIGHHEAT_100MS = 0x32;
constexpr static byte SHT4x_MEDHEAT_1S = 0x2F;
constexpr static byte SHT4x_MEDHEAT_100MS = 0x24;
constexpr static byte SHT4x_LOWHEAT_1S = 0x1E;
constexpr static byte SHT4x_LOWHEAT_100MS = 0x15;
constexpr static byte SHT4x_READSERIAL = 0x89;
constexpr static byte SHT4x_SOFTRESET = 0x94;
static byte SerialNumber[6];

void init();
void begin();

// Set the raw sensor reading (adc_T) given temperature T in degC
// This solves the quadratic equation given in Appendix 8.1 in the BME datasheet
void set_T(const double &T);
double get_T(void);
void set_H(const double &H);

bool is_measure_command(int16_t command);
void eval_command(int16_t command);
uint8_t crc8(const uint8_t *data, int len);

void on_wire_receive(int num_bytes);
void on_wire_request(void);

void init_serial_number();

} // namespace sht

#endif // SHT_INCLUDED
