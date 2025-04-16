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
- Over-the-Air (OTA) Updates mit Passwortschutz
- Automatische Wiederverbindung bei WLAN-Verlust
- Offline-fähige Weboberfläche ohne externe Ressourcen
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

### Over-the-Air (OTA) Updates

Nach der ersten Installation über USB können weitere Updates drahtlos durchgeführt werden:

1. ESP32 mit dem WLAN verbinden
2. In der Arduino IDE unter Tools -> Port den Netzwerk-Port auswählen
   - Der ESP32 erscheint als "ESP32-Schichtung at 192.168.x.x"
3. Beim Upload das Passwort "puffer123" eingeben
4. Der Upload startet automatisch

## Hardware-Anforderungen

### Notwendige Hardware

- ESP32 Mikrocontroller https://amzn.to/3G2k1DU
- DS18B20 Temperatursensoren (max. 10 Stück) https://amzn.to/4cv8dWT
- DS18B20 Adapter Platine https://amzn.to/3R8Sxif
- USB-C Kabel https://amzn.to/4lx5KPV
- USB-Type-C Netzteil https://amzn.to/3XSio1A
- Gehäuse https://amzn.to/4cBsmL2

### Wenn du mir helfen möchtest

Wenn du bei der Auswahl der Hardware über die hier verlinkten Amazon-Produktlinks kaufst, unterstützt du meine Arbeit an diesem Projekt. Einige Prozent des Kaufpreises werden mir als Provision zugesprochen, ohne dass du dadurch Mehrkosten hast. Vielen Dank für deine Unterstützung!

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

## Over-the-Air (OTA) Updates

Das System unterstützt drahtlose Firmware-Updates über WLAN:

1. ESP32 muss mit dem WLAN verbunden sein
2. In der Arduino IDE unter Tools -> Port den Netzwerk-Port auswählen
   - Der ESP32 erscheint als "ESP32-Puffer at 192.168.x.x"
3. Beim Upload das OTA-Passwort eingeben: "puffer123"
4. Der Upload startet automatisch

Hinweise:
- Während des Updates blinkt die LED des ESP32
- Das System startet nach erfolgreichem Update automatisch neu
- Bei Verbindungsproblemen bitte die WLAN-Verbindung prüfen

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