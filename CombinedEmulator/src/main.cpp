#include "bme.hpp"
#include "sht.hpp"
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// This is a combined BME and SHT emulator
// This used both Wire and Wire1 interfaces available on the Pico

WebServer server(80);
constexpr char ssid[] = "Biryani4Souls"; //  your network SSID
constexpr char pass[] = "Queen1991"; //  your network password
double T_store;
double H_store;

void set_T(const double &T) {
  T_store = T;
  double T_adjusted = (T_store + 4.3766) / 0.9861;

  sht::set_T(T_adjusted);
  bme::set_T(T_adjusted);
}

void set_H(const double &H) {
  H_store = H;

  constexpr double a = 0.740036139896326;
  constexpr double b = -0.0017671331702309168;
  constexpr double c = 0.0005783465707743796;
  constexpr double d = 0.05096062356332354;

  double H_adjusted = a * H_store + b * T_store + c * H_store * T_store + d;

  sht::set_H(H_adjusted);
  bme::set_H(H_adjusted);
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
  pinMode(0, INPUT_PULLDOWN);
  pinMode(1, INPUT_PULLDOWN);
  pinMode(2, INPUT_PULLDOWN);

  Serial.begin(9600);

  Serial1.setTX(12);  // Set TX pin to GPIO 12 
  Serial1.setRX(13);  // Set RX pin to GPIO 13 
  Serial1.begin(115200);

  sht::init();
  bme::init();

  set_T(20.0);
  set_H(0.0, 20.0);

  sht::begin();
  bme::begin();

  auto status = WL_DISCONNECTED;
  // Uncomment this line to connect to wifi
  // You must set the EMBEDDED_PASS and EMBEDDED_SSID environment variables before compiling
  // status = WiFi.begin(ssid, pass);

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

static unsigned long last_refresh_ = 0;
static const unsigned long refresh_interval_ = 3000;
static char buffer_[40];
static char io0_;
static char io1_;
static char io2_;

void handle_serial_input(HardwareSerial &serial) {
  if (serial.available() > 0) {
    JsonDocument doc;
    auto error = deserializeJson(doc, serial);

    if ( error ) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
    } else {
      const auto temperature = doc["temperature"];
      if (temperature.is<double>()) {
        set_T(temperature.as<double>());
      }
      const auto humidity = doc["humidity"];
      if (humidity.is<double>()) {
        set_H(humidity.as<double>());
      }
    }
  }
}

void loop() {
  server.handleClient();

  auto input0 = digitalRead(0);
  auto input1 = digitalRead(1);
  auto input2 = digitalRead(2);

  // This is the condition to flag for sending new input values to the client
  bool stale_inputs{false};

  // A timeout will trigger stale_inputs
  if ( millis() - last_refresh_ >= refresh_interval_ ) {
    stale_inputs = true;
  }

  // If any of the cached values change, that will also trigger stale_inputs
  if ( input0 != io0_ ||
       input1 != io1_ ||
       input2 != io2_
  ) {
    stale_inputs = true;
  }

  if ( stale_inputs ) {
    io0_ = input0;
    io1_ = input1;
    io2_ = input2;

    sprintf(buffer_, "{\"input0\": %d, \"input1\": %d, \"input2\": %d}",input0, input1, input2);

    Serial1.println(buffer_);

    last_refresh_ = millis();
    stale_inputs = false;
  }

  handle_serial_input(Serial1);
}
