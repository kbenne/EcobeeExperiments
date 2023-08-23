#include "sht.hpp"
#include "Wire.h"

namespace sht {

void init() {
  init_serial_number();

  set_T(22.0);
  set_H(50.0);

  Wire1.setSDA(14);
  Wire1.setSCL(15);

  Wire1.onReceive(on_wire_receive);
  Wire1.onRequest(on_wire_request);
}

void begin() {
  Wire1.begin(SHT4x_DEFAULT_ADDR); // This is the I2C address of a BME280
  Serial.println("SHT emulator started with I2C address 0x44 on Wire1");
}

// Set the raw sensor reading (adc_T) given temperature T in degC
// This solves the quadratic equation given in Appendix 8.1 in the BME datasheet
void set_T(const double &T) {
  // This is an ecobee specific adjustment that should get the display to
  // nearly match the value provided by the sensor
  double T_adjusted = T; //(T + 4.3766) / 0.9861;
  // This is the function provided by the datasheet
  TemperatureRegister = (T_adjusted + 45) * (pow(2, 16) - 1) / 175;
}

double get_T(void) {
  double T_adjusted = TemperatureRegister * 175 / (pow(2, 16) - 1) - 45;
  return T_adjusted; // * 0.9861 - 4.3766;
}

void set_H(const double &H) {
  double H_adjusted = H; // TODO need a humidity correction curve as function of H AND T
  HumidityRegister = (H_adjusted + 6) * (pow(2, 16) - 1) / 125;
}

bool is_measure_command(int16_t command) {
  return (command == SHT4x_NOHEAT_HIGHPRECISION) ||
         (command == SHT4x_NOHEAT_MEDPRECISION) ||
         (command == SHT4x_NOHEAT_LOWPRECISION) ||
         (command == SHT4x_HIGHHEAT_1S) ||
         (command == SHT4x_HIGHHEAT_100MS) ||
         (command == SHT4x_MEDHEAT_1S) ||
         (command == SHT4x_MEDHEAT_100MS) ||
         (command == SHT4x_LOWHEAT_1S) ||
         (command == SHT4x_LOWHEAT_100MS);

}

uint8_t crc8(const uint8_t *data, int len) {
  /*
   *
   * CRC-8 formula from page 14 of SHT spec pdf
   *
   * Test data 0xBE, 0xEF should yield 0x92
   *
   * Initialization data 0xFF
   * Polynomial 0x31 (x8 + x5 +x4 +1)
   * Final XOR 0x00
   */

  const uint8_t POLYNOMIAL(0x31);
  uint8_t crc(0xFF);

  for (int j = len; j; --j) {
    crc ^= *data++;

    for (int i = 8; i; --i) {
      crc = (crc & 0x80) ? (crc << 1) ^ POLYNOMIAL : (crc << 1);
    }
  }
  return crc;
}

void eval_command(int16_t command) {
  // Some commands setup an expected read request, which require no action. (until the read request)
  // Other commands are a command to write data, and they should be handled here.
  // Update: Actually for SHT4x (unlike SHT3x), there are no write commands, beyond reset
  if (command == SHT4x_SOFTRESET) {
    Serial.println("SOFTRESET Issued");
  }
}

void on_wire_receive(int num_bytes) {
  Serial.println("Begin on_wire_receive");

  if (num_bytes == 1) {
    int index = 0;

    while (Wire1.available()) {
      if (index > 0) {
        Serial.println("Unexpected data received from controller");
        break;
      }
      Command = Wire1.read();
      index++; 
    }

    // Second byte in the command is the last significant digit
    eval_command(Command);

    Serial.print("Received command ");
    Serial.println(Command, HEX);
  } else {
    Serial.print("One byte was expected, but received ");
    Serial.print(num_bytes, 0);
    Serial.println(" bytes from controller");
    Command = 0x0;
  }

  Serial.println("End on_wire_receive");
}

void on_wire_request(void) {
  Serial.println("Begin on_wire_request");
  Serial.print("Current Command: ");
  Serial.println(Command, HEX);

  if (is_measure_command(Command)) {
    byte data[6];
    data[0] = TemperatureRegister >> 8;
    data[1] = TemperatureRegister;
    data[2] = crc8(data, 2);
    data[3] = HumidityRegister >> 8;
    data[4] = HumidityRegister;
    data[5] = crc8(data + 3, 2);
    Wire1.write(data, 6);
  } else if(Command == SHT4x_READSERIAL) {
    Wire1.write(SerialNumber, 6);
  }

  Serial.println("End on_wire_request");
}

void init_serial_number() {
  SerialNumber[0] = 0x0E;
  SerialNumber[1] = 0xFE;
  SerialNumber[2] = crc8(SerialNumber, 2);
  SerialNumber[3] = 0x7F;
  SerialNumber[4] = 0xBF;
  SerialNumber[5] = crc8(SerialNumber + 3, 2);
}


} // namespace sht
