# Buffer Tank Temperature Sensor System

[Deutsche Version](README.md)

This project implements a smart temperature sensor system for buffer tanks based on an ESP32. It enables monitoring of multiple DS18B20 temperature sensors and their flexible arrangement via a web interface.

## Features

- Support for up to 10 DS18B20 temperature sensors
- User-friendly web interface for configuration
- Drag & drop interface for sensor arrangement
- Save and reset sensor order
- Automatic WiFi configuration on first start
- Modbus TCP server for system integration
- Over-the-Air (OTA) updates with password protection
- Automatic reconnection on WiFi loss
- Offline-capable web interface without external resources

## Installation and Setup

### Arduino IDE Settings

1. Board: "ESP32 Dev Module" (default settings)
2. Upload Speed: 921600
3. CPU Frequency: 240MHz (WiFi/BT)
4. Flash Frequency: 80MHz
5. Flash Mode: QIO
6. Flash Size: 4MB (32Mb)
7. Partition Scheme: Default 4MB with spiffs

### Over-the-Air (OTA) Updates

After the initial installation via USB, further updates can be performed wirelessly:

1. Connect ESP32 to WiFi
2. In Arduino IDE, select the network port under Tools -> Port
   - The ESP32 appears as "ESP32-Schichtung at 192.168.x.x"
3. Enter the password "puffer123" when prompted
4. The upload starts automatically

## Hardware Requirements

### Required Hardware

- ESP32 Microcontroller https://amzn.to/3G2k1DU
- DS18B20 Temperature Sensors (max. 10 pieces) https://amzn.to/4cv8dWT
- DS18B20 Adapter Board https://amzn.to/3R8Sxif
- USB-C Cable https://amzn.to/4lx5KPV
- USB Type-C Power Supply https://amzn.to/3XSio1A
- Enclosure https://amzn.to/4cBsmL2

### Support the Project

If you purchase the hardware through the Amazon product links provided here, you'll support my work on this project. I receive a small percentage of the purchase price as commission, without any additional cost to you. Thank you for your support!

## Connections

- OneWire Bus (DS18B20): GPIO4 (D4)

## Installation

1. Clone the repository:
   ```bash
   git clone https://github.com/MichaelPenzkofer/Pufferspeicher.git
   ```

2. Open the project in Arduino IDE

3. Install the required libraries:
   - WiFi
   - WebServer
   - OneWire
   - DallasTemperature
   - LittleFS
   - ArduinoOTA
   - ModbusTCPServer
   - ArduinoJson
   - DNSServer

4. Flash the code to your ESP32

## Initial Setup

1. After the first start, the ESP32 creates an access point named "ESP32-Puffer-Setup"
2. Connect to this WiFi network
3. Open the web interface at IP address 192.168.4.1
4. Enter your WiFi credentials
5. After restart, the ESP32 will connect to your WiFi

## Sensor Configuration

1. Connect to the same WiFi network as the ESP32
2. Open the web interface (IP address is shown in Serial Monitor)
3. Detected sensors are displayed automatically
4. Arrange the sensors via drag & drop from top (topmost sensor in buffer) to bottom
5. Save the configuration

## Modbus TCP

Temperature values are available via Modbus TCP:
- Server Port: 502 (standard Modbus port)
- Register Start: 100
- Temperature values: Multiplied by 10 (e.g., 23.5°C = 235)

## Troubleshooting

1. WiFi Connection Failed
   - ESP32 automatically switches to AP mode
   - Reconfigure the WiFi settings

2. Sensor Not Detected
   - Check the wiring
   - Verify the pull-up resistor
   - Check sensor addresses in Serial Monitor

## License

This project is released under the MIT License. See [LICENSE](LICENSE) file for details.

## Author

Michael Penzkofer

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.
