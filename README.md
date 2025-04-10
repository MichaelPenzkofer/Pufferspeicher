# Pufferspeicher-Temperatursensor System

Dieses Projekt implementiert ein intelligentes Temperatursensor-System für Pufferspeicher auf Basis eines ESP32. Es ermöglicht die Überwachung mehrerer DS18B20 Temperatursensoren und deren flexible Anordnung über eine Weboberfläche.

## Funktionen

- Unterstützung für bis zu 10 DS18B20 Temperatursensoren
- Benutzerfreundliche Weboberfläche zur Konfiguration
- Drag & Drop Interface zur Anordnung der Sensoren
- Automatische WLAN-Konfiguration im ersten Start
- Modbus TCP Server für Systemintegration
- Over-the-Air (OTA) Updates
- Automatische Wiederverbindung bei WLAN-Verlust

## Hardware-Anforderungen

- ESP32 Mikrocontroller
- DS18B20 Temperatursensoren (max. 10 Stück)
- 4,7kΩ Pull-up Widerstand für den OneWire Bus

## Anschlüsse

- OneWire Bus (DS18B20): GPIO4 (D4)

## Installation

1. Klonen Sie das Repository:
   ```bash
   git clone https://github.com/MichaelPenzkofer/Pufferspeicher.git
   ```

2. Öffnen Sie das Projekt in der Arduino IDE

3. Installieren Sie die benötigten Bibliotheken:
   - WiFi
   - WebServer
   - OneWire
   - DallasTemperature
   - LittleFS
   - ArduinoOTA
   - ModbusTCPServer
   - ArduinoJson
   - DNSServer

4. Flashen Sie den Code auf Ihren ESP32

## Ersteinrichtung

1. Nach dem ersten Start erstellt der ESP32 einen Access Point mit dem Namen "ESP32-Puffer-Setup"
2. Verbinden Sie sich mit diesem WLAN-Netzwerk
3. Öffnen Sie die Weboberfläche unter der IP-Adresse 192.168.4.1
4. Geben Sie Ihre WLAN-Zugangsdaten ein
5. Nach dem Neustart verbindet sich der ESP32 mit Ihrem WLAN

## Sensorkonfiguration

1. Verbinden Sie sich mit dem gleichen WLAN wie der ESP32
2. Öffnen Sie die Weboberfläche (IP-Adresse wird über Serial Monitor angezeigt)
3. Die erkannten Sensoren werden automatisch angezeigt
4. Ordnen Sie die Sensoren per Drag & Drop von oben (oberster Sensor im Puffer) nach unten an
5. Speichern Sie die Konfiguration

## Modbus TCP

Die Temperaturwerte sind über Modbus TCP verfügbar:
- Server Port: 502 (Standard Modbus Port)
- Register Start: 100
- Temperaturwerte: Multipliziert mit 10 (z.B. 23,5°C = 235)

## Fehlersuche

1. WLAN-Verbindung fehlgeschlagen
   - ESP32 wechselt automatisch in den AP-Modus
   - Konfigurieren Sie das WLAN erneut

2. Sensor nicht erkannt
   - Überprüfen Sie die Verkabelung
   - Kontrollieren Sie den Pull-up Widerstand
   - Prüfen Sie die Sensor-Adressen im Serial Monitor

## Lizenz

Dieses Projekt ist unter der MIT-Lizenz veröffentlicht. Siehe [LICENSE](LICENSE) Datei für Details.

## Autor

Michael Penzkofer

## Contributing

Pull Requests sind willkommen. Für größere Änderungen erstellen Sie bitte zuerst ein Issue, um die Änderungen zu diskutieren.