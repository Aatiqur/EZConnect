#ifndef EZCONNECT_H
#define EZCONNECT_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

// Classic Bluetooth is available only on the original ESP32 target.
#if defined(CONFIG_IDF_TARGET_ESP32)
  #include <BluetoothSerial.h>
  #define EZCONNECT_CLASSIC_BT_SUPPORTED 1
#else
  #define EZCONNECT_CLASSIC_BT_SUPPORTED 0
#endif

enum EZConnectMode {
  EZCONNECT_CLASSIC_BT,
  EZCONNECT_BLE,
  EZCONNECT_WIFI_PORTAL
};

class EZConnect {
public:
  // Existing constructor: Classic Bluetooth or BLE. Kept for compatibility.
  EZConnect(EZConnectMode mode,
            const char *deviceName,
            uint8_t ledPin,
            uint8_t buttonPin);

  // New Wi-Fi portal constructor. portalTitle is optional and defaults to "EZConnect".
  EZConnect(EZConnectMode mode,
            const char *portalSSID,
            const char *portalPassword,
            uint8_t ledPin,
            uint8_t buttonPin,
            const char *portalTitle = "EZConnect");

  // Optional BLE UUID override; call before begin().
  void setUUIDs(const char *serviceUUID, const char *rxCharUUID);

  void showDebugmsg();
  void begin();
  String getSSID();
  String getPass();
  void checkConnectivity(uint32_t wifiTimeout, uint32_t blinkOn, uint32_t blinkOff);
  void check();

  // Optional manual control for the browser portal.
  bool startWiFiPortal();
  void stopWiFiPortal();

  static void _onBLEData(const String &data);

private:
  EZConnectMode _mode;
  String _deviceName;
  String _portalSSID;
  String _portalPassword;
  String _portalTitle;
#if EZCONNECT_CLASSIC_BT_SUPPORTED
  BluetoothSerial _bt;
#endif

  Preferences _prefs;
  WebServer _webServer{80};
  DNSServer _dnsServer;

  uint8_t _ledPin;
  uint8_t _btnPin;
  uint32_t _blinkOn = 500;
  uint32_t _blinkOff = 500;

  bool _configMode = false;
  bool _debug = false;
  bool _portalActive = false;
  bool _buttonTriggered = false;

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
  bool startPortalAP();
  void configureWebRoutes();
  void handlePortalRoot();
  void handlePortalConnect();
  void trySavedWiFiOrStartPortal();
};

#endif
