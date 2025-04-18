# Pufferspeicher-Temperatursensor System

[English version](README.en.md)

Dieses Projekt implementiert ein intelligentes Temperatursensor-System für Pufferspeicher auf Basis eines ESP32. Es ermöglicht die Überwachung mehrerer DS18B20 Temperatursensoren und deren flexible Anordnung über eine Weboberfläche.

## Funktionen

- Unterstützung für bis zu 10 DS18B20 Temperatursensoren
- Benutzerfreundliche Weboberfläche zur Konfiguration
- Drag & Drop Interface zur Anordnung der Sensoren
- Speichern und Zurücksetzen der Fühlerreihenfolge
- Automatische WLAN-Konfiguration im ersten Start
- Modbus TCP Server für Systemintegration
- Automatische Wiederverbindung bei WLAN-Verlust
- Webserver-Watchdog für erhöhte Stabilität
- Regelmäßige WLAN-Verbindungsprüfung
- Temperaturaktualisierung alle 12 Sekunden
- DNS-Server im AP-Modus für einfache Konfiguration
- Möglichkeit zum Zurücksetzen der WLAN-Konfiguration

## Installation und Einrichtung

### Arduino IDE Einstellungen

1. Board: "ESP32 Dev Module" (Standardeinstellungen)
2. Upload Speed: 921600
3. CPU Frequency: 240MHz (WiFi/BT)
4. Flash Frequency: 80MHz
5. Flash Mode: QIO
6. Flash Size: 4MB (32Mb)
7. Partition Scheme: Default 4MB with spiffs

## Hardware-Anforderungen

### Notwendige Hardware

- ESP32 Mikrocontroller https://amzn.to/3GezHUC
- DS18B20 Temperatursensoren (max. 10 Stück) https://amzn.to/4cv8dWT
- DS18B20 Adapter Platine https://amzn.to/3R8Sxif
- USB-C Kabel https://amzn.to/4lx5KPV
- USB-Type-C Netzteil https://amzn.to/3XSio1A
- Gehäuse https://amzn.to/4cBsmL2
- Aluminium Klebeband https://amzn.to/3EAXIEM

### Wenn du mir helfen möchtest

Wenn du bei der Auswahl der Hardware über die hier verlinkten Amazon-Produktlinks kaufst, unterstützt du meine Arbeit an diesem Projekt. Einige Prozent des Kaufpreises werden mir als Provision zugesprochen, ohne dass du dadurch Mehrkosten hast. Vielen Dank für deine Unterstützung!

## Anschlüsse

- OneWire Bus (DS18B20): GPIO4 (D4)

| Anzahl Sensoren | Kabellänge | Pull-Up Empfehlung
|----------------|------------|------------------
| 1–3            | <5 m       | 4,7 kΩ
| 4–10           | <10 m      | 2,2–3,3 kΩ

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

## Geplante Erweiterungen

Die folgenden Integrationen sind für zukünftige Versionen geplant:

### Home Assistant Integration
- Modbus TCP Einbindung der Temperatursensoren in Home Assistant
- Visualisierung mit Home Assistant
- Konfiguration über Home Assistant

### ESPHome Display Integration
- Anzeige der Temperaturen auf einem ESP32 Display
- Erstellung mit ESPHome Builder
- Echtzeit-Visualisierung der Pufferschichtung

### Node-RED Integration
- Fertige Node-RED Flows für gängige Anwendungsfälle
- Automatisierte Datenaufzeichnung

### Wago CoDeSys Integration
- Modbus TCP Bibliothek für CoDeSys
- Beispielprogramm für Wago Controller
- Dokumentation zur SPS-Integration

*Diese Funktionen befinden sich in Entwicklung und werden nach und nach veröffentlicht.*

## Contributing

Pull Requests sind willkommen. Für größere Änderungen erstellen Sie bitte zuerst ein Issue, um die Änderungen zu diskutieren.