#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include "registers.hpp"

//byte Address;
//WebServer server(80);
//constexpr char ssid[] = EMBEDDED_SSID; //  your network SSID
//constexpr char pass[] = EMBEDDED_PASS; //  your network password


// Set the raw sensor reading (adc_T) given temperature T in degC
// This solves the quadratic equation given in Appendix 8.1 in the BME datasheet
void set_T(const double &T) {
  TemperatureRegister = (T + 45) * (pow(2, 16) - 1) / 175;
}

void set_H(const double &H) {
  HumidityRegister = H * (pow(2, 16) - 1) / 100;
}

int16_t Command;

bool is_measure_command(int16_t command) {
  return (command == SHT31_MEAS_HIGHREP_STRETCH) ||
         (command == SHT31_MEAS_MEDREP_STRETCH)  ||
         (command == SHT31_MEAS_LOWREP_STRETCH)  ||
         (command == SHT31_MEAS_HIGHREP)         ||
         (command == SHT31_MEAS_MEDREP)          ||
         (command == SHT31_MEAS_LOWREP);
}

void clear_status() {
  // 0 is normal (all systems go) status
  StatusRegister = 0x0;
}

void reset_status() {
  // See datasheet for status register bit assignments
  // On startup the "reset" bit and the "alert pending" bits are set
  StatusRegister = 0x8008;
}

void enable_heater() {
  StatusRegister = StatusRegister | SHT31_REG_HEATER_MASK;
}

void disable_heater() {
  StatusRegister = StatusRegister & ~SHT31_REG_HEATER_MASK;
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
  if (command == SHT31_CLEARSTATUS) {
    clear_status();
  } else if (command == SHT31_SOFTRESET) {
    reset_status();
  } else if (command == SHT31_HEATEREN) {
    enable_heater(); 
  } else if (command == SHT31_HEATERDIS) {
    disable_heater();
  }
}

void on_wire_receive(int num_bytes) {
  Serial.println("Begin on_wire_receive");

  if (num_bytes == 2) {
    byte data[2];
    int index = 0;

    while (Wire.available()) {
      if (index > 1) {
        Serial.println("Unexpected data received from controller");
        break;
      }
      data[index] = Wire.read();
      index++; 
    }

    // Second byte in the command is the last significant digit
    Command = int16_t(data[0] << 8) | int16_t(data[1]);
    eval_command(Command);

    Serial.print("Received command ");
    Serial.println(Command, HEX);
  } else {
    Serial.print("Two bytes were expected, but received ");
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

  if (Command == SHT31_READSTATUS) {
    byte data[3];
    data[0] = StatusRegister >> 8;
    data[1] = StatusRegister;
    data[2] = crc8(data, 2);
    Wire.write(data, 3);
  } else if (is_measure_command(Command)) {
    byte data[6];
    data[0] = TemperatureRegister >> 8;
    data[1] = TemperatureRegister;
    data[2] = crc8(data, 2);
    data[3] = HumidityRegister >> 8;
    data[4] = HumidityRegister;
    data[5] = crc8(data + 3, 2);
    Wire.write(data, 6);
  }

  Serial.println("End on_wire_request");
}

//void http_api_endpoint() {
//  const auto method = server.method();
//  if (method == HTTP_GET) {
//    // Need to either cash the current T, or implement the calibration function
//    // to get it back out of the adc_T.
//    server.send(200, "text/plain", "Not implemented"); 
//  } else if (method == HTTP_PUT) {
//    if (server.hasArg("temperature")) {
//      const auto value = server.arg("temperature");
//      const double new_temp = value.toDouble();
//      set_T(new_temp);
//      server.send(200); 
//    }
//    server.send(400, "text/plain", "Bad request");
//  } else {
//    server.send(405, "text/plain", "Method not allowed");
//  }
//}
//
//void http_not_found_endpoint(){
//  server.send(404, "text/plain", "Not found");
//}

void setup() {
  Serial.begin(9600);

  // Set T in degress C and the emulator will communicate this value
  // over I2C as if connected to a normal BME280
  reset_status();
  set_T(28.13);
  set_H(50.0);

  Wire.onReceive(on_wire_receive);
  Wire.onRequest(on_wire_request);
  Wire.begin(SHT31_DEFAULT_ADDR); // This is the I2C address of a BME280
  Serial.println("I2C Wire protocol started");

  //const auto status = WiFi.begin(ssid, pass);
  //if ( status != WL_CONNECTED) {
  //  Serial.println("Could not connect to wireless network");
  //} else {
  //  const auto ip = WiFi.localIP();
  //  Serial.print("IP address is: ");
  //  Serial.println(ip);
  //}

  //server.on("/api", http_api_endpoint);
  //server.onNotFound(http_not_found_endpoint);
  //server.begin();
  //Serial.println("HTTP server started");
}

void loop() {
  delay(2000);

  //server.handleClient();
}
