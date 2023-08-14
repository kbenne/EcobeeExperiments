#include <Arduino.h>
#include <Wire.h>
#include "registers.hpp"

byte Address;

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
