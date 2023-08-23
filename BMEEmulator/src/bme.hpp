#ifndef BME_INCLUDED
#define BME_INCLUDED

#include <Arduino.h>

// bme namespace contains functions to emulate a BME280
// The default i2C "Wire" interface is used for communication
namespace bme {

void init();
void begin();

// Set the raw sensor reading (adc_T) given temperature T in degC
// This solves the quadratic equation given in Appendix 8.1 in the BME datasheet
void set_T(const double &T);
void set_H(double H);

// Many calibration terms are composed of two bytes.
// The first register is the least significant digit.

// Temperature calibration
uint16_t dig_T1();
int16_t dig_T2();
int16_t dig_T3();

// Humidity calibration
uint8_t dig_H1();
int16_t dig_H2();
uint8_t dig_H3();
int16_t dig_H4();
int16_t dig_H5();
uint8_t dig_H6();

enum class Root { Min, Max };
double solve_quadratic(const double &a, const double &b, const double &c, Root root);

int32_t adc_T();
int32_t t_fine();

void on_wire_receive(int numBytes);
void on_wire_request(void);

static byte Address = 0x0;
constexpr static int RegisterSize = 256;
static byte Registers[RegisterSize];
void init_registers();

constexpr static byte BME280_REGISTER_DIG_T1 = 0x88;
constexpr static byte BME280_REGISTER_DIG_T2 = 0x8A;
constexpr static byte BME280_REGISTER_DIG_T3 = 0x8C;
constexpr static byte BME280_REGISTER_DIG_P1 = 0x8E;
constexpr static byte BME280_REGISTER_DIG_P2 = 0x90;
constexpr static byte BME280_REGISTER_DIG_P3 = 0x92;
constexpr static byte BME280_REGISTER_DIG_P4 = 0x94;
constexpr static byte BME280_REGISTER_DIG_P5 = 0x96;
constexpr static byte BME280_REGISTER_DIG_P6 = 0x98;
constexpr static byte BME280_REGISTER_DIG_P7 = 0x9A;
constexpr static byte BME280_REGISTER_DIG_P8 = 0x9C;
constexpr static byte BME280_REGISTER_DIG_P9 = 0x9E;
constexpr static byte BME280_REGISTER_DIG_H1 = 0xA1;
constexpr static byte BME280_REGISTER_DIG_H2 = 0xE1;
constexpr static byte BME280_REGISTER_DIG_H3 = 0xE3;
constexpr static byte BME280_REGISTER_DIG_H4 = 0xE4;
constexpr static byte BME280_REGISTER_DIG_H5 = 0xE5;
constexpr static byte BME280_REGISTER_DIG_H6 = 0xE7;
constexpr static byte BME280_REGISTER_CHIPID = 0xD0;
constexpr static byte BME280_REGISTER_VERSION = 0xD1;
constexpr static byte BME280_REGISTER_SOFTRESET = 0xE0;
constexpr static byte BME280_REGISTER_CAL26 = 0xE1;
constexpr static byte BME280_REGISTER_CONTROLHUMID = 0xF2;
constexpr static byte BME280_REGISTER_STATUS = 0xF3;
constexpr static byte BME280_REGISTER_CONTROL = 0xF4;
constexpr static byte BME280_REGISTER_CONFIG = 0xF5;
constexpr static byte BME280_REGISTER_PRESSUREDATA = 0xF7;
constexpr static byte BME280_REGISTER_TEMPDATA = 0xFA;
constexpr static byte BME280_REGISTER_HUMIDDATA = 0xFD;

} // namespace bme

#endif // BME_INCLUDED
