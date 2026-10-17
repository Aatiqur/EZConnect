 <div align="center">

# EZConnect

**Easy Wi-Fi Configuration for ESP32 via Classic Bluetooth, BLE, and Wi-Fi Portal**

[![Arduino](https://img.shields.io/badge/Arduino-Compatible-00979D?style=flat&logo=arduino)](https://www.arduino.cc/)
[![ESP32](https://img.shields.io/badge/Platform-ESP32-E7352C?style=flat&logo=espressif)](https://www.espressif.com/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Version](https://img.shields.io/badge/version-1.2.2-blue.svg)](https://github.com/Aatiqur/EZConnect)

*Simplify Wi-Fi provisioning with Classic Bluetooth, BLE, or a browser-based setup portal.*

[Features](#features) • [Why EZConnect?](#why-ezconnect) • [Installation](#installation) • [Quick Start](#quick-start) • [Examples](#examples) • [Documentation](#documentation)

</div>

---

## About

EZConnect is a versatile Arduino/ESP32 library that simplifies Wi-Fi configuration using **Classic Bluetooth, BLE (Bluetooth Low Energy), or a browser-based Wi-Fi configuration portal**.

The library allows users to configure Wi-Fi credentials wirelessly without hardcoding the network name or password into their sketches.

With Classic Bluetooth and BLE modes, users can send Wi-Fi credentials through compatible Bluetooth applications. With the Wi-Fi Portal mode introduced in **version 1.2.2**, users can connect to an ESP32 setup hotspot, open a web browser, select a nearby Wi-Fi network, and submit its password through a web interface.

The library supports different ESP32 variants according to their Bluetooth hardware availability and provides a convenient configuration workflow for IoT projects, smart devices, and wireless systems.

## Why EZConnect?

### Flexible Wi-Fi Provisioning

EZConnect provides three configuration modes:

- **Classic Bluetooth:** Configure Wi-Fi through a standard Bluetooth serial terminal application.
- **BLE:** Configure Wi-Fi using a Bluetooth Low Energy-compatible application.
- **Wi-Fi Portal:** Configure Wi-Fi through a browser-based interface hosted by the ESP32.

Choose the mode that best fits your hardware and project requirements.

### Universal Bluetooth Support

Not all ESP32 variants support the same Bluetooth hardware:

- **Generic ESP32** — Supports both Classic Bluetooth and BLE.
- **ESP32-C3 / ESP32-C6** — BLE only; no Classic Bluetooth.
- **ESP32-S2** — No Bluetooth support.
- **ESP32-S3** — BLE only; no Classic Bluetooth.

The Wi-Fi Portal mode provides an alternative for supported ESP32 boards that do not have Bluetooth hardware.

### Developer and Hobbyist Friendly

- **No hardcoded Wi-Fi credentials** — Configure credentials wirelessly.
- **Multiple provisioning methods** — Classic Bluetooth, BLE, or a browser-based portal.
- **No custom app required for portal mode** — Use a phone or computer web browser.
- **Persistent storage** — Wi-Fi credentials are stored in device memory.
- **Automatic reconnection** — Reuse saved credentials after reboot.
- **Configurable connection timeout** — Set the Wi-Fi connection timeout.
- **Status LED support** — Get visual feedback on connection status.
- **Simple API** — Minimal setup required.
- **Browser-based configuration** — Scan nearby Wi-Fi networks and select one through the portal.
- **Modern portal interface** — Includes credential submission feedback and a loading indicator.

## Features

- **Three Wi-Fi provisioning modes**
  - Classic Bluetooth — Works with standard Bluetooth serial applications.
  - BLE (Bluetooth Low Energy) — Requires a BLE-compatible application.
  - Wi-Fi Portal — Configure Wi-Fi through a browser-based setup page.

- **Wi-Fi Portal (New in v1.2.2)**
  - Configurable setup hotspot SSID and password.
  - Configurable portal title.
  - Default portal title: `EZConnect`.
  - Scans nearby Wi-Fi networks for selection.
  - Allows users to submit their Wi-Fi credentials through a web form.
  - Displays loading feedback after credentials are submitted.
  - Saves submitted credentials and restarts the ESP32 to apply them.
  - Displays a confirmation page after credential submission.
  - Includes the “Powered by EZConnect” footer.

- **Persistent storage** — Stores Wi-Fi credentials using Preferences.
- **Automatic reconnection** — Reconnects using saved credentials.
- **Configurable timeout** — Set your own Wi-Fi connection timeout.
- **Status LED support** — Visual feedback for connection state.
- **Simple API** — Minimal setup required.
- **Debug messages** — Enable library messages in the Serial Monitor.
- **Non-blocking design** — Uses `millis()` for efficient operation where applicable.

## Supported Boards and Bluetooth Modes

| Board | Classic Bluetooth | BLE | Wi-Fi Portal |
|---|---|---|---|
| ESP32 (Generic) | Yes | Yes | Yes |
| ESP32-C3 | No | Yes | Yes |
| ESP32-C6 | No | Yes | Yes |
| ESP32-S2 | No | No | Yes |
| ESP32-S3 | No | Yes | Yes |

**Note:** Wi-Fi Portal mode uses Wi-Fi rather than Bluetooth for provisioning. Actual compatibility depends on the board's Wi-Fi hardware and the ESP32 Arduino core.

For ESP32-C3, ESP32-C6, and ESP32-S3, use BLE mode if you choose Bluetooth provisioning. The original ESP32 supports either Bluetooth mode. Wi-Fi Portal mode is an alternative when browser-based setup is preferred.

## Installation

### Method 1: Arduino Library Manager (Recommended)

1. Open Arduino IDE.
2. Go to **Sketch → Include Library → Manage Libraries**.
3. Search for **EZConnect**.
4. Click **Install**.

### Method 2: Manual Installation

1. Download this repository as a ZIP file.
2. Open Arduino IDE.
3. Go to **Sketch → Include Library → Add .ZIP Library**.
4. Select the downloaded ZIP file.

---

## 🔴 CRITICAL STEP FOR GENERIC ESP32 DEV BOARDS — DO NOT SKIP! 🔴

> **Important partition configuration**
>
> After installing the library, check the partition scheme for your ESP32 board.
>
> ### Required steps
>
> 1. In Arduino IDE, go to **Tools → Partition Scheme**.
> 2. Check whether your current partition provides sufficient application space.
> 3. If required, select **Huge APP (3MB No OTA/1MB SPIFFS)** instead of **Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)**.
>
> **Why might this be necessary?**
>
> Bluetooth and Wi-Fi functionality can increase application size. If compilation or uploading fails because the application exceeds the available partition space, select a suitable partition scheme for your board.
>
> Partition options vary between ESP32 boards and Arduino core versions.

---

## Quick Start

### Classic Bluetooth Mode (Standard Bluetooth Terminal App)

```cpp
#include <EZConnect.h>

// Initialize with Classic Bluetooth mode.
EZConnect ez(EZCONNECT_CLASSIC_BT, "EZConnect-Device", 2, 0);

void setup() {
  Serial.begin(115200);

  ez.begin();
  ez.showDebugmsg();

  // Retrieve the saved Wi-Fi credentials.
  String ssid = ez.getSSID();
  String pass = ez.getPass();

  // Connect to Wi-Fi using the retrieved credentials.
  WiFi.begin(ssid.c_str(), pass.c_str());

  // Parameters:
  // 10000 = Wi-Fi connection timeout in milliseconds (10 seconds).
  // 500    = LED ON interval in milliseconds.
  // 500    = LED OFF interval in milliseconds.
  ez.checkConnectivity(10000, 500, 500);
}

void loop() {
  // Process incoming Bluetooth data and maintain EZConnect.
  ez.check();
}
```

### BLE Mode (Requires a BLE-Compatible App)

```cpp
#include <EZConnect.h>

// Initialize with BLE mode.
EZConnect ez(EZCONNECT_BLE, "EZConnect-Device", 2, 0);

void setup() {
  Serial.begin(115200);

  ez.begin();
  ez.showDebugmsg();

  // Retrieve the saved Wi-Fi credentials.
  String ssid = ez.getSSID();
  String pass = ez.getPass();

  // Connect to Wi-Fi using the retrieved credentials.
  WiFi.begin(ssid.c_str(), pass.c_str());

  // Parameters: timeout (ms), LED ON interval (ms), LED OFF interval (ms).
  ez.checkConnectivity(10000, 500, 500);
}

void loop() {
  // Process incoming BLE data and maintain EZConnect.
  ez.check();
}
```

### Wi-Fi Portal Mode (New in v1.2.2)

The Wi-Fi Portal mode allows users to configure their Wi-Fi connection using a web browser instead of a Bluetooth application.

```cpp
#include <EZConnect.h>

// Arguments: mode, hotspot SSID, hotspot password, status LED pin,
// setup button pin, optional portal title.
EZConnect ez(EZCONNECT_WIFI_PORTAL,
             "EZConnect-Setup",
             "ChangeThis123",  // Replace with a unique password (8+ characters).
             2,                 // Status LED pin; use 0 if no LED is connected.
             9,                 // Setup button pin; use 0 if no button is connected.
             "My Device Setup"); // Optional portal title.

void setup() {
  Serial.begin(115200);

  // Initialize EZConnect and start its Wi-Fi provisioning workflow.
  ez.begin();

  // Enable EZConnect debug messages in the Serial Monitor.
  ez.showDebugmsg();

  // Read the saved Wi-Fi credentials.
  String ssid = ez.getSSID();
  String pass = ez.getPass();

  // Attempt to connect using the retrieved credentials.
  WiFi.begin(ssid.c_str(), pass.c_str());

  // Check Wi-Fi connectivity.
  // Parameters:
  // 10000 = connection timeout in milliseconds (10 seconds).
  // 500   = LED ON interval in milliseconds while connecting.
  // 500   = LED OFF interval in milliseconds while connecting.
  ez.checkConnectivity(10000, 500, 500);
}

void loop() {
  // Maintain EZConnect's portal and background processing.
  ez.check();
}
```

**Important:** The example above follows the credential-retrieval and connection workflow. The exact timing of portal submission, credential persistence, and restart depends on the library's portal implementation. If the portal restarts the ESP32 after saving credentials, the connection code will run again on the next boot.

### Wi-Fi Portal Setup Procedure

1. Upload the Wi-Fi Portal example to the ESP32.
2. Power on or reset the device.
3. On your phone or computer, open the Wi-Fi settings.
4. Connect to the setup hotspot configured in the constructor, such as `EZConnect-Setup`.
5. Enter the hotspot password configured in the example.
6. Open a browser and navigate to the portal address shown by EZConnect. The default access address is commonly `http://192.168.4.1`.
7. Select the Wi-Fi network you want the ESP32 to join.
8. Enter the network password.
9. Submit the form and wait for the loading indicator and confirmation message.
10. Allow the ESP32 to restart and attempt to connect using the saved credentials.

**Security note:** Change the example hotspot password to a unique password before deploying the device.

### Choosing Between Modes

- **Classic Bluetooth:** Suitable for the original ESP32 and projects using Bluetooth serial terminal apps.
- **BLE:** Suitable for BLE-capable ESP32 boards and BLE applications.
- **Wi-Fi Portal:** Suitable when users prefer configuring the device through a browser without a Bluetooth app.

## How It Works

### Bluetooth Provisioning

1. The ESP32 boots and checks for saved Wi-Fi credentials.
2. If saved credentials are available, the application attempts to connect to Wi-Fi.
3. If provisioning is needed, the selected Bluetooth mode is used.
4. The user opens a Bluetooth terminal app for Classic Bluetooth or a BLE-compatible app for BLE mode.
5. The user sends Wi-Fi credentials.
6. EZConnect stores the credentials in persistent memory.
7. The ESP32 attempts to connect to Wi-Fi.
8. The LED can provide visual feedback on the connection state.
9. Saved credentials can be reused after reboot.

### Wi-Fi Portal Provisioning (v1.2.2)

1. The ESP32 starts the Wi-Fi provisioning workflow.
2. The device creates the configured setup hotspot when portal setup is required.
3. The user connects a phone or computer to the hotspot.
4. The user opens the setup page in a web browser.
5. The portal scans nearby Wi-Fi networks and displays available networks for selection.
6. The user selects a network and enters its password.
7. The portal displays a loading state while the credentials are submitted.
8. EZConnect saves the submitted credentials and follows its configured restart/connection workflow.
9. The ESP32 attempts to connect using the saved credentials.
10. On subsequent boots, saved credentials can be reused.

### Supported Bluetooth Credential Formats

For Bluetooth modes, send Wi-Fi credentials from your Bluetooth application using one of these formats:

| Format | Example |
|---|---|
| Newline separator | `MyWiFi\n12345678` |
| Pipe separator | `MyWiFi\|12345678` |
| Tab separator | `MyWiFi	12345678` |
| Comma separator | `MyWiFi,12345678` |

Replace `MyWiFi` with your Wi-Fi SSID and `12345678` with your Wi-Fi password.

The browser-based Wi-Fi Portal mode uses its web form instead of these Bluetooth message formats.

## Documentation

### Constructors

#### Classic Bluetooth Mode

```cpp
EZConnect(EZCONNECT_CLASSIC_BT,
          const char* deviceName,
          uint8_t ledPin,
          uint8_t statusPin);
```

#### BLE Mode

```cpp
EZConnect(EZCONNECT_BLE,
          const char* deviceName,
          uint8_t ledPin,
          uint8_t statusPin);
```

#### Wi-Fi Portal Mode (v1.2.2)

```cpp
EZConnect(EZCONNECT_WIFI_PORTAL,
          const char* hotspotSSID,
          const char* hotspotPassword,
          uint8_t ledPin,
          uint8_t setupButtonPin,
          const char* portalTitle);
```

The portal title is optional. When omitted, the default title is `EZConnect`.

### Constructor Parameters

| Parameter | Type | Description |
|---|---|---|
| `mode` | Enum | `EZCONNECT_CLASSIC_BT`, `EZCONNECT_BLE`, or `EZCONNECT_WIFI_PORTAL` |
| `deviceName` | `const char*` | Bluetooth/BLE device name |
| `hotspotSSID` | `const char*` | SSID of the setup hotspot in portal mode |
| `hotspotPassword` | `const char*` | Password for the setup hotspot; use at least 8 characters |
| `ledPin` | `uint8_t` | GPIO pin for the status LED; `0` disables it |
| `statusPin` | `uint8_t` | GPIO pin for the status indicator in Bluetooth constructors; `0` disables it |
| `setupButtonPin` | `uint8_t` | GPIO pin used for the setup button in portal mode; `0` disables it if supported by the implementation |
| `portalTitle` | `const char*` | Optional title shown on the Wi-Fi setup page; defaults to `EZConnect` when omitted |

### Methods

```cpp
void begin();
// Initialize EZConnect using the selected mode.

String getSSID();
// Get the stored Wi-Fi SSID.

String getPass();
// Get the stored Wi-Fi password.

void check();
// Process incoming Bluetooth data or maintain portal/background processing.

void showDebugmsg();
// Enable debug messages in the Serial Monitor.

void checkConnectivity(int wifiTimeout, int blinkOn, int blinkOff);
// Check Wi-Fi connectivity with LED feedback.
// wifiTimeout: Wi-Fi connection timeout in milliseconds.
// blinkOn: LED ON duration in milliseconds.
// blinkOff: LED OFF duration in milliseconds.

void setUUIDs(const char* serviceUUID, const char* rxCharUUID);
// BLE only: Set custom UUIDs for the BLE service and RX characteristic.
```

### Debug Messages

Call `ez.showDebugmsg()` to enable EZConnect debug output.

Messages use the following format:

```text
[EZConnect] -> Initialized
[EZConnect] -> Mode: Wi-Fi Portal
[EZConnect] -> Setup hotspot: EZConnect-Setup
[EZConnect] -> Open http://192.168.4.1
```

The exact messages depend on the selected mode and the state of the device.

## Examples

Check out the [examples](examples/) folder for detailed implementations:

- **[BLE.ino](examples/BLE/BLE.ino)** — BLE mode setup with Wi-Fi provisioning.
- **[ClassicBT.ino](examples/ClassicBT/ClassicBT.ino)** — Classic Bluetooth mode setup.
- **[CustomUUID.ino](examples/CustomUUID/CustomUUID.ino)** — BLE with custom UUIDs.
- **[WiFiPortal.ino](examples/WiFiPortal/WiFiPortal.ino)** — Browser-based Wi-Fi provisioning with a configurable setup hotspot.

## Bluetooth App Requirements

### For Classic Bluetooth Mode

Any standard Bluetooth serial terminal app can be used:

- **Android:** Bluetooth Terminal, Serial Bluetooth Terminal, HC-05 Bluetooth Terminal.
- **iOS:** LightBlue (limited support), or a Mac/PC with Bluetooth.
- **PC/Mac:** Bluetooth-capable serial terminal software.

### 🔴 IMPORTANT: BLE Mode Requires a BLE-Compatible App 🔴

When using BLE mode, you **must use a Bluetooth Low Energy-compatible application**. Standard Bluetooth serial terminal apps will not work with BLE mode.

**Recommended BLE apps:**

- **Android:** nRF Connect, BLE Scanner, LightBlue.
- **iOS:** nRF Connect, LightBlue, BLE Scanner.
- **PC/Mac:** nRF Connect Desktop or other compatible BLE software.

The nRF Connect app is a widely used BLE application for discovering services and exchanging data with BLE devices.

### For Wi-Fi Portal Mode

No Bluetooth application is required.

You need:

- A phone, tablet, or computer with Wi-Fi.
- A web browser.
- The setup hotspot SSID and password configured in the example.

Connect to the setup hotspot, open the portal address displayed by EZConnect, and follow the on-screen instructions.

## Project Structure

```text
EZConnect/
├── src/
│   ├── EZConnect.h
│   └── EZConnect.cpp
├── examples/
│   ├── BLE/
│   │   └── BLE.ino
│   ├── ClassicBT/
│   │   └── ClassicBT.ino
│   ├── CustomUUID/
│   │   └── CustomUUID.ino
│   └── WiFiPortal/
│       └── WiFiPortal.ino
├── library.properties
├── LICENSE
└── README.md
```

## Contributing

Contributions are welcome! Here's how you can help:

1. Fork the repository.
2. Create a feature branch (`git checkout -b feature/AmazingFeature`).
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`).
4. Push to the branch (`git push origin feature/AmazingFeature`).
5. Open a Pull Request.

Found a bug? Please [open an issue](https://github.com/Aatiqur/EZConnect/issues).

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

You are free to use, modify, and distribute this library under the terms of the license.

## Author

**Atiqur Rahman**

- Embedded Systems, Robotics, and IoT
- Email: aatiqurrahman111@gmail.com
- LinkedIn: [linkedin.com/in/aatiqurrahman](https://www.linkedin.com/in/aatiqurrahman/)

## Support

If you find this project useful, please consider:

- Giving this project a star on GitHub.
- Reporting bugs and issues.
- Suggesting new features.
- Contributing to the codebase.

---

<div align="center">

**Developed for the ESP32 community**

</div>