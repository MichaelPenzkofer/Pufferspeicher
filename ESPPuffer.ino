// --- Bibliotheken einbinden ---
#include <WiFi.h>
#include <WebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LittleFS.h>
#include <ArduinoOTA.h>
#include <ModbusTCPServer.h>
#include <ArduinoJson.h>
#include <DNSServer.h>

// --- Konstanten und Einstellungen ---
#define ONE_WIRE_BUS 4 // GPIO4 (D4) für DS18B20
#define NUM_SENSORS_MAX 10
#define CONFIG_FILE "/order.json"
#define WIFI_CONFIG_FILE "/wifi.json"
#define INDEX_HTML "/index.html"
#define WIFI_TIMEOUT 30000 // 30 Sekunden Timeout für WLAN-Verbindung
#define TEMP_UPDATE_INTERVAL 10000 // 10 Sekunden Intervall für Temperaturmessungen
#define DNS_PORT 53

// --- Netzwerkdaten ---
String ssid;
String password;
const char* ap_ssid = "ESP32-Puffer-Setup";
DNSServer dnsServer;
bool isAP = false;
unsigned long lastTempUpdate = 0;

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

void loadWiFiConfig() {
  if (!LittleFS.exists(WIFI_CONFIG_FILE)) return;
  
  File file = LittleFS.open(WIFI_CONFIG_FILE, "r");
  if (!file) return;
  
  DynamicJsonDocument doc(256);
  DeserializationError err = deserializeJson(doc, file);
  file.close();
  
  if (err) return;
  
  ssid = doc["ssid"].as<String>();
  password = doc["password"].as<String>();
}

void saveWiFiConfig() {
  DynamicJsonDocument doc(256);
  doc["ssid"] = ssid;
  doc["password"] = password;
  
  File file = LittleFS.open(WIFI_CONFIG_FILE, "w");
  if (!file) return;
  
  serializeJson(doc, file);
  file.close();
}

void startAP() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid);
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
  isAP = true;
  Serial.println("AP Mode started");
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());
}

bool connectWiFi() {
  if (ssid.length() == 0) return false;
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && 
         millis() - startAttemptTime < WIFI_TIMEOUT) {
    delay(100);
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection failed");
    return false;
  }
  
  Serial.println("WiFi connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  return true;
}

void handleWiFiConfig() {
  if (server.hasArg("plain")) {
    DynamicJsonDocument doc(256);
    DeserializationError err = deserializeJson(doc, server.arg("plain"));
    
    if (!err) {
      ssid = doc["ssid"].as<String>();
      password = doc["password"].as<String>();
      saveWiFiConfig();
      
      server.send(200, "text/plain", "OK");
      delay(1000);
      ESP.restart();
      return;
    }
  }
  server.send(400, "text/plain", "Invalid request");
}

void setup() {
  Serial.begin(115200);
  LittleFS.begin();
  
  loadWiFiConfig();
  if (!connectWiFi()) {
    startAP();
  }

  LittleFS.begin();
  scanSensors();
  loadOrder();

  server.on("/", handleRoot);
  server.on("/get_sensors", handleGetSensors);
  server.on("/set_order", HTTP_POST, handlePostOrder);
  server.on("/set_wifi", HTTP_POST, handleWiFiConfig);
  server.on("/get_mode", HTTP_GET, []() {
    DynamicJsonDocument doc(64);
    doc["isAP"] = isAP;
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  });
  server.begin();

  ArduinoOTA.setHostname("ESP32-Schichtung");
  ArduinoOTA.begin();
  Serial.println("OTA bereit");

  modbusServer.begin();
  modbusServer.configureHoldingRegisters(100, NUM_SENSORS_MAX);
  Serial.println("Modbus TCP Server gestartet");
}

void loop() {
  if (isAP) {
    dnsServer.processNextRequest();
  }
  
  server.handleClient();
  ArduinoOTA.handle();
  
  // Temperaturmessung im definierten Intervall
  unsigned long currentMillis = millis();
  if (currentMillis - lastTempUpdate >= TEMP_UPDATE_INTERVAL) {
    lastTempUpdate = currentMillis;
    
    sensors.requestTemperatures();
    for (uint8_t i = 0; i < sensorCount; i++) {
      uint8_t realIndex = sensorOrder[i];
      float temp = sensors.getTempC(foundAddresses[realIndex]);
      temps[i] = (temp == DEVICE_DISCONNECTED_C) ? -1270 : (int16_t)(temp * 10);
      modbusServer.holdingRegisterWrite(100 + i, temps[i]);
    }
  }
  
  // WLAN-Verbindung überprüfen und ggf. neu verbinden
  if (!isAP && WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection lost, reconnecting...");
    if (!connectWiFi()) {
      Serial.println("Reconnection failed, starting AP mode...");
      startAP();
    }
  }
  
  delay(10); // Kleine Pause für ESP32 Watchdog
}
