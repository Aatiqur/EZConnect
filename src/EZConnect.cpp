#include "EZConnect.h"

EZConnect::EZConnect(uint8_t ledPin,
                     uint8_t buttonPin,
                     const char* btName)
: _ledPin(ledPin),
  _buttonPin(buttonPin),
  _btName(btName) {

  _wifiChecking = false;
}

void EZConnect::begin() {
  pinMode(_ledPin, OUTPUT);
  pinMode(_buttonPin, INPUT_PULLUP);
  digitalWrite(_ledPin, LOW);

  _prefs.begin("wifi", true);
  _ssid = _prefs.getString("ssid", "");
  _pass = _prefs.getString("pass", "");
  _prefs.end();
}

// ================= USER GETTERS =================
String EZConnect::getSSID() { return _ssid; }
String EZConnect::getPass() { return _pass; }

// ================= WIFI CHECK =================
void EZConnect::checkConnectivity(unsigned long wifiTimeout,
                                  unsigned long blinkOn,
                                  unsigned long blinkOff) {

  _wifiTimeout = wifiTimeout;
  _blinkOn = blinkOn;
  _blinkOff = blinkOff;

  _wifiStart = millis();
  _wifiChecking = true;
}

// ================= LOOP HANDLER =================
void EZConnect::check() {
  handleButton();

  if (_wifiChecking && WiFi.status() != WL_CONNECTED) {
    blinkLED();

    if (millis() - _wifiStart >= _wifiTimeout) {
      _wifiChecking = false;
      startBluetoothConfig();
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(_ledPin, LOW);
    _wifiChecking = false;
  }
}

// ================= BUTTON =================
void EZConnect::handleButton() {
  if (digitalRead(_buttonPin) == LOW) {
    if (!_buttonActive) {
      _pressStart = millis();
      _buttonActive = true;
    }

    if (millis() - _pressStart >= _longPressTime) {
      startBluetoothConfig();
      _buttonActive = false;
    }
  } else {
    _buttonActive = false;
  }
}

// ================= LED =================
void EZConnect::blinkLED() {
  static unsigned long lastMillis = 0;
  static bool state = false;

  unsigned long interval = state ? _blinkOn : _blinkOff;

  if (millis() - lastMillis >= interval) {
    lastMillis = millis();
    state = !state;
    digitalWrite(_ledPin, state);
  }
}

// ================= BLUETOOTH =================
void EZConnect::startBluetoothConfig() {
  _bt.begin(_btName);

  // Fixed BT blink rate
  _blinkOn = 150;
  _blinkOff = 150;

  while (!_bt.available()) {
    blinkLED();
    delay(10);
  }

  String input = _bt.readString();
  input.trim();

  String ssid, pass;
  if (!parseCredentials(input, ssid, pass)) {
    _bt.end();
    return;
  }

  _prefs.begin("wifi", false);
  _prefs.putString("ssid", ssid);
  _prefs.putString("pass", pass);
  _prefs.end();

  _bt.end();
  delay(500);
  ESP.restart();
}

// ================= PARSER =================
bool EZConnect::parseCredentials(String input,
                                 String &ssid,
                                 String &pass) {
  int i;

  if ((i = input.indexOf('\n')) > 0 ||
      (i = input.indexOf('|')) > 0 ||
      (i = input.indexOf('\t')) > 0 ||
      ((i = input.indexOf(',')) > 0 && input.lastIndexOf(',') == i)) {

    ssid = input.substring(0, i);
    pass = input.substring(i + 1);
    return ssid.length() && pass.length();
  }

  return false;
}
