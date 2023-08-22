#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include "registers.hpp"

byte Address;
WebServer server(80);
constexpr char ssid[] = EMBEDDED_SSID; //  your network SSID
constexpr char pass[] = EMBEDDED_PASS; //  your network password


// Set the raw sensor reading (adc_T) given temperature T in degC
// This solves the quadratic equation given in Appendix 8.1 in the BME datasheet
void set_T(const double &T) {
  // This is an ecobee specific adjustment that should get the display to
  // nearly match the value provided by the sensor
  double T_adjusted = (T + 4.3766) / 0.9861;
  // This is the function provided by the datasheet
  TemperatureRegister = (T_adjusted + 45) * (pow(2, 16) - 1) / 175;
}

double get_T(void) {
  double T_adjusted = TemperatureRegister * 175 / (pow(2, 16) - 1) - 45;
  return T_adjusted * 0.9861 - 4.3766;
}

void set_H(const double &H) {
  double H_adjusted = H; // TODO need a humidity correction curve as function of H AND T
  HumidityRegister = (H_adjusted + 6) * (pow(2, 16) - 1) / 125;
}

int16_t Command;

//#define SHT4x_READSERIAL 0x89 /**< Read Out of Serial Register */
//#define SHT4x_SOFTRESET 0x94  /**< Soft Reset */

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

    while (Wire.available()) {
      if (index > 0) {
        Serial.println("Unexpected data received from controller");
        break;
      }
      Command = Wire.read();
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
    Wire.write(data, 6);
  } else if(Command == SHT4x_READSERIAL) {
    Wire.write(SerialNumber, 6);
  }

  Serial.println("End on_wire_request");
}

// TODO: Reduce code duplication in endpoints
void http_temperature_endpoint() {
  const auto method = server.method();
  if (method == HTTP_GET) {
    // Need to either cash the current T, or implement the calibration function
    // to get it back out of the adc_T.
    server.send(200, "text/plain", "Not implemented"); 
  } else if (method == HTTP_PUT) {
    if (server.hasArg("value")) {
      const auto value = server.arg("value");
      const double new_temp = value.toDouble();
      set_T(new_temp);
      server.send(200); 
    }
    server.send(400, "text/plain", "Bad request");
  } else {
    server.send(405, "text/plain", "Method not allowed");
  }
}

void http_humidity_endpoint() {
  const auto method = server.method();
  if (method == HTTP_GET) {
    // Need to either cash the current T, or implement the calibration function
    // to get it back out of the adc_T.
    server.send(200, "text/plain", "Not implemented"); 
  } else if (method == HTTP_PUT) {
    if (server.hasArg("value")) {
      const auto value = server.arg("value");
      const double new_h = value.toDouble();
      set_H(new_h);
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

void init_serial_number() {
  SerialNumber[0] = 0x0E;
  SerialNumber[1] = 0xFE;
  SerialNumber[2] = crc8(SerialNumber, 2);
  SerialNumber[3] = 0x7F;
  SerialNumber[4] = 0xBF;
  SerialNumber[5] = crc8(SerialNumber + 3, 2);
}

void setup() {
  Serial.begin(9600);

  init_serial_number();
  // Set T in degress C and the emulator will communicate this value
  // over I2C as if connected to a normal BME280
  set_T(22.00);
  set_H(50.0);

  Wire.onReceive(on_wire_receive);
  Wire.onRequest(on_wire_request);
  Wire.begin(SHT4x_DEFAULT_ADDR); // This is the I2C address of a BME280
  Serial.println("I2C Wire protocol started");

  const auto status = WiFi.begin(ssid, pass);
  if ( status != WL_CONNECTED) {
    Serial.println("Could not connect to wireless network");
  } else {
    const auto ip = WiFi.localIP();
    Serial.print("IP address is: ");
    Serial.println(ip);
  }

  server.on("/api/temperature", http_temperature_endpoint);
  server.on("/api/humidity", http_humidity_endpoint);
  server.onNotFound(http_not_found_endpoint);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  //delay(20);

  server.handleClient();
}
