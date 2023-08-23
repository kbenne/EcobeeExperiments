#include "bme.hpp"
#include <Wire.h>

namespace bme {

void init() {
  init_registers();

  set_T(22.0);
  set_H(50.0);

  Wire.onReceive(on_wire_receive);
  Wire.onRequest(on_wire_request);
}

void begin() {
  Wire.begin(0x76); // This is the I2C address of a BME280
  Serial.println("BME emulator started with I2C address 0x76 on Wire");
}

uint16_t dig_T1() {
   return uint16_t((Registers[BME280_REGISTER_DIG_T1 + 1]) << 8) | uint16_t((Registers[BME280_REGISTER_DIG_T1]));
}

int16_t dig_T2() {
   return int16_t((Registers[BME280_REGISTER_DIG_T2 + 1]) << 8) | int16_t((Registers[BME280_REGISTER_DIG_T2]));
}

int16_t dig_T3() {
   return int16_t((Registers[BME280_REGISTER_DIG_T3 + 1]) << 8) | int16_t((Registers[BME280_REGISTER_DIG_T3]));
}

uint8_t dig_H1() {
  return Registers[BME280_REGISTER_DIG_H1];
}

int16_t dig_H2() {
  return int16_t((Registers[BME280_REGISTER_DIG_H2 + 1]) << 8) | int16_t((Registers[BME280_REGISTER_DIG_H2]));
}

uint8_t dig_H3() {
  return Registers[BME280_REGISTER_DIG_H3];
}

int16_t dig_H4() {
  return ((int8_t)Registers[BME280_REGISTER_DIG_H4] << 4) | (Registers[BME280_REGISTER_DIG_H4 + 1] & 0xF);
}

int16_t dig_H5() {
  return ((int8_t)Registers[BME280_REGISTER_DIG_H5 + 1] << 4) | (Registers[BME280_REGISTER_DIG_H5] >> 4);
}

uint8_t dig_H6() {
  return Registers[BME280_REGISTER_DIG_H6];
}

double solve_quadratic(const double &a, const double &b, const double &c, Root root) {
  // a * x ** 2 + b * x + c = 0, return max x
  const double discriminant = pow(b, 2.0) - 4.0 * a * c;

  if ( discriminant > 0) {
    const double square_root = sqrt(discriminant);
    if (root == Root::Min) {
      return fmin((-b + square_root) / (2.0 * a), (-b - square_root) / (2.0 * a));
    } else {
      return fmax((-b + square_root) / (2.0 * a), (-b - square_root) / (2.0 * a));
    }
  } else {
    return -b / (2.0 * a);
  }
}

// Set the raw sensor reading (adc_T) given temperature T in degC
// This solves the quadratic equation given in Appendix 8.1 in the BME datasheet
void set_T(const double &T) {
  const double T1 = dig_T1();
  const double T2 = dig_T2();
  const double T3 = dig_T3();

  const double a = T3 / pow(131072.0, 2.0);
  const double b = (T2 / 16384.0) - (2.0 * T1 * T3 / 131072.0 / 8192.0);
  const double c = (pow(T1 / 8192.0, 2.0) * T3) - (T1 * T2 / 1024.0) - (T * 5120.0);

  const double result = solve_quadratic(a, b, c, Root::Max);

  // The four least significant digits are not used by the TEMPDATA register
  // so shift 4 digits to the left
  //return (int32_t)result << 4;
  int32_t adc_T_value = (int32_t)result << 4;
  Registers[BME280_REGISTER_TEMPDATA] = adc_T_value >> 16;
  Registers[BME280_REGISTER_TEMPDATA + 1] = adc_T_value >> 8;
  Registers[BME280_REGISTER_TEMPDATA + 2] = adc_T_value;
}

int32_t adc_T() {
  return 
    int32_t(Registers[BME280_REGISTER_TEMPDATA]) << 16 |
    int32_t(Registers[BME280_REGISTER_TEMPDATA + 1]) << 8 |
    int32_t(Registers[BME280_REGISTER_TEMPDATA + 2]);
}

int32_t t_fine() {
  const int32_t t_adc_T = adc_T() >> 4;
  const int32_t t1 = dig_T1();
  const int32_t t2 = dig_T2();
  const int32_t t3 = dig_T3();

  const int32_t var1 = ((((t_adc_T >> 3) - (t1 << 1))) * t2) >> 11;
  const int32_t var2 = (((((t_adc_T >> 4) - t1) * ((t_adc_T >> 4) - t1)) >> 12) * t3) >> 14;

  return var1 + var2;
}

void set_H(double H) {
  if (H > 100.0)
     H = 100.0;
  else if (H < 0.0)
     H = 0.0;
  
  const double t_fine_double = t_fine();
  const double h1 = dig_H1();
  const double h2 = dig_H2();
  const double h3 = dig_H3();
  const double h4 = dig_H4();
  const double h5 = dig_H5();
  const double h6 = dig_H6();

  const double c1 = t_fine_double - 76800.0;
  const double c2 = (h4 * 64.0 + h5 / 16384.0 * c1);
  const double c3 = (h2 / 65536.0 * (1.0 + h6 / 67108864.0 * c1 * (1.0 + h3 / 67108864.0 * c1)));
  const double c4 = c2 * c3;
  
  // H = x - dig_H1 / 524288.0 * pow(x, 2)
  // Where x = ((adcH * c3) - c4)
  // Therefore
  // -1 * dig_H1 / 524288.0 * pow(x, 2) + x - H = 0;
  
  const double a = -1.0 * h1 / 524288.0;
  const double b = 1.0;
  const double c = -1.0 * H;
  const double x = solve_quadratic(a, b, c, Root::Min);
  
  const int32_t adc_H = (x + c4) / c3;

  Registers[BME280_REGISTER_HUMIDDATA] = adc_H >> 8;
  Registers[BME280_REGISTER_HUMIDDATA + 1] = adc_H;
}

void on_wire_receive(int numBytes) {
  Serial.println("onRequestHandler received data");
  int byteCount = 0;

  while (Wire.available()) {
    byte rxByte = Wire.read();

    // By convention, (see BME280 datasheet) the first bit of a pair (of bits) recieved by the BME280 from a controller
    // is the address of the register to write. The "second" bit of a pair is the 
    // value to write. If only a single bit is received, then we should expect a read
    // request. (See onRequestHandler)
    if((byteCount % 2) == 0) {
      Serial.print("Setting Address ");
      Serial.println(rxByte, HEX);
      Address = rxByte;
    } else {
      // TODO Protect read only registers
      Serial.print("Setting Register at Address ");
      Serial.print(Address, HEX);
      Serial.print(" to ");
      Serial.println(rxByte, HEX);
      Registers[Address] = rxByte;
    }

    byteCount++;
  }
}

void on_wire_request(void) {
  Serial.print("onReceiveHandler got a request and the Address is currently ");
  Serial.print(Address, HEX);
  Serial.println(".");

  // The Wire API does not tell us how many bytes were requested
  // so the fastest and safest thing to do is to fill the buffer with the entire register
  // Careful, an ESP32 I2C buffer is 32 bytes,
  // but a Raspberry Pi Pico is 1024. In other words a Pico can easily buffer the entire Register
  // If this is running on an ESP32 then adjust code to buffer only 32 bytes
  if(Address < RegisterSize) {
    Wire.write(Registers + Address, RegisterSize - Address);
  }
}

// This is a raw dump of data from a real BME280
// According to the datasheet the first register address is actually 0x88,
// but this emulator is using a byte array of size 256, so initRegisters will
// initialize the entire byte array.
void init_registers() {
  Registers[0x00] = 0x0;
  Registers[0x01] = 0x0;
  Registers[0x02] = 0x0;
  Registers[0x03] = 0x0;
  Registers[0x04] = 0x0;
  Registers[0x05] = 0x0;
  Registers[0x06] = 0x0;
  Registers[0x07] = 0x0;
  Registers[0x08] = 0x0;
  Registers[0x09] = 0x0;
  Registers[0x0A] = 0x0;
  Registers[0x0B] = 0x0;
  Registers[0x0C] = 0x0;
  Registers[0x0D] = 0x0;
  Registers[0x0E] = 0x0;
  Registers[0x0F] = 0x0;
  Registers[0x10] = 0x0;
  Registers[0x11] = 0x0;
  Registers[0x12] = 0x0;
  Registers[0x13] = 0x0;
  Registers[0x14] = 0x0;
  Registers[0x15] = 0x0;
  Registers[0x16] = 0x0;
  Registers[0x17] = 0x0;
  Registers[0x18] = 0x0;
  Registers[0x19] = 0x0;
  Registers[0x1A] = 0x0;
  Registers[0x1B] = 0x0;
  Registers[0x1C] = 0x0;
  Registers[0x1D] = 0x0;
  Registers[0x1E] = 0x0;
  Registers[0x1F] = 0x0;
  Registers[0x20] = 0x0;
  Registers[0x21] = 0x0;
  Registers[0x22] = 0x0;
  Registers[0x23] = 0x0;
  Registers[0x24] = 0x0;
  Registers[0x25] = 0x0;
  Registers[0x26] = 0x0;
  Registers[0x27] = 0x0;
  Registers[0x28] = 0x0;
  Registers[0x29] = 0x0;
  Registers[0x2A] = 0x0;
  Registers[0x2B] = 0x0;
  Registers[0x2C] = 0x0;
  Registers[0x2D] = 0x0;
  Registers[0x2E] = 0x0;
  Registers[0x2F] = 0x0;
  Registers[0x30] = 0x0;
  Registers[0x31] = 0x0;
  Registers[0x32] = 0x0;
  Registers[0x33] = 0x0;
  Registers[0x34] = 0x0;
  Registers[0x35] = 0x0;
  Registers[0x36] = 0x0;
  Registers[0x37] = 0x0;
  Registers[0x38] = 0x0;
  Registers[0x39] = 0x0;
  Registers[0x3A] = 0x0;
  Registers[0x3B] = 0x0;
  Registers[0x3C] = 0x0;
  Registers[0x3D] = 0x0;
  Registers[0x3E] = 0x0;
  Registers[0x3F] = 0x0;
  Registers[0x40] = 0x0;
  Registers[0x41] = 0x0;
  Registers[0x42] = 0x0;
  Registers[0x43] = 0x0;
  Registers[0x44] = 0x0;
  Registers[0x45] = 0x0;
  Registers[0x46] = 0x0;
  Registers[0x47] = 0x0;
  Registers[0x48] = 0x0;
  Registers[0x49] = 0x0;
  Registers[0x4A] = 0x0;
  Registers[0x4B] = 0x0;
  Registers[0x4C] = 0x0;
  Registers[0x4D] = 0x0;
  Registers[0x4E] = 0x0;
  Registers[0x4F] = 0x0;
  Registers[0x50] = 0x0;
  Registers[0x51] = 0x0;
  Registers[0x52] = 0x0;
  Registers[0x53] = 0x0;
  Registers[0x54] = 0x0;
  Registers[0x55] = 0x0;
  Registers[0x56] = 0x0;
  Registers[0x57] = 0x0;
  Registers[0x58] = 0x0;
  Registers[0x59] = 0x0;
  Registers[0x5A] = 0x0;
  Registers[0x5B] = 0x0;
  Registers[0x5C] = 0x0;
  Registers[0x5D] = 0x0;
  Registers[0x5E] = 0x0;
  Registers[0x5F] = 0x0;
  Registers[0x60] = 0x0;
  Registers[0x61] = 0x0;
  Registers[0x62] = 0x0;
  Registers[0x63] = 0x0;
  Registers[0x64] = 0x0;
  Registers[0x65] = 0x0;
  Registers[0x66] = 0x0;
  Registers[0x67] = 0x0;
  Registers[0x68] = 0x0;
  Registers[0x69] = 0x0;
  Registers[0x6A] = 0x0;
  Registers[0x6B] = 0x0;
  Registers[0x6C] = 0x0;
  Registers[0x6D] = 0x0;
  Registers[0x6E] = 0x0;
  Registers[0x6F] = 0x0;
  Registers[0x70] = 0x0;
  Registers[0x71] = 0x0;
  Registers[0x72] = 0x0;
  Registers[0x73] = 0x0;
  Registers[0x74] = 0x0;
  Registers[0x75] = 0x0;
  Registers[0x76] = 0x0;
  Registers[0x77] = 0x0;
  Registers[0x78] = 0x0;
  Registers[0x79] = 0x0;
  Registers[0x7A] = 0x0;
  Registers[0x7B] = 0x0;
  Registers[0x7C] = 0x0;
  Registers[0x7D] = 0x0;
  Registers[0x7E] = 0x0;
  Registers[0x7F] = 0x0;
  Registers[0x80] = 0x8D;
  Registers[0x81] = 0x71;
  Registers[0x82] = 0x89;
  Registers[0x83] = 0x6B;
  Registers[0x84] = 0x9E;
  Registers[0x85] = 0x44;
  Registers[0x86] = 0xF5;
  Registers[0x87] = 0x6;
  Registers[0x88] = 0x26;
  Registers[0x89] = 0x6E;
  Registers[0x8A] = 0x3;
  Registers[0x8B] = 0x67;
  Registers[0x8C] = 0x32;
  Registers[0x8D] = 0x0;
  Registers[0x8E] = 0xA0;
  Registers[0x8F] = 0x8E;
  Registers[0x90] = 0x5A;
  Registers[0x91] = 0xD6;
  Registers[0x92] = 0xD0;
  Registers[0x93] = 0xB;
  Registers[0x94] = 0xA;
  Registers[0x95] = 0x1E;
  Registers[0x96] = 0xDB;
  Registers[0x97] = 0xFF;
  Registers[0x98] = 0xF9;
  Registers[0x99] = 0xFF;
  Registers[0x9A] = 0xAC;
  Registers[0x9B] = 0x26;
  Registers[0x9C] = 0xA;
  Registers[0x9D] = 0xD8;
  Registers[0x9E] = 0xBD;
  Registers[0x9F] = 0x10;
  Registers[0xA0] = 0x0;
  Registers[0xA1] = 0x4B;
  Registers[0xA2] = 0xFA;
  Registers[0xA3] = 0x0;
  Registers[0xA4] = 0x0;
  Registers[0xA5] = 0x0;
  Registers[0xA6] = 0x0;
  Registers[0xA7] = 0x0;
  Registers[0xA8] = 0x0;
  Registers[0xA9] = 0x0;
  Registers[0xAA] = 0x0;
  Registers[0xAB] = 0x0;
  Registers[0xAC] = 0x33;
  Registers[0xAD] = 0x0;
  Registers[0xAE] = 0x0;
  Registers[0xAF] = 0xC0;
  Registers[0xB0] = 0x0;
  Registers[0xB1] = 0x54;
  Registers[0xB2] = 0x0;
  Registers[0xB3] = 0x0;
  Registers[0xB4] = 0x0;
  Registers[0xB5] = 0x0;
  Registers[0xB6] = 0x60;
  Registers[0xB7] = 0x2;
  Registers[0xB8] = 0x0;
  Registers[0xB9] = 0x1;
  Registers[0xBA] = 0xFF;
  Registers[0xBB] = 0xFF;
  Registers[0xBC] = 0x1F;
  Registers[0xBD] = 0x60;
  Registers[0xBE] = 0x3;
  Registers[0xBF] = 0x0;
  Registers[0xC0] = 0x0;
  Registers[0xC1] = 0x0;
  Registers[0xC2] = 0x0;
  Registers[0xC3] = 0xFF;
  Registers[0xC4] = 0x0;
  Registers[0xC5] = 0x0;
  Registers[0xC6] = 0x0;
  Registers[0xC7] = 0x0;
  Registers[0xC8] = 0x0;
  Registers[0xC9] = 0x0;
  Registers[0xCA] = 0x0;
  Registers[0xCB] = 0x0;
  Registers[0xCC] = 0x0;
  Registers[0xCD] = 0x0;
  Registers[0xCE] = 0x0;
  Registers[0xCF] = 0x0;
  Registers[0xD0] = 0x60;
  Registers[0xD1] = 0x0;
  Registers[0xD2] = 0x0;
  Registers[0xD3] = 0x0;
  Registers[0xD4] = 0x0;
  Registers[0xD5] = 0x0;
  Registers[0xD6] = 0x0;
  Registers[0xD7] = 0x0;
  Registers[0xD8] = 0x0;
  Registers[0xD9] = 0x0;
  Registers[0xDA] = 0x0;
  Registers[0xDB] = 0x0;
  Registers[0xDC] = 0x0;
  Registers[0xDD] = 0x0;
  Registers[0xDE] = 0x0;
  Registers[0xDF] = 0x0;
  Registers[0xE0] = 0x0;
  Registers[0xE1] = 0x73;
  Registers[0xE2] = 0x1;
  Registers[0xE3] = 0x0;
  Registers[0xE4] = 0x12;
  Registers[0xE5] = 0x29;
  Registers[0xE6] = 0x3;
  Registers[0xE7] = 0x1E;
  Registers[0xE8] = 0xCA;
  Registers[0xE9] = 0x41;
  Registers[0xEA] = 0xFF;
  Registers[0xEB] = 0xFF;
  Registers[0xEC] = 0xFF;
  Registers[0xED] = 0xFF;
  Registers[0xEE] = 0xFF;
  Registers[0xEF] = 0xFF;
  Registers[0xF0] = 0xFF;
  Registers[0xF1] = 0x0;
  Registers[0xF2] = 0x0;
  Registers[0xF3] = 0x0;
  Registers[0xF4] = 0x0;
  Registers[0xF5] = 0x0;
  Registers[0xF6] = 0x0;
  Registers[0xF7] = 0x80;
  Registers[0xF8] = 0x0;
  Registers[0xF9] = 0x0;
  Registers[0xFA] = 0x80;
  Registers[0xFB] = 0x0;
  Registers[0xFC] = 0x0;
  Registers[0xFD] = 0x80;
  Registers[0xFE] = 0x0;
  Registers[0xFF] = 0x80;

  // Temporary hack to put in some reasonable pressure data
  // This should be close to 1 atm, but not exact, because the value
  // is temperature dependent.
  int32_t adc_P_value = 0x539BA0;
  Registers[BME280_REGISTER_PRESSUREDATA] = adc_P_value >> 16;
  Registers[BME280_REGISTER_PRESSUREDATA + 1] = adc_P_value >> 8;
  Registers[BME280_REGISTER_PRESSUREDATA + 2] = adc_P_value;
}

} // namespace bme
