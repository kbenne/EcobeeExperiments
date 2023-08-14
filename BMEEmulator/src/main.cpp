#include <Arduino.h>
#include <Wire.h>

// Communication with a BME280 amounts to reading and writing some registers
enum {
  BME280_REGISTER_DIG_T1 = 0x88,
  BME280_REGISTER_DIG_T2 = 0x8A,
  BME280_REGISTER_DIG_T3 = 0x8C,

  BME280_REGISTER_DIG_P1 = 0x8E,
  BME280_REGISTER_DIG_P2 = 0x90,
  BME280_REGISTER_DIG_P3 = 0x92,
  BME280_REGISTER_DIG_P4 = 0x94,
  BME280_REGISTER_DIG_P5 = 0x96,
  BME280_REGISTER_DIG_P6 = 0x98,
  BME280_REGISTER_DIG_P7 = 0x9A,
  BME280_REGISTER_DIG_P8 = 0x9C,
  BME280_REGISTER_DIG_P9 = 0x9E,

  BME280_REGISTER_DIG_H1 = 0xA1,
  BME280_REGISTER_DIG_H2 = 0xE1,
  BME280_REGISTER_DIG_H3 = 0xE3,
  BME280_REGISTER_DIG_H4 = 0xE4,
  BME280_REGISTER_DIG_H5 = 0xE5,
  BME280_REGISTER_DIG_H6 = 0xE7,

  BME280_REGISTER_CHIPID = 0xD0,
  BME280_REGISTER_VERSION = 0xD1,
  BME280_REGISTER_SOFTRESET = 0xE0,

  BME280_REGISTER_CAL26 = 0xE1, // R calibration stored in 0xE1-0xF0

  BME280_REGISTER_CONTROLHUMID = 0xF2,
  BME280_REGISTER_STATUS = 0xF3,
  BME280_REGISTER_CONTROL = 0xF4,
  BME280_REGISTER_CONFIG = 0xF5,
  BME280_REGISTER_PRESSUREDATA = 0xF7,
  BME280_REGISTER_TEMPDATA = 0xFA,
  BME280_REGISTER_HUMIDDATA = 0xFD
};

constexpr int RegisterSize = 256;
byte Registers[RegisterSize];
byte Address;

void initRegisters() {
  Registers[BME280_REGISTER_CHIPID] = 0x60; // The BME280 CHIPID should be 0x60
  Registers[BME280_REGISTER_SOFTRESET] = 0x00; // Reset doesn't (yet) do anything to the emulator
  Registers[BME280_REGISTER_STATUS] = 0x00; // Status 0 means the emulator is always ready
}

// bool Adafruit_BME280::init() {
//   // check if sensor, i.e. the chip ID is correct
//   _sensorID = read8(BME280_REGISTER_CHIPID);
//   if (_sensorID != 0x60)
//     return false;
//
//   // reset the device using soft-reset
//   // this makes sure the IIR is off, etc.
//   write8(BME280_REGISTER_SOFTRESET, 0xB6);
//
//   // wait for chip to wake up.
//   delay(10);
//
//   // if chip is still reading calibration, delay
//   while (isReadingCalibration())
//     delay(10);
//
//   readCoefficients(); // read trimming parameters, see DS 4.2.2
//
//   setSampling(); // use defaults
//
//   delay(100);
//
//   return true;
// }

void onReceiveHandler(int numBytes) {
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
      Address = 0x00;
    }

    byteCount++;
  }
}

void onRequestHandler(void) {
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

void setup() {
  initRegisters();

  Wire.onReceive(onReceiveHandler);
  Wire.onRequest(onRequestHandler);

  Wire.begin(0x76);
  Serial.begin(9600);

  Serial.println();
}

void loop() {
    delay(10);
}
