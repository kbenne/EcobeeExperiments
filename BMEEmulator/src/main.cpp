#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include "registers.hpp"

byte Address;
WebServer server(80);
constexpr char ssid[] = EMBEDDED_SSID; //  your network SSID
constexpr char pass[] = EMBEDDED_PASS; //  your network password

// Calibration terms are composed of two bytes.
// The first register is the least significant digit.

// T1 temperature calibration term
uint16_t dig_T1() {
   return uint16_t((Registers[BME280_REGISTER_DIG_T1 + 1]) << 8) | uint16_t((Registers[BME280_REGISTER_DIG_T1]));
}

// T2 temperature calibration term
int16_t dig_T2() {
   return int16_t((Registers[BME280_REGISTER_DIG_T2 + 1]) << 8) | int16_t((Registers[BME280_REGISTER_DIG_T2]));
}

// T3 temperature calibration term
int16_t dig_T3() {
   return int16_t((Registers[BME280_REGISTER_DIG_T3 + 1]) << 8) | int16_t((Registers[BME280_REGISTER_DIG_T3]));
}

// Return adc_T (the raw sensor reading) given temperature T in degC
// This solves the quadratic equation given in Appendix 8.1 in the BME datasheet
int32_t adc_T(const double &T) {
  const double T1 = dig_T1();
  const double T2 = dig_T2();
  const double T3 = dig_T3();

  const double a = T3 / pow(131072.0, 2.0);
  const double b = (T2 / 16384.0) - (2.0 * T1 * T3 / 131072.0 / 8192.0);
  const double c = (pow(T1 / 8192.0, 2.0) * T3) - (T1 * T2 / 1024.0) - (T * 5120.0);

  const double discriminant = pow(b, 2.0) - 4.0 * a * c;

  double result;

  if ( discriminant > 0) {
    const double square_root = sqrt(discriminant);
    result = fmax((-b + square_root) / (2.0 * a), (-b - square_root) / (2.0 * a));
  } else {
    result = -b / (2.0 * a);
  }

  // The four least significant digits are not used by the TEMPDATA register
  // so shift 4 digits to the left
  return (int32_t)result << 4;
}

void set_T(const double &T) {
  const int32_t adc_T_value = adc_T(T);

  // BME280_REGISTER_TEMPDATA is the most significant digit
  Registers[BME280_REGISTER_TEMPDATA] = adc_T_value >> 16;
  Registers[BME280_REGISTER_TEMPDATA + 1] = adc_T_value >> 8;
  Registers[BME280_REGISTER_TEMPDATA + 2] = adc_T_value;
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

void http_api_endpoint() {
  const auto method = server.method();
  if (method == HTTP_GET) {
    // Need to either cash the current T, or implement the calibration function
    // to get it back out of the adc_T.
    server.send(200, "text/plain", "Not implemented"); 
  } else if (method == HTTP_PUT) {
    if (server.hasArg("temperature")) {
      const auto value = server.arg("temperature");
      const double new_temp = value.toDouble();
      set_T(new_temp);
      server.send(200); 
    }
    server.send(400, "text/plain", "Bad request");
  } else {
    server.send(405, "text/plain", "Method not allowed");
  }
}

void http_not_found_endpoint(){
  server.send(404, "text/plain", "Not found");
}

void setup() {
  initRegisters();

  // Set T in degress C and the emulator will communicate this value
  // over I2C as if connected to a normal BME280
  set_T(28.13);

  Wire.onReceive(on_wire_receive);
  Wire.onRequest(on_wire_request);

  const auto status = WiFi.begin(ssid, pass);
  if ( status != WL_CONNECTED) {
    Serial.println("Could not connect to wireless network");
  } else {
    const auto ip = WiFi.localIP();
    Serial.print("IP address is: ");
    Serial.println(ip);
  }

  delay(1000);

  server.on("/api", http_api_endpoint);
  server.onNotFound(http_not_found_endpoint);
  server.begin();
  Serial.println("HTTP server started");

  Wire.begin(0x76); // This is the I2C address of a BME280
  Serial.begin(9600);
}

void loop() {
  server.handleClient();
}
