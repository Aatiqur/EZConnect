#ifndef EZCONNECT_H
#define EZCONNECT_H

#include <Arduino.h>
#include <WiFi.h>
#include <BluetoothSerial.h>
#include <Preferences.h>

class EZConnect {
  public:
    EZConnect(uint8_t ledPin,
              uint8_t buttonPin,
              const char* btName = "ESP32_Setup");

    void begin();

    // User access
    String getSSID();
    String getPass();

    // Connectivity
    void checkConnectivity(unsigned long wifiTimeout,
                           unsigned long blinkOn,
                           unsigned long blinkOff);

    void check();  // call inside loop()

  private:
    // Pins
    uint8_t _ledPin;
    uint8_t _buttonPin;

    // WiFi
    String _ssid;
    String _pass;
    unsigned long _wifiTimeout;
    unsigned long _wifiStart;
    bool _wifiChecking;

    // LED
    unsigned long _blinkOn;
    unsigned long _blinkOff;

    // Button
    unsigned long _pressStart;
    bool _buttonActive;
    const unsigned long _longPressTime = 3000;

    // Bluetooth
    const char* _btName;
    BluetoothSerial _bt;

    // Preferences
    Preferences _prefs;

    // Internal
    void blinkLED();
    void handleButton();
    void startBluetoothConfig();
    bool parseCredentials(String input, String &ssid, String &pass);
};

#endif
