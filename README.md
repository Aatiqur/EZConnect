<div align="center">

# EZConnect

**Easy WiFi Configuration for ESP32 via Bluetooth (Classic BT & BLE)**

[![Arduino](https://img.shields.io/badge/Arduino-Compatible-00979D?style=flat&logo=arduino)](https://www.arduino.cc/)
[![ESP32](https://img.shields.io/badge/Platform-ESP32-E7352C?style=flat&logo=espressif)](https://www.espressif.com/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Version](https://img.shields.io/badge/version-1.2.0-blue.svg)](https://github.com/Aatiqur/EZConnect)

*Simplify WiFi provisioning with Classic Bluetooth or BLE on ESP32 boards*

[Features](#features) • [Why EZConnect?](#why-ezconnect) • [Installation](#installation) • [Quick Start](#quick-start) • [Examples](#examples) • [Documentation](#documentation)

</div>

---

## About

EZConnect is a versatile Arduino/ESP32 library that simplifies WiFi configuration using either **Classic Bluetooth** or **BLE (Bluetooth Low Energy)**. It allows users to send WiFi credentials wirelessly via Bluetooth and automatically connect the ESP32 to WiFi without hardcoding SSID or password.

The library supports both Classic BT and BLE modes, making it compatible with all ESP32 variants including the newer **ESP32-C3, ESP32-S3, and ESP32-S2 series** which have different Bluetooth hardware availability. This flexibility empowers hobbyists and developers to choose the best Bluetooth mode for their specific ESP32 board.

Perfect for IoT projects, smart devices, wireless configuration systems, and simplifying first-time WiFi setup.

## Why EZConnect?

### Universal Bluetooth Support

Not all ESP32 variants support the same Bluetooth hardware:
- **ESP32** (Original) - Supports both Classic BT and BLE
- **ESP32-C3 / ESP32-C6** - BLE only (no Classic BT)
- **ESP32-S2** - No Bluetooth support
- **ESP32-S3** - BLE only (no Classic BT)

EZConnect **supports both Classic BT and BLE modes**, so you can use whichever is available on your specific ESP32 variant. This eliminates the frustration of incompatibilities and gives developers flexibility in their hardware choices.

### Developer & Hobbyist Friendly

- **No hardcoded credentials** - Credentials are sent wirelessly via Bluetooth
- **No app development required** - Use standard Bluetooth terminal apps for Classic BT
- **Works with existing apps** - Most Android/iOS devices have built-in Bluetooth support
- **BLE option included** - For projects that need lower power consumption with BLE-enabled apps
- **Automatic reconnection** - Once configured, ESP32 reconnects automatically on reboot
- **Persistent storage** - WiFi credentials are safely stored in device memory

## Features

- **Dual Bluetooth Mode Support**
  - Classic Bluetooth - Works with standard Bluetooth serial apps
  - BLE (Bluetooth Low Energy) - Lower power, requires BLE-capable app
- **Persistent storage** - Stores WiFi credentials using Preferences
- **Auto-reconnection** - Automatically reconnects on reboot
- **Configurable timeout** - Set your own WiFi connection timeout
- **Status LED support** - Visual feedback for connection state
- **Simple API** - Minimal setup required
- **Non-blocking design** - Uses millis() for efficient operation

## Supported Boards & Bluetooth Modes

| Board | Classic BT | BLE | Recommended |
|-------|-----------|-----|------------|
| ESP32 (Generic) | Yes | Yes | Either |
| ESP32-C3 | No | Yes | BLE |
| ESP32-C6 | No | Yes | BLE |
| ESP32-S2 | No | No | Not Supported |
| ESP32-S3 | No | Yes | BLE |

**Note:** For ESP32-C3, ESP32-C6, and ESP32-S3, use BLE mode. For original ESP32, you can choose either mode based on your needs.

## Installation

### Method 1: Arduino Library Manager (Recommended)

1. Open Arduino IDE
2. Go to **Sketch** → **Include Library** → **Manage Libraries**
3. Search for **EZConnect**
4. Click **Install**

### Method 2: Manual Installation

1. Download this repository as a ZIP file
2. Open Arduino IDE
3. Go to **Sketch** → **Include Library** → **Add .ZIP Library**
4. Select the downloaded ZIP file

---

## 🔴 CRITICAL STEP FOR GENERIC ESP32 DEV BOARDS - DO NOT SKIP! 🔴


> **CRITICAL STEP**
>
> After installing the library, you must change the partition scheme to avoid compilation or runtime errors.
>
> ### Required Steps:
>
> 1. In Arduino IDE, go to **Tools** → **Partition Scheme**
> 2. **Change from:** `Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)`
> 3. **Change to:** `Huge APP (3MB No OTA/1MB SPIFFS)`
>
> **Why is this necessary?**  
> This library uses both WiFi and Bluetooth simultaneously, which requires more program space than the default partition scheme allows. Without this change, your sketch may fail to compile or upload.

---

## Quick Start

### Classic Bluetooth Mode (Standard Bluetooth Terminal App)

```cpp
#include <EZConnect.h>

// Initialize with Classic Bluetooth mode
EZConnect ez(EZCONNECT_CLASSIC_BT, "EZConnect-Device", 2, 0);

void setup() {
  Serial.begin(115200);
  
  ez.begin();
  ez.showDebugmsg();
  
  String ssid = ez.getSSID();
  String pass = ez.getPass();
  
  WiFi.begin(ssid.c_str(), pass.c_str());
  
  // WiFi timeout = 10s, LED blink interval = 500ms
  ez.checkConnectivity(10000, 500, 500);
}

void loop() {
  ez.check();
}
```

### BLE Mode (Lower Power, Requires BLE App)

```cpp
#include <EZConnect.h>

// Initialize with BLE mode
EZConnect ez(EZCONNECT_BLE, "EZConnect-Device", 2, 0);

void setup() {
  Serial.begin(115200);
  
  ez.begin();
  ez.showDebugmsg();
  
  String ssid = ez.getSSID();
  String pass = ez.getPass();
  
  WiFi.begin(ssid.c_str(), pass.c_str());
  
  ez.checkConnectivity(10000, 500, 500);
}

void loop() {
  ez.check();
}
```

**Choosing between modes:**
- **Classic Bluetooth:** Compatible with standard Bluetooth terminal applications on Android and iOS without additional setup
- **BLE:** Required for ESP32 variants that do not support Classic BT (C3, S3, C6), or when lower power consumption is needed

## How It Works

![System Flowchart](images/Flowchart.png)

1. ESP32 boots and checks for saved WiFi credentials
2. If credentials exist, ESP32 attempts to connect to WiFi
3. If connection fails or credentials are missing, device enters Bluetooth mode
4. User opens a Bluetooth terminal app (for Classic BT) or BLE app (for BLE mode) on their phone or PC
5. User pairs with the device and sends WiFi credentials
6. Credentials are saved to device memory (persistent storage)
7. ESP32 connects to WiFi automatically
8. LED provides visual feedback on connection status
9. Device reconnects automatically on reboot if credentials exist

### Supported Credentials Format

Send WiFi credentials from your Bluetooth app using one of these formats:

| Format | Example |
|--------|---------|
| **Newline separator** | `MyWiFi\n12345678` |
| **Pipe separator** | `MyWiFi\|12345678` |
| **Tab separator** | `MyWiFi	12345678` |
| **Comma separator** | `MyWiFi,12345678` |

Replace `MyWiFi` with your SSID and `12345678` with your WiFi password.

## Documentation

### Constructor

#### Classic Bluetooth Mode
```cpp
EZConnect(EZCONNECT_CLASSIC_BT, const char* deviceName, uint8_t ledPin, uint8_t statusPin);
```

#### BLE Mode
```cpp
EZConnect(EZCONNECT_BLE, const char* deviceName, uint8_t ledPin, uint8_t statusPin);
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `mode` | Enum | Either `EZCONNECT_CLASSIC_BT` or `EZCONNECT_BLE` |
| `deviceName` | `const char*` | Bluetooth/BLE device name (max 31 characters) |
| `ledPin` | `uint8_t` | GPIO pin for status LED (0 = disabled) |
| `statusPin` | `uint8_t` | GPIO pin for status indicator (0 = disabled) |

### Methods

```cpp
void begin();                    
// Initialize EZConnect with chosen Bluetooth mode

String getSSID();                
// Get stored WiFi SSID

String getPass();                
// Get stored WiFi password

void check();                    
// Check for incoming Bluetooth data and process

void showDebugmsg();
// Enable debug messages to Serial Monitor

void checkConnectivity(int wifiTimeout, int blinkOn, int blinkOff);
// Check WiFi connectivity with LED feedback
// Parameters: WiFi timeout (ms), LED on duration (ms), LED off duration (ms)

void setUUIDs(const char* serviceUUID, const char* rxCharUUID);
// (BLE only) Set custom UUIDs for BLE Service and RX Characteristic
```

## Examples

Check out the [examples](examples/) folder for detailed implementations:

- **[BLE.ino](examples/BLE/BLE.ino)** - BLE mode setup with WiFi provisioning
- **[ClassicBT.ino](examples/ClassicBT/ClassicBT.ino)** - Classic Bluetooth mode setup
- **[CustomUUID.ino](examples/CustomUUID/CustomUUID.ino)** - BLE with custom UUIDs

## Bluetooth App Requirements

### For Classic Bluetooth Mode
Any standard Bluetooth serial terminal app works:
- **Android:** Bluetooth Terminal, Serial Bluetooth Terminal, HC-05 Bluetooth Terminal
- **iOS:** LightBlue (limited support), or use a Mac/PC with Bluetooth
- **PC/Mac:** Built-in Bluetooth with serial terminal software

### 🔴 IMPORTANT: BLE Mode requires a BLE-compatible app 🔴

When using BLE mode, you **must use a BLE (Bluetooth Low Energy) compatible application**. Standard Bluetooth terminal apps will NOT work with BLE mode.

**Recommended BLE Apps:**
- **Android:** nRF Connect (official Nordic app), BLE Scanner, LightBlue
- **iOS:** nRF Connect, LightBlue, BLE Scanner apps from App Store
- **PC/Mac:** nRF Connect Desktop (cross-platform), or built-in BLE support with appropriate software

**Note:** The nRF Connect app is the most widely used and recommended BLE application across all platforms. It provides a user-friendly interface specifically designed for BLE device communication and is available for Android, iOS, and desktop platforms.

## Project Structure

```
EZConnect/
├── src/
│   ├── EZConnect.h
│   └── EZConnect.cpp
├── examples/
│   ├── BLE/
│   │   └── BLE.ino
│   ├── ClassicBT/
│   │   └── ClassicBT.ino
│   └── CustomUUID/
│       └── CustomUUID.ino
├── library.properties
├── LICENSE
└── README.md
```

## Contributing

Contributions are welcome! Here's how you can help:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

Found a bug? Please [open an issue](https://github.com/Aatiqur/EZConnect/issues).

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

You are free to use, modify, and distribute this library.

## Author

**Atiqur Rahman**

- Embedded Systems, Robotics, and IoT
- Email: aatiqurrahman111@gmail.com
- LinkedIn: [linkedin.com/in/aatiqurrahman](https://www.linkedin.com/in/aatiqurrahman/)

## Support

If you find this project useful, please consider:

- Giving it a star on GitHub
- Reporting bugs and issues
- Suggesting new features
- Contributing to the codebase

---

<div align="center">

**Developed for the ESP32 community**

</div>
