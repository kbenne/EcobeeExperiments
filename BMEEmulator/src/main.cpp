#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "bme.hpp"

WebServer server(80);
constexpr char ssid[] = EMBEDDED_SSID; //  your network SSID
constexpr char pass[] = EMBEDDED_PASS; //  your network password

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
      bme::set_T(new_temp);
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
      bme::set_H(new_h);
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

  bme::init();
  bme::begin();

  //Wire.begin(0x76); // This is the I2C address of a BME280
  //Serial.println("I2C Wire protocol started");

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
