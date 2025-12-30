#ifndef EZCONNECT_H
#define EZCONNECT_H

#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

// ===== Classic BT only on original ESP32 =====
#if defined(CONFIG_IDF_TARGET_ESP32)
  #include <BluetoothSerial.h>
  #define EZCONNECT_CLASSIC_BT_SUPPORTED 1
#else
  #define EZCONNECT_CLASSIC_BT_SUPPORTED 0
#endif

enum EZConnectMode {
  EZCONNECT_CLASSIC_BT,
  EZCONNECT_BLE
};

class EZConnect {
public:
  // ===== Constructor (safe at global scope) =====
  EZConnect(EZConnectMode mode,
            const char *deviceName,
            uint8_t ledPin,
            uint8_t buttonPin);

  // ===== OPTIONAL (safe before setup) =====
  void setUUIDs(const char *serviceUUID,
                const char *rxCharUUID);

  // ===== Runtime =====
  void showDebugmsg();
  void begin();

  String getSSID();
  String getPass();

  void checkConnectivity(uint32_t wifiTimeout,
                         uint32_t blinkOn,
                         uint32_t blinkOff);

  void check();

  static void _onBLEData(const String &data);

private:
  EZConnectMode _mode;
  const char *_deviceName;

#if EZCONNECT_CLASSIC_BT_SUPPORTED
  BluetoothSerial _bt;
#endif

  Preferences _prefs;

  uint8_t _ledPin;
  uint8_t _btnPin;

  uint32_t _blinkOn;
  uint32_t _blinkOff;

  bool _configMode = false;
  bool _debug = false;

  // ===== BLE UUIDs (can be overridden anytime before begin()) =====
  const char *_bleServiceUUID;
  const char *_bleRxUUID;

  static String _rxData;
  static bool _newData;

  void startClassicBT();
  void startBLE();

  void handleConfigInput(String input);
  bool parseCredentials(String input, String &ssid, String &pass);
  void blink(uint32_t onTime, uint32_t offTime);
  void checkButton();
};

#endif
