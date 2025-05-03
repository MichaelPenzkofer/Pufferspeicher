// --- Bibliotheken einbinden ---
#include <WiFi.h>
#include <WebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ModbusIP_ESP8266.h>
#include <ArduinoJson.h>
#include <DNSServer.h>
#include <SPIFFS.h>

// --- Watchdog für ESP32 ---
#ifdef ESP32
#include <esp_task_wdt.h>
#endif

// --- Konstanten und Einstellungen ---
#define ONE_WIRE_BUS 4 // GPIO4 für DS18B20
#define NUM_SENSORS_MAX 10
#define CONFIG_FILE "/order.json"
#define WIFI_CONFIG_FILE "/wifi.json"
#define INDEX_HTML "/index.html"
#define WIFI_TIMEOUT 30000 // 30 Sekunden Timeout für WLAN-Verbindung
#define TEMP_UPDATE_INTERVAL 12000 // 12 Sekunden Intervall für Temperaturmessungen (750ms * 10 Sensoren + Overhead)
#define DNS_PORT 53
#define WIFI_CHECK_INTERVAL 60000 // 60 Sekunden Intervall für WLAN-Prüfung
// --- Netzwerkdaten ---
String ssid;
String password;
const char* ap_ssid = "ESP32-Puffer-Setup";
DNSServer dnsServer;
bool isAP = false;
unsigned long lastTempUpdate = 0;
unsigned long lastWiFiCheck = 0; // Entfernt: Webserver-Watchdog-Variablen


// --- Globale Variablen ---
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
ModbusIP mb; // ModbusIP Instanz

// --- Re-Init Intervalle (z.B. alle 12h = 43.200.000 ms) ---
#define REINIT_INTERVAL 43200000UL
unsigned long lastReinit = 0;

// Modbus Register Adressen
#define MB_TEMP_START 100  // Startadresse für Temperaturen
#define WEB_SERVER_INTERVAL 100  // Webserver-Aktualisierungsintervall in ms
#define MB_STATUS_REG 99   // Status Register

// HTML-Datei im Flash-Speicher
#include "index_html.h"

WebServer* server = nullptr;
DeviceAddress foundAddresses[NUM_SENSORS_MAX];
uint8_t sensorCount = 0;
uint8_t sensorOrder[NUM_SENSORS_MAX];
int16_t temps[NUM_SENSORS_MAX];

// --- Hilfsfunktionen für Re-Init ---
void reinitModbus() {
  mb = ModbusIP(); // Neu instanzieren
  mb.server();
  mb.addHreg(MB_STATUS_REG);
  for(int i = 0; i < NUM_SENSORS_MAX; i++) {
    mb.addHreg(MB_TEMP_START + i);
  }
  Serial.println("Modbus TCP Server wurde re-initialisiert!");
}

void reinitWebServer() {
  if (server) {
    server->close();
    delete server;
  }
  server = new WebServer(80);
  server->on("/", HTTP_GET, handleRoot);
  server->on("/get_sensors", HTTP_GET, handleGetSensors);
  server->on("/set_order", HTTP_POST, handlePostOrder);
  server->on("/get_initial_order", HTTP_GET, handleGetInitialOrder);
  server->on("/reset_order", HTTP_POST, handleResetOrder);
  server->on("/reset_wifi", HTTP_POST, handleResetWiFi);
  server->on("/set_wifi", HTTP_POST, handleWiFiConfig);
  server->on("/get_mode", HTTP_GET, []() {
    DynamicJsonDocument doc(64);
    doc["isAP"] = isAP;
    String response;
    serializeJson(doc, response);
    Serial.print("Current mode: ");
    Serial.println(isAP ? "AP" : "STA");
    server->send(200, "application/json", response);
  });
  server->begin();
  Serial.println("Webserver wurde re-initialisiert!");
}

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
  // Initialize default order
  for (uint8_t i = 0; i < sensorCount; i++) {
    sensorOrder[i] = i;
  }

  if (!SPIFFS.exists(CONFIG_FILE)) {
    return;
  }

  File file = SPIFFS.open(CONFIG_FILE, "r");
  if (!file) return;

  String content = file.readString();
  file.close();

  DynamicJsonDocument doc(512);
  DeserializationError err = deserializeJson(doc, content);
  if (err) return;

  // Create a temporary array to track which sensors have been assigned
  bool assigned[NUM_SENSORS_MAX] = {false};
  uint8_t newOrder[NUM_SENSORS_MAX];
  uint8_t assignedCount = 0;

  // First pass: assign positions to sensors that are in the saved order
  JsonArray array = doc.as<JsonArray>();
  uint8_t savedCount = array.size();

  for (uint8_t i = 0; i < savedCount && i < sensorCount; i++) {
    String savedAddr = array[i].as<String>();
    for (uint8_t j = 0; j < sensorCount; j++) {
      if (addressToString(foundAddresses[j]) == savedAddr) {
        newOrder[assignedCount] = j;
        assigned[j] = true;
        assignedCount++;
        break;
      }
    }
  }

  // Second pass: append any new sensors that weren't in the saved order
  for (uint8_t i = 0; i < sensorCount; i++) {
    if (!assigned[i]) {
      newOrder[assignedCount] = i;
      assignedCount++;
    }
  }

  // Copy the new order to sensorOrder
  for (uint8_t i = 0; i < sensorCount; i++) {
    sensorOrder[i] = newOrder[i];
  }
}

void saveOrder(String json) {
  File file = SPIFFS.open(CONFIG_FILE, "w");
  if (!file) return;
  file.print(json);
  file.close();
}

// --- HTTP-Endpunkte ---
void handleRoot() {
  Serial.println("Handling root request...");
  server->send(200, "text/html", index_html);
  Serial.println("index.html gesendet");
}

void handleGetInitialOrder() {
  DynamicJsonDocument doc(1024);
  JsonArray array = doc.to<JsonArray>();

  // Send sensors in their configured order with temperatures
  for (uint8_t i = 0; i < sensorCount; i++) {
    JsonObject obj = array.createNestedObject();
    uint8_t sensorIndex = sensorOrder[i];
    obj["address"] = addressToString(foundAddresses[sensorIndex]);
    float temp = sensors.getTempC(foundAddresses[sensorIndex]);
    obj["temperature"] = (temp == DEVICE_DISCONNECTED_C) ? -127.0 : temp;
  }

  String out;
  serializeJson(doc, out);
  server->send(200, "application/json", out);
}

void handleGetSensors() {
  DynamicJsonDocument doc(1024);
  JsonArray array = doc.to<JsonArray>();

  // Send all sensors with current temperatures
  for (uint8_t i = 0; i < sensorCount; i++) {
    JsonObject obj = array.createNestedObject();
    obj["address"] = addressToString(foundAddresses[i]);
    float temp = sensors.getTempC(foundAddresses[i]);
    obj["temperature"] = (temp == DEVICE_DISCONNECTED_C) ? -127.0 : temp;
  }

  String out;
  serializeJson(doc, out);
  server->send(200, "application/json", out);
}

void handlePostOrder() {
  String body = server->arg("plain");
  saveOrder(body);
  server->send(200, "text/plain", "OK");
  loadOrder();
}

void loadWiFiConfig() {
  if (!SPIFFS.exists(WIFI_CONFIG_FILE)) return;
  
  File file = SPIFFS.open(WIFI_CONFIG_FILE, "r");
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
  
  File file = SPIFFS.open(WIFI_CONFIG_FILE, "w");
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
  if (server->method() != HTTP_POST) {
    server->send(405, "text/plain", "Method Not Allowed");
    return;
  }

  if (!server->hasArg("plain")) {
    server->send(400, "text/plain", "Keine Daten empfangen");
    return;
  }

  DynamicJsonDocument doc(256);
  DeserializationError error = deserializeJson(doc, server->arg("plain"));

  if (error) {
    server->send(400, "text/plain", "Ungültige JSON-Daten");
    return;
  }

  if (!doc.containsKey("ssid") || !doc.containsKey("password")) {
    server->send(400, "text/plain", "SSID und Passwort erforderlich");
    return;
  }

  String newSsid = doc["ssid"].as<String>();
  String newPassword = doc["password"].as<String>();

  if (newSsid.length() == 0) {
    server->send(400, "text/plain", "SSID darf nicht leer sein");
    return;
  }
  
  ssid = newSsid;
  password = newPassword;
  saveWiFiConfig();
  
  server->send(200, "text/plain", "OK");
  delay(1000);
  ESP.restart();
}

void resetWiFiConfig() {
  if (SPIFFS.exists(WIFI_CONFIG_FILE)) {
    SPIFFS.remove(WIFI_CONFIG_FILE);
  }
  ssid = "";
  password = "";
  WiFi.mode(WIFI_OFF);
  delay(1000);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid);
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
  isAP = true;
  Serial.println("AP Mode started");
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());
}

void resetSensorOrder() {
  if (SPIFFS.exists(CONFIG_FILE)) {
    SPIFFS.remove(CONFIG_FILE);
  }
  // Reset to default order
  for (uint8_t i = 0; i < sensorCount; i++) {
    sensorOrder[i] = i;
  }
}

void handleResetWiFi() {
  server->send(200, "text/plain", "OK");
  delay(100);  // Brief delay to allow response to be sent
  resetWiFiConfig();
  delay(1000);  // Allow time for AP mode to start
  ESP.restart();
}

void handleResetOrder() {
  resetSensorOrder();
  server->send(200, "text/plain", "OK");
  // No restart needed for order reset
}

void setup() {
  server = new WebServer(80);
  Serial.begin(115200);
  Serial.println("\nPufferspeicher Temperatursensor System startet...");

  // --- Hardware-Watchdog initialisieren (nur ESP32) ---
#ifdef ESP32
  // Watchdog-Konfiguration für aktuelle ESP-IDF
  esp_task_wdt_config_t wdt_config = {
    .timeout_ms = 10000,      // 10 Sekunden Timeout
    .idle_core_mask = (1 << portNUM_PROCESSORS) - 1, // alle Kerne überwachen
    .trigger_panic = true     // Reset bei Timeout
  };
  esp_task_wdt_init(&wdt_config);
  esp_task_wdt_add(NULL); // Haupt-Task hinzufügen
  Serial.println("ESP32 Hardware-Watchdog aktiviert (10s Timeout)");
#endif
  
  // Sensoren initialisieren
  sensors.begin();
  sensors.setResolution(12); // 12-bit Auflösung (0.0625°C)
  sensors.setWaitForConversion(true);
  
  // Initialize SPIFFS
  if(!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
  Serial.println("SPIFFS Mount Successful");
   
  
  loadWiFiConfig();
  if (!connectWiFi()) {
    startAP();
  } else {
    // Modbus Server nur initialisieren, wenn wir im Station-Modus sind
    mb.server();
    
    // Modbus Register initialisieren
    mb.addHreg(MB_STATUS_REG);
    for(int i = 0; i < NUM_SENSORS_MAX; i++) {
      mb.addHreg(MB_TEMP_START + i);
    }
    
    Serial.println("Modbus TCP Server gestartet auf Port 502");
  }
  

  scanSensors();
  loadOrder();

  // Server-Endpunkte registrieren
  server->on("/", HTTP_GET, handleRoot);
  server->on("/get_sensors", HTTP_GET, handleGetSensors);
  server->on("/set_order", HTTP_POST, handlePostOrder);
  server->on("/get_initial_order", HTTP_GET, handleGetInitialOrder);
  server->on("/reset_order", HTTP_POST, handleResetOrder);
  server->on("/reset_wifi", HTTP_POST, handleResetWiFi);
  server->on("/set_wifi", HTTP_POST, handleWiFiConfig);
  server->on("/get_mode", HTTP_GET, []() {
    DynamicJsonDocument doc(64);
    doc["isAP"] = isAP;
    String response;
    serializeJson(doc, response);
    Serial.print("Current mode: ");
    Serial.println(isAP ? "AP" : "STA");
    server->send(200, "application/json", response);
  });
  server->begin();


 
}

void loop() {
  unsigned long currentMillis = millis();
  static unsigned long lastHeapLog = 0;
  static unsigned long lastWebServerUpdate = 0;

  // --- Modbus & Webserver regelmäßig re-initialisieren ---
  if (currentMillis - lastReinit >= REINIT_INTERVAL) {
    lastReinit = currentMillis;
    Serial.println("[ReInit] Modbus und Webserver werden neu initialisiert...");
    reinitModbus();
    reinitWebServer();
  }
  
  // --- Watchdog regelmäßig zurücksetzen (nur ESP32) ---
#ifdef ESP32
  esp_task_wdt_reset();
#endif

  // --- Heap-Speicher regelmäßig ausgeben ---
  int heapFree = ESP.getFreeHeap();
  if (currentMillis - lastHeapLog >= 60000) { // alle 60 Sekunden
    lastHeapLog = currentMillis;
    Serial.print("[Heap] Freier Speicher: ");
    Serial.print(heapFree);
    Serial.println(" Bytes");
  }

  // --- Automatischer Neustart bei zu wenig Speicher ---
  if (heapFree < 20000) { // Schwellenwert ggf. anpassen
    Serial.println("[Heap] Kritisch wenig Speicher! Neustart wird ausgelöst...");
    delay(1000); // kurze Wartezeit für serielle Ausgabe
    ESP.restart();
  }

  // Webserver Tasks mit Intervall
  if (currentMillis - lastWebServerUpdate >= WEB_SERVER_INTERVAL) {
    lastWebServerUpdate = currentMillis;
    server->handleClient();
    // Entfernt: serverRequestCount++
    
    if (isAP) {
      dnsServer.processNextRequest();
    }
    
    // DNS Server nur im AP Modus
  }
  
  // Modbus Tasks ausführen
  mb.task();
  
  // Watchdog Reset verhindern
  yield();
  
  // Temperaturen aktualisieren
  if (currentMillis - lastTempUpdate >= TEMP_UPDATE_INTERVAL) {
    lastTempUpdate = currentMillis;
    sensors.requestTemperatures();
    
    // Status Register aktualisieren (1 = OK, 0 = Fehler)
    mb.Hreg(MB_STATUS_REG, 1);
    
    // Temperaturen in Modbus Register schreiben
    for(int i = 0; i < NUM_SENSORS_MAX; i++) {
      if (i < sensorCount) {
        uint8_t realIndex = sensorOrder[i];
        // Temperatur * 10 um eine Dezimalstelle zu behalten
        float temp = sensors.getTempC(foundAddresses[realIndex]);
        temps[i] = (temp == DEVICE_DISCONNECTED_C) ? -1270 : (int16_t)(temp * 10);
        mb.Hreg(MB_TEMP_START + i, (uint16_t)temps[i]);
      } else {
        mb.Hreg(MB_TEMP_START + i, 0xFFFF); // Markiere nicht vorhandene Sensoren
      }
    }
  }
  
  // WLAN-Verbindung regelmäßig überprüfen
  if (currentMillis - lastWiFiCheck >= WIFI_CHECK_INTERVAL) {
    lastWiFiCheck = currentMillis;
    if (!isAP) {
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi connection lost, reconnecting...");
        WiFi.disconnect();
        delay(1000);
        if (!connectWiFi()) {
          Serial.println("Reconnection failed, starting AP mode...");
          startAP();
        }
      }
    }
  }
  

  
  // Yield statt delay für besseres Task-Management
  yield();
}
