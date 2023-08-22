#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include "registers.hpp"

byte Address;
WebServer server(80);
constexpr char ssid[] = EMBEDDED_SSID; //  your network SSID
constexpr char pass[] = EMBEDDED_PASS; //  your network password

// Many calibration terms are composed of two bytes.
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

// H1 humidity calibration term
uint8_t dig_H1() {
  return Registers[BME280_REGISTER_DIG_H1];
}

// H2 humidity calibration term
int16_t dig_H2() {
  return int16_t((Registers[BME280_REGISTER_DIG_H2 + 1]) << 8) | int16_t((Registers[BME280_REGISTER_DIG_H2]));
}

// H3 humidity calibration term
uint8_t dig_H3() {
  return Registers[BME280_REGISTER_DIG_H3];
}

// H4 humidity calibration term
int16_t dig_H4() {
  return ((int8_t)Registers[BME280_REGISTER_DIG_H4] << 4) | (Registers[BME280_REGISTER_DIG_H4 + 1] & 0xF);
}

// H5 humidity calibration term
int16_t dig_H5() {
  return ((int8_t)Registers[BME280_REGISTER_DIG_H5 + 1] << 4) | (Registers[BME280_REGISTER_DIG_H5] >> 4);
}

// H6 humidity calibration term
uint8_t dig_H6() {
  return Registers[BME280_REGISTER_DIG_H6];
}

enum class Root { Min, Max };

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

double solve_quadratic_min(const double &a, const double &b, const double &c) {
  // a * x ** 2 + b * x + c = 0, return max x
  const double discriminant = pow(b, 2.0) - 4.0 * a * c;

  if ( discriminant > 0) {
    const double square_root = sqrt(discriminant);
    return fmin((-b + square_root) / (2.0 * a), (-b - square_root) / (2.0 * a));
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

void setup() {
  Serial.begin(9600);

  initRegisters();

  // Temporary hack to put in some reasonable pressure data
  int32_t adc_P_value = 0x539BA0;
  Registers[BME280_REGISTER_PRESSUREDATA] = adc_P_value >> 16;
  Registers[BME280_REGISTER_PRESSUREDATA + 1] = adc_P_value >> 8;
  Registers[BME280_REGISTER_PRESSUREDATA + 2] = adc_P_value;

  // Set T in degress C and the emulator will communicate this value
  // over I2C as if connected to a normal BME280
  set_T(28.13);
  set_H(50.0);

  Wire.onReceive(on_wire_receive);
  Wire.onRequest(on_wire_request);
  Wire.begin(0x76); // This is the I2C address of a BME280
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
  server.handleClient();
}
