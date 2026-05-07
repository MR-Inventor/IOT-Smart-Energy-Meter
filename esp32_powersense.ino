/*
  esp32_powersense.ino — PowerSense WiFi Energy Monitor
  ======================================================
  Connects ESP32 to your WiFi network.
  Serves sensor data as JSON at:   http://<ESP32_IP>/data
  run.py on your PC polls this URL every 2 seconds.

  Hardware:
    - ACS712 current sensor  → GPIO 34
    - Voltage divider sensor → GPIO 36
    - LCD 16x2 I2C (0x27)   → SDA=21, SCL=22

  Change WIFI_SSID and WIFI_PASS below, then flash to ESP32.
*/

#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ============================================================
//  CHANGE THESE TO YOUR WiFi CREDENTIALS
// ============================================================
const char* WIFI_SSID = "Harshvardhan";   // <--- your WiFi SSID
const char* WIFI_PASS = "12345678";   // <--- your WiFi password
// ============================================================

LiquidCrystal_I2C lcd(0x27, 16, 2);
WebServer server(80);

// Pins
#define CURRENT_PIN 34
#define VOLTAGE_PIN 36

// Constants
float vRef        = 3.3;
float adcMax      = 4095.0;
float sensitivity = 0.185;   // ACS712 5A model
float costPerUnit = 8.0;     // Rs per kWh

// Calibration offsets
float currentOffset = 0;
float voltageOffset = 0;

// Energy tracking
float energy_kWh = 0;
unsigned long lastTime = 0;

// Latest readings (read by HTTP handler)
float gVoltage = 0, gCurrent = 0, gPower = 0, gCost = 0;

// LCD toggle
bool showVI = true;


// ============================================================
//  CALIBRATION
// ============================================================
void calibrate() {
  long cSum = 0, vSum = 0;
  Serial.println("Calibrating... keep no load connected");
  lcd.clear();
  lcd.print("Calibrating...");
  for (int i = 0; i < 1000; i++) {
    cSum += analogRead(CURRENT_PIN);
    vSum += analogRead(VOLTAGE_PIN);
    delay(1);
  }
  currentOffset = cSum / 1000.0;
  voltageOffset = vSum / 1000.0;
  Serial.println("Calibration done.");
}


// ============================================================
//  SENSORS
// ============================================================
float getCurrentRMS() {
  float sum = 0;
  for (int i = 0; i < 500; i++) {
    float adc  = analogRead(CURRENT_PIN);
    float volt = ((adc - currentOffset) / adcMax) * vRef;
    float curr = volt / sensitivity;
    sum += curr * curr;
  }
  float rms = sqrt(sum / 500.0);
  if (rms < 0.03) rms = 0;
  return rms;
}

float getVoltageRMS() {
  float sum = 0;
  for (int i = 0; i < 500; i++) {
    float adc  = analogRead(VOLTAGE_PIN);
    float volt = ((adc - voltageOffset) / adcMax) * vRef;
    sum += volt * volt;
  }
  float rms        = sqrt(sum / 500.0);
  float realVoltage = rms * 350;   // calibration factor — adjust if needed
  if (realVoltage < 10) realVoltage = 0;
  return realVoltage;
}


// ============================================================
//  HTTP HANDLER  — run.py polls this endpoint
// ============================================================
void handleData() {
  // Build JSON manually (no extra library needed)
  String json = "{";
  json += "\"voltage\":"  + String(gVoltage, 2)  + ",";
  json += "\"current\":"  + String(gCurrent, 3)  + ",";
  json += "\"power\":"    + String(gPower, 2)    + ",";
  json += "\"energy\":"   + String(energy_kWh, 6) + ",";
  json += "\"cost\":"     + String(gCost, 4);
  json += "}";

  server.sendHeader("Access-Control-Allow-Origin", "*");  // allow cross-origin from browser
  server.send(200, "application/json", json);
}

void handleRoot() {
  server.send(200, "text/plain",
    "PowerSense ESP32 running.\n"
    "Fetch data at: /data\n"
    "Example: http://" + WiFi.localIP().toString() + "/data"
  );
}


// ============================================================
//  SETUP
// ============================================================
void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();

  // --- WiFi connect ---
  lcd.print("Connecting WiFi");
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 30) {
    delay(500);
    Serial.print(".");
    tries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("ESP32 IP: ");
    Serial.println(WiFi.localIP());

    lcd.clear();
    lcd.print("WiFi OK!");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
    delay(3000);
  } else {
    Serial.println("\nWiFi FAILED — check SSID/password");
    lcd.clear();
    lcd.print("WiFi FAILED!");
    delay(3000);
  }

  // --- HTTP server routes ---
  server.on("/",     handleRoot);
  server.on("/data", handleData);
  server.begin();
  Serial.println("HTTP server started.");

  // --- Calibrate sensors ---
  calibrate();
  delay(1000);
  lcd.clear();
  lastTime = millis();
}


// ============================================================
//  LOOP
// ============================================================
void loop() {
  server.handleClient();   // handle incoming HTTP requests

  float current = getCurrentRMS();
  float voltage = getVoltageRMS();
  float power   = voltage * current;

  // Energy accumulation
  unsigned long now      = millis();
  float timeHours        = (now - lastTime) / 3600000.0;
  lastTime               = now;
  energy_kWh            += (power / 1000.0) * timeHours;
  float totalCost        = energy_kWh * costPerUnit;

  // Store globally for HTTP handler
  gVoltage = voltage;
  gCurrent = current;
  gPower   = power;
  gCost    = totalCost;

  // Serial monitor
  Serial.print("V: ");    Serial.print(voltage, 1);
  Serial.print("V  I: "); Serial.print(current, 3);
  Serial.print("A  P: "); Serial.print(power, 1);
  Serial.print("W  E: "); Serial.print(energy_kWh, 4);
  Serial.print("kWh  Rs:"); Serial.println(totalCost, 4);

  // LCD display (alternates between V/I and P/Cost)
  lcd.clear();
  if (showVI) {
    lcd.setCursor(0, 0);
    lcd.print("V:"); lcd.print(voltage, 1); lcd.print("V");
    lcd.setCursor(0, 1);
    lcd.print("I:"); lcd.print(current, 3); lcd.print("A");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("P:"); lcd.print(power, 0); lcd.print("W");
    lcd.setCursor(0, 1);
    lcd.print("Rs:"); lcd.print(totalCost, 2);
  }
  showVI = !showVI;

  delay(1500);
}