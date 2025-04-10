// --- Bibliotheken einbinden ---
#include <WiFi.h>
#include <WebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LittleFS.h>
#include <ArduinoOTA.h>
#include <ModbusTCPServer.h>
#include <ArduinoJson.h>

// --- Konstanten und Einstellungen ---
#define ONE_WIRE_BUS 4 // GPIO4 (D4) für DS18B20
#define NUM_SENSORS_MAX 10
#define CONFIG_FILE "/order.json"
#define INDEX_HTML "/index.html"

// --- Netzwerkdaten ---
const char* ssid = "FRITZ!Box Fon WLAN 7390";
const char* password = "9247083248452941";

// --- Globale Variablen ---
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
WebServer server(80);
ModbusTCPServer modbusServer;
DeviceAddress foundAddresses[NUM_SENSORS_MAX];
uint8_t sensorCount = 0;
uint8_t sensorOrder[NUM_SENSORS_MAX];
int16_t temps[NUM_SENSORS_MAX];

// --- Hilfsfunktionen ---
String addressToString(DeviceAddress addr) {
  char buffer[17];
  for (uint8_t i = 0; i < 8; i++) {
    sprintf(&buffer[i * 2], "%02X", addr[i]);
  }
  return String(buffer);
}

bool stringToAddress(String str, DeviceAddress &addr) {
  if (str.length() != 16) return false;
  for (uint8_t i = 0; i < 8; i++) {
    String byteStr = str.substring(i * 2, i * 2 + 2);
    addr[i] = (uint8_t) strtol(byteStr.c_str(), nullptr, 16);
  }
  return true;
}

void scanSensors() {
  sensors.begin();
  sensorCount = 0;
  while (oneWire.search(foundAddresses[sensorCount])) {
    sensorCount++;
    if (sensorCount >= NUM_SENSORS_MAX) break;
  }
  oneWire.reset_search();
  for (uint8_t i = 0; i < sensorCount; i++) {
    sensors.setResolution(foundAddresses[i], 12);
  }
}

void loadOrder() {
  if (!LittleFS.exists(CONFIG_FILE)) {
    for (uint8_t i = 0; i < sensorCount; i++) sensorOrder[i] = i;
    return;
  }
  File file = LittleFS.open(CONFIG_FILE, "r");
  if (!file) return;
  String content = file.readString();
  file.close();
  DynamicJsonDocument doc(512);
  DeserializationError err = deserializeJson(doc, content);
  if (err) return;
  for (uint8_t i = 0; i < sensorCount; i++) {
    for (uint8_t j = 0; j < sensorCount; j++) {
      if (addressToString(foundAddresses[j]) == doc[i].as<String>()) {
        sensorOrder[i] = j;
        break;
      }
    }
  }
}

void saveOrder(String json) {
  File file = LittleFS.open(CONFIG_FILE, "w");
  if (!file) return;
  file.print(json);
  file.close();
}

// --- HTTP-Endpunkte ---
void handleRoot() {
  File file = LittleFS.open(INDEX_HTML, "r");
  server.streamFile(file, "text/html");
  file.close();
}

void handleGetSensors() {
  DynamicJsonDocument doc(1024);
  for (uint8_t i = 0; i < sensorCount; i++) {
    JsonObject obj = doc.createNestedObject();
    obj["address"] = addressToString(foundAddresses[i]);
    float temp = sensors.getTempC(foundAddresses[i]);
    obj["temperature"] = (temp == DEVICE_DISCONNECTED_C) ? -127.0 : temp;
  }
  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void handlePostOrder() {
  String body = server.arg("plain");
  saveOrder(body);
  server.send(200, "text/plain", "OK");
  loadOrder();
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  LittleFS.begin();
  scanSensors();
  loadOrder();

  server.on("/", handleRoot);
  server.on("/get_sensors", handleGetSensors);
  server.on("/set_order", HTTP_POST, handlePostOrder);
  server.begin();

  ArduinoOTA.setHostname("ESP32-Schichtung");
  ArduinoOTA.begin();
  Serial.println("OTA bereit");

  modbusServer.begin();
  modbusServer.configureHoldingRegisters(100, NUM_SENSORS_MAX);
  Serial.println("Modbus TCP Server gestartet");
}

void loop() {
  server.handleClient();
  ArduinoOTA.handle();

  sensors.requestTemperatures();
  for (uint8_t i = 0; i < sensorCount; i++) {
    uint8_t realIndex = sensorOrder[i];
    float temp = sensors.getTempC(foundAddresses[realIndex]);
    temps[i] = (temp == DEVICE_DISCONNECTED_C) ? -1270 : (int16_t)(temp * 10);
    modbusServer.holdingRegisterWrite(100 + i, temps[i]);
  }

  delay(2000);
}
