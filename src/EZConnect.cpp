#include "EZConnect.h"

// ===== Static BLE data =====
String EZConnect::_rxData = "";
bool EZConnect::_newData = false;

// ===== BLE callbacks =====
class RXCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *c) override {
    EZConnect::_onBLEData(c->getValue());
  }
};

class ServerCallbacks : public BLEServerCallbacks {
  void onDisconnect(BLEServer *s) override {
    s->getAdvertising()->start();
  }
};

// ===== Constructor =====
EZConnect::EZConnect(EZConnectMode mode,
                     const char *deviceName,
                     uint8_t ledPin,
                     uint8_t buttonPin) {

  _deviceName = deviceName;
  _ledPin = ledPin;
  _btnPin = buttonPin;

  // Default UUIDs (used unless overridden)
  _bleServiceUUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
  _bleRxUUID      = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";

#if EZCONNECT_CLASSIC_BT_SUPPORTED
  _mode = mode;
#else
  _mode = EZCONNECT_BLE;
  Serial.println("[EZConnect] Classic BT not supported on this board");
  Serial.println("[EZConnect] Falling back to BLE");
#endif
}

// ===== Optional UUID override (SAFE before setup) =====
void EZConnect::setUUIDs(const char *serviceUUID,
                         const char *rxCharUUID) {
  _bleServiceUUID = serviceUUID;
  _bleRxUUID = rxCharUUID;
}

// ===== Debug =====
void EZConnect::showDebugmsg() {
  _debug = true;
}

// ===== Begin (Arduino runtime starts here) =====
void EZConnect::begin() {
  pinMode(_ledPin, OUTPUT);
  pinMode(_btnPin, INPUT_PULLUP);
  digitalWrite(_ledPin, LOW);

  if (_debug) {
    Serial.println("[EZConnect] Initialized");
    Serial.print("[EZConnect] Device: ");
    Serial.println(_deviceName);
  }

  if (_mode == EZCONNECT_BLE) {
    BLEDevice::init(_deviceName);
  }
}

// ===== WiFi credentials =====
String EZConnect::getSSID() {
  _prefs.begin("wifi", true);
  String s = _prefs.getString("ssid", "");
  _prefs.end();
  return s;
}

String EZConnect::getPass() {
  _prefs.begin("wifi", true);
  String p = _prefs.getString("pass", "");
  _prefs.end();
  return p;
}

// ===== WiFi connection check =====
void EZConnect::checkConnectivity(uint32_t wifiTimeout,
                                  uint32_t blinkOn,
                                  uint32_t blinkOff) {
  _blinkOn = blinkOn;
  _blinkOff = blinkOff;

  String ssid = getSSID();
  if (_debug && ssid.length()) {
    Serial.print("[EZConnect] Connecting to ");
    Serial.println(ssid);
  }

  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED) {
    blink(blinkOn, blinkOff);

    if (millis() - t0 > wifiTimeout) {
      _configMode = true;
      if (_debug) {
        Serial.println("[EZConnect] WiFi failed, entering config mode");
      }
      break;
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(_ledPin, LOW);
    if (_debug) {
      Serial.println("[EZConnect] WiFi Connected");
      Serial.println(WiFi.localIP());
    }
  }
}

// ===== Loop =====
void EZConnect::check() {
  checkButton();

  if (!_configMode) return;

  blink(150, 150);

#if EZCONNECT_CLASSIC_BT_SUPPORTED
  if (_mode == EZCONNECT_CLASSIC_BT && _bt.available()) {
    handleConfigInput(_bt.readString());
  }
#endif

  if (_mode == EZCONNECT_BLE && _newData) {
    _newData = false;
    handleConfigInput(_rxData);
  }
}

// ===== Button =====
void EZConnect::checkButton() {
  static uint32_t t0 = 0;
  static bool active = false;

  if (digitalRead(_btnPin) == LOW) {
    if (!active) {
      t0 = millis();
      active = true;
    }

    if (millis() - t0 >= 3000) {
      _configMode = true;

      if (_debug) {
        Serial.println(_mode == EZCONNECT_BLE
          ? "[EZConnect] BLE Enabled"
          : "[EZConnect] Classic BT Enabled");
      }

      (_mode == EZCONNECT_BLE) ? startBLE() : startClassicBT();
      active = false;
    }
  } else {
    active = false;
  }
}

// ===== Classic BT =====
void EZConnect::startClassicBT() {
#if EZCONNECT_CLASSIC_BT_SUPPORTED
  _bt.begin(_deviceName);
#endif
}

// ===== BLE =====
void EZConnect::startBLE() {
  BLEServer *server = BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  BLEService *service = server->createService(_bleServiceUUID);
  BLECharacteristic *rx =
    service->createCharacteristic(_bleRxUUID,
                                   BLECharacteristic::PROPERTY_WRITE);

  rx->setCallbacks(new RXCallbacks());
  service->start();

  BLEAdvertising *adv = BLEDevice::getAdvertising();
  adv->addServiceUUID(_bleServiceUUID);
  adv->start();
}

// ===== BLE data =====
void EZConnect::_onBLEData(const String &data) {
  _rxData = data;
  _newData = true;
}

// ===== Store credentials =====
void EZConnect::handleConfigInput(String input) {
  input.trim();

  if (_debug) {
    Serial.print("[EZConnect] Received: ");
    Serial.println(input);
  }

  String ssid, pass;
  if (!parseCredentials(input, ssid, pass)) {
    if (_debug) Serial.println("[EZConnect] Invalid format");
    return;
  }

  _prefs.begin("wifi", false);
  _prefs.putString("ssid", ssid);
  _prefs.putString("pass", pass);
  _prefs.end();

  if (_debug) Serial.println("[EZConnect] Saved, restarting...");
  delay(500);
  ESP.restart();
}

bool EZConnect::parseCredentials(String in, String &s, String &p) {
  int i = in.indexOf(',');
  if (i < 0) return false;
  s = in.substring(0, i);
  p = in.substring(i + 1);
  return s.length() && p.length();
}

// ===== LED blink =====
void EZConnect::blink(uint32_t onTime, uint32_t offTime) {
  static uint32_t t = 0;
  static bool state = false;
  uint32_t interval = state ? onTime : offTime;

  if (millis() - t >= interval) {
    t = millis();
    state = !state;
    digitalWrite(_ledPin, state);
  }
}
