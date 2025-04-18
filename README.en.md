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
- Automatic reconnection on WiFi loss
- Offline-capable web interface without external resources
- Webserver watchdog for increased stability
- Regular WiFi connection checks
- Temperature updates every 12 seconds
- DNS server in AP mode for easy configuration
- Option to reset WiFi configuration

## Installation and Setup

### Arduino IDE Settings

1. Board: "ESP32 Dev Module" (default settings)
2. Upload Speed: 921600
3. CPU Frequency: 240MHz (WiFi/BT)
4. Flash Frequency: 80MHz
5. Flash Mode: QIO
6. Flash Size: 4MB (32Mb)
7. Partition Scheme: Default 4MB with spiffs

## Hardware Requirements

### Required Hardware

- ESP32 Microcontroller https://amzn.to/3GezHUC
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

## Planned Extensions

The following integrations are planned for future versions:

### Home Assistant Integration
- Modbus TCP integration of temperature sensors into Home Assistant
- Visualization with Home Assistant
- Configuration through Home Assistant

### ESPHome Display Integration
- Display temperatures on an ESP32 display
- Creation using ESPHome Builder
- Real-time visualization of buffer tank stratification

### Node-RED Integration
- Ready-to-use Node-RED flows for common use cases
- Automated data logging

### Wago CoDeSys Integration
- Modbus TCP library for CoDeSys
- Example program for Wago controllers
- PLC integration documentation

*These features are under development and will be released gradually.*

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.
