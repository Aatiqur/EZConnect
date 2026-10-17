#include "EZConnect.h"

String EZConnect::_rxData = "";
bool EZConnect::_newData = false;

namespace {
String htmlEscape(const String &value) {
  String out;
  out.reserve(value.length() + 8);
  for (size_t i = 0; i < value.length(); ++i) {
    switch (value[i]) {
      case '&': out += F("&amp;"); break;
      case '<': out += F("&lt;"); break;
      case '>': out += F("&gt;"); break;
      case '"': out += F("&quot;"); break;
      case '\'': out += F("&#39;"); break;
      default: out += value[i]; break;
    }
  }
  return out;
}

String signalLabel(int32_t rssi) {
  if (rssi >= -55) return "Excellent signal";
  if (rssi >= -67) return "Good signal";
  if (rssi >= -75) return "Fair signal";
  return "Weak signal";
}
}

class RXCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *characteristic) override {
    EZConnect::_onBLEData(characteristic->getValue());
  }
};

class ServerCallbacks : public BLEServerCallbacks {
  void onDisconnect(BLEServer *server) override {
    server->getAdvertising()->start();
  }
};

// Original constructor preserved for existing sketches.
EZConnect::EZConnect(EZConnectMode mode, const char *deviceName,
                     uint8_t ledPin, uint8_t buttonPin) {
  _deviceName = deviceName ? deviceName : "EZConnect-Device";
  _portalSSID = "EZConnect-Setup";
  _portalPassword = "";
  _portalTitle = "EZConnect";
  _ledPin = ledPin;
  _btnPin = buttonPin;
  _bleServiceUUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
  _bleRxUUID = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";

  if (mode == EZCONNECT_WIFI_PORTAL) {
    // A portal needs explicit hotspot credentials; use the new constructor.
    _mode = EZCONNECT_WIFI_PORTAL;
  } else {
#if EZCONNECT_CLASSIC_BT_SUPPORTED
    _mode = mode;
#else
    _mode = EZCONNECT_BLE;
    Serial.println("[EZConnect] -> Classic Bluetooth is not supported on this board.");
    Serial.println("[EZConnect] -> Falling back to BLE.");
#endif
  }
}

// New constructor for Wi-Fi provisioning portal.
EZConnect::EZConnect(EZConnectMode mode, const char *portalSSID,
                     const char *portalPassword, uint8_t ledPin,
                     uint8_t buttonPin, const char *portalTitle) {
  _mode = mode;
  _deviceName = portalSSID ? portalSSID : "EZConnect-Setup";
  _portalSSID = (portalSSID && portalSSID[0]) ? portalSSID : "EZConnect-Setup";
  _portalPassword = portalPassword ? portalPassword : "";
  _portalTitle = (portalTitle && portalTitle[0]) ? portalTitle : "EZConnect";
  _ledPin = ledPin;
  _btnPin = buttonPin;
  _bleServiceUUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
  _bleRxUUID = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";
}

void EZConnect::setUUIDs(const char *serviceUUID, const char *rxCharUUID) {
  if (serviceUUID) _bleServiceUUID = serviceUUID;
  if (rxCharUUID) _bleRxUUID = rxCharUUID;
}

void EZConnect::showDebugmsg() { _debug = true; }

void EZConnect::begin() {
  if (_ledPin != 0) {
    pinMode(_ledPin, OUTPUT);
    digitalWrite(_ledPin, LOW);
  }
  if (_btnPin != 0) pinMode(_btnPin, INPUT_PULLUP);

  if (_debug) {
    Serial.println("[EZConnect] -> Initialized");
    Serial.print("[EZConnect] -> Mode: ");
    Serial.println(_mode == EZCONNECT_WIFI_PORTAL ? "Wi-Fi Portal" :
                   (_mode == EZCONNECT_BLE ? "BLE" : "Classic Bluetooth"));
  }

  if (_mode == EZCONNECT_WIFI_PORTAL) {
    trySavedWiFiOrStartPortal();
    return;
  }

  if (_mode == EZCONNECT_BLE) BLEDevice::init(_deviceName.c_str());
}

String EZConnect::getSSID() {
  _prefs.begin("wifi", true);
  String value = _prefs.getString("ssid", "");
  _prefs.end();
  return value;
}

String EZConnect::getPass() {
  _prefs.begin("wifi", true);
  String value = _prefs.getString("pass", "");
  _prefs.end();
  return value;
}

void EZConnect::checkConnectivity(uint32_t wifiTimeout, uint32_t blinkOn, uint32_t blinkOff) {
  _blinkOn = blinkOn;
  _blinkOff = blinkOff;
  String ssid = getSSID();

  if (_debug && ssid.length()) {
    Serial.print("[EZConnect] -> Connecting to ");
    Serial.println(ssid);
  }

  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    blink(blinkOn, blinkOff);
    if (millis() - start > wifiTimeout) {
      _configMode = true;
      if (_debug) Serial.println("[EZConnect] -> Wi-Fi connection timed out; configuration mode enabled.");
      break;
    }
    delay(1);
  }

  if (WiFi.status() == WL_CONNECTED) {
    if (_ledPin != 0) digitalWrite(_ledPin, LOW);
    if (_debug) {
      Serial.println("[EZConnect] -> Wi-Fi connected");
      Serial.print("[EZConnect] -> IP address: ");
      Serial.println(WiFi.localIP());
    }
  }
}

void EZConnect::check() {
  checkButton();

  if (_portalActive) {
    _dnsServer.processNextRequest();
    _webServer.handleClient();
    return;
  }

  if (_mode == EZCONNECT_WIFI_PORTAL) return;
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

void EZConnect::checkButton() {
  if (_btnPin == 0) return;
  static uint32_t pressedAt = 0;
  static bool pressed = false;

  if (digitalRead(_btnPin) == LOW) {
    if (!pressed) {
      pressedAt = millis();
      pressed = true;
      _buttonTriggered = false;
    }
    if (! _buttonTriggered && millis() - pressedAt >= 3000) {
      _buttonTriggered = true;
      if (_mode == EZCONNECT_WIFI_PORTAL) {
        if (_debug) Serial.println("[EZConnect] -> Opening Wi-Fi provisioning portal");
        startPortalAP();
      } else {
        _configMode = true;
        if (_debug) Serial.println(_mode == EZCONNECT_BLE ? "[EZConnect] -> BLE enabled" : "[EZConnect] -> Classic Bluetooth enabled");
        if (_mode == EZCONNECT_BLE) startBLE();
        else startClassicBT();
      }
    }
  } else {
    pressed = false;
    _buttonTriggered = false;
  }
}

void EZConnect::startClassicBT() {
#if EZCONNECT_CLASSIC_BT_SUPPORTED
  _bt.begin(_deviceName.c_str());
#endif
}

void EZConnect::startBLE() {
  BLEServer *server = BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());
  BLEService *service = server->createService(_bleServiceUUID);
  BLECharacteristic *rx = service->createCharacteristic(
    _bleRxUUID, BLECharacteristic::PROPERTY_WRITE);
  rx->setCallbacks(new RXCallbacks());
  service->start();
  BLEAdvertising *advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(_bleServiceUUID);
  advertising->start();
}

void EZConnect::_onBLEData(const String &data) {
  _rxData = data;
  _newData = true;
}

void EZConnect::handleConfigInput(String input) {
  input.trim();
  if (_debug) Serial.println("[EZConnect] -> Received Bluetooth configuration data");
  String ssid, pass;
  if (!parseCredentials(input, ssid, pass)) {
    if (_debug) Serial.println("[EZConnect] -> Invalid format. Send SSID,password");
    return;
  }
  _prefs.begin("wifi", false);
  _prefs.putString("ssid", ssid);
  _prefs.putString("pass", pass);
  _prefs.end();
  if (_debug) Serial.println("[EZConnect] -> Credentials saved; restarting...");
  delay(500);
  ESP.restart();
}

bool EZConnect::parseCredentials(String input, String &ssid, String &pass) {
  int separator = input.indexOf(',');
  if (separator < 0) return false;
  ssid = input.substring(0, separator);
  pass = input.substring(separator + 1);
  ssid.trim();
  return ssid.length() > 0;
}

void EZConnect::trySavedWiFiOrStartPortal() {
  String ssid = getSSID();
  String pass = getPass();
  if (ssid.length() > 0) {
    if (_debug) {
      Serial.print("[EZConnect] -> Trying saved network: ");
      Serial.println(ssid);
    }
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());
    uint32_t started = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - started < 10000) {
      blink(150, 150);
      delay(20);
    }
    if (WiFi.status() == WL_CONNECTED) {
      if (_ledPin != 0) digitalWrite(_ledPin, LOW);
      if (_debug) {
        Serial.print("[EZConnect] -> Connected. IP: ");
        Serial.println(WiFi.localIP());
      }
      return;
    }
    if (_debug) Serial.println("[EZConnect] -> Saved network unavailable; starting setup portal.");
  }
  startPortalAP();
}

bool EZConnect::startWiFiPortal() {
  if (_mode != EZCONNECT_WIFI_PORTAL) {
    if (_debug) Serial.println("[EZConnect] -> startWiFiPortal() requires EZCONNECT_WIFI_PORTAL mode.");
    return false;
  }
  return startPortalAP();
}

bool EZConnect::startPortalAP() {
  if (_portalActive) return true;
  if (_portalSSID.length() == 0 || _portalSSID.length() > 32 || _portalPassword.length() < 8) {
    if (_debug) Serial.println("[EZConnect] -> Portal requires an SSID (1-32 chars) and hotspot password (at least 8 chars).");
    return false;
  }

  WiFi.mode(WIFI_AP_STA);
  if (!WiFi.softAP(_portalSSID.c_str(), _portalPassword.c_str())) {
    if (_debug) Serial.println("[EZConnect] -> Failed to start setup hotspot.");
    return false;
  }

  configureWebRoutes();
  _webServer.begin();
  IPAddress apIP = WiFi.softAPIP();
  _dnsServer.start(53, "*", apIP);
  _portalActive = true;
  _configMode = true;

  if (_debug) {
    Serial.print("[EZConnect] -> Setup hotspot: "); Serial.println(_portalSSID);
    Serial.print("[EZConnect] -> Open http://"); Serial.println(apIP);
  }
  return true;
}

void EZConnect::stopWiFiPortal() {
  if (!_portalActive) return;
  _dnsServer.stop();
  _webServer.stop();
  WiFi.softAPdisconnect(true);
  _portalActive = false;
  _configMode = false;
  WiFi.mode(WiFi.status() == WL_CONNECTED ? WIFI_STA : WIFI_OFF);
}

void EZConnect::configureWebRoutes() {
  _webServer.on("/", HTTP_GET, [this]() { handlePortalRoot(); });
  _webServer.on("/connect", HTTP_POST, [this]() { handlePortalConnect(); });
  _webServer.on("/generate_204", HTTP_GET, [this]() { handlePortalRoot(); });
  _webServer.on("/hotspot-detect.html", HTTP_GET, [this]() { handlePortalRoot(); });
  _webServer.on("/connecttest.txt", HTTP_GET, [this]() { handlePortalRoot(); });
  _webServer.on("/ncsi.txt", HTTP_GET, [this]() { handlePortalRoot(); });
  _webServer.onNotFound([this]() {
    _webServer.sendHeader("Location", String("http://") + WiFi.softAPIP().toString() + "/", true);
    _webServer.send(302, "text/plain", "");
  });
}

void EZConnect::handlePortalRoot() {
  String page;
  page.reserve(9500);
  page += F("<!doctype html><html lang='en'><head><meta charset='utf-8'>"
            "<meta name='viewport' content='width=device-width,initial-scale=1'>"
            "<meta name='theme-color' content='#111827'><title>");
  page += htmlEscape(_portalTitle);
  page += F("</title><style>"
    ":root{color-scheme:dark;--bg:#0b1020;--card:#141c30;--muted:#9aa8c0;--line:#283550;--accent:#7c8cff;--accent2:#36d6c5}"
    "*{box-sizing:border-box}body{margin:0;min-height:100vh;font-family:Inter,ui-sans-serif,system-ui,-apple-system,Segoe UI,sans-serif;"
    "background:radial-gradient(ellipse at 15% 0%,#25346a 0,transparent 42%),radial-gradient(ellipse at 100% 10%,#134e5e 0,transparent 32%),var(--bg);color:#f4f7ff}"
    ".wrap{width:min(100% - 28px,620px);margin:0 auto;padding:36px 0 22px}.brand{display:flex;align-items:center;gap:12px;margin:0 0 24px}"
    ".logo{width:46px;height:46px;border-radius:15px;display:grid;place-items:center;background:linear-gradient(135deg,var(--accent),var(--accent2));color:#07101d;font-weight:900;font-size:22px;box-shadow:0 8px 28px #5268ff35}"
    "h1{font-size:clamp(23px,5vw,32px);letter-spacing:-.7px;margin:0;font-weight:750}.sub{color:var(--muted);margin:5px 0 0;font-size:14px}"
    ".card{background:linear-gradient(160deg,#172139ed,#111a2ded);border:1px solid #ffffff14;border-radius:24px;padding:22px;box-shadow:0 24px 70px #0004}"
    ".section-head{display:flex;align-items:center;justify-content:space-between;gap:12px;margin-bottom:12px}h2{font-size:16px;margin:0}.count{font-size:12px;color:var(--muted)}"
    ".networks{display:grid;gap:9px;margin:12px 0 20px}.network{display:flex;align-items:center;gap:12px;padding:13px;border:1px solid var(--line);border-radius:15px;background:#0e1729;cursor:pointer;transition:.15s}"
    ".network:has(input:checked){border-color:var(--accent);background:#202a4b;box-shadow:0 0 0 1px #7c8cff40}.network input{accent-color:var(--accent);width:17px;height:17px;flex:0 0 auto}"
    ".neticon{width:38px;height:38px;display:grid;place-items:center;border-radius:12px;background:#202b43;font-size:18px;flex:0 0 auto}.netinfo{min-width:0;flex:1}.ssid{font-size:14px;font-weight:650;overflow-wrap:anywhere}.meta{font-size:12px;color:var(--muted);margin-top:4px}.signal{font-size:11px;color:var(--muted);text-align:right;white-space:nowrap}"
    ".empty{border:1px dashed #394763;border-radius:15px;padding:20px;text-align:center;color:var(--muted);font-size:13px}.refresh{color:#c4cbff;text-decoration:none;font-size:12px;font-weight:650}"
    "label.field{display:block;font-size:13px;color:#d9e2f4;margin:15px 0 7px}.password-wrap{display:flex;align-items:center;background:#0b1425;border:1px solid var(--line);border-radius:13px;overflow:hidden}.password-wrap:focus-within{border-color:var(--accent);box-shadow:0 0 0 3px #7c8cff25}.password-wrap input{width:100%;min-width:0;background:transparent;border:0;padding:14px;color:#fff;font-size:15px;outline:none}.password-toggle{border:0;background:transparent;color:#c4cbff;padding:12px 14px;font-size:12px;font-weight:700;cursor:pointer;white-space:nowrap}.password-toggle:focus-visible{outline:2px solid var(--accent);outline-offset:-4px}"
    ".hint{font-size:12px;color:var(--muted);line-height:1.5;margin:9px 0 0}.upload{width:100%;border:0;border-radius:14px;padding:15px;margin-top:20px;background:linear-gradient(100deg,var(--accent),var(--accent2));color:#07101d;font-size:15px;font-weight:800;cursor:pointer;box-shadow:0 8px 24px #5268ff26;display:flex;align-items:center;justify-content:center;gap:10px;transition:opacity .2s,transform .2s}.upload:active{transform:scale(.99)}.upload:disabled{opacity:.78;cursor:wait;transform:none}.spinner{width:17px;height:17px;border:2px solid #07101d45;border-top-color:#07101d;border-radius:50%;display:inline-block;animation:spin .7s linear infinite;vertical-align:middle}@keyframes spin{to{transform:rotate(360deg)}}"
    "footer{text-align:center;color:#74819b;font-size:12px;padding:22px 0 0}footer strong{color:#aab6d2;font-weight:700}.lock{display:inline-block;margin-right:5px}"
    "@media(max-width:440px){.wrap{padding-top:24px}.card{padding:16px;border-radius:20px}.network{padding:11px}.signal{display:none}}"
    "</style></head><body><main class='wrap'><header class='brand'><div class='logo'>E</div><div><h1>");
  page += htmlEscape(_portalTitle);
  page += F("</h1><p class='sub'>Secure Wi-Fi setup · powered device onboarding</p></div></header>"
    "<section class='card'><div class='section-head'><h2>Choose your Wi-Fi network</h2><a class='refresh' href='/'>↻ Refresh scan</a></div>"
    "<div class='count'>Nearby networks detected by your device</div><form method='POST' action='/connect' id='wifiForm'><div class='networks'>");

  int count = WiFi.scanNetworks();
  if (count <= 0) {
    page += F("<div class='empty'>No Wi-Fi networks found.<br>Move closer to your router and refresh the scan.</div>");
  } else {
    for (int i = 0; i < count; ++i) {
      String ssid = WiFi.SSID(i);
      if (ssid.length() == 0) continue;
      String safeSSID = htmlEscape(ssid);
      int32_t rssi = WiFi.RSSI(i);
      bool secured = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;
      page += F("<label class='network'><input type='radio' name='ssid' value='");
      page += safeSSID;
      page += F("' required><span class='neticon'>");
      page += secured ? "🔒" : "◉";
      page += F("</span><span class='netinfo'><span class='ssid'>");
      page += safeSSID;
      page += F("</span><span class='meta'>");
      page += secured ? "Password protected" : "Open network";
      page += F(" · ");
      page += htmlEscape(signalLabel(rssi));
      page += F("</span></span><span class='signal'>");
      page += String(rssi);
      page += F(" dBm</span></label>");
    }
  }
  WiFi.scanDelete();
  page += F("</div><label class='field' for='pass'>Wi-Fi password</label>"
    "<div class='password-wrap'><input id='pass' name='pass' type='password' maxlength='64' autocomplete='current-password' placeholder='Enter the selected network password'>"
    "<button class='password-toggle' id='passwordToggle' type='button' aria-controls='pass' aria-pressed='false'>Show</button></div>"
    "<p class='hint'><span class='lock'>▣</span>Your Wi-Fi credentials are saved on this device and are not sent to an external server.</p>"
    "<button class='upload' id='uploadBtn' type='submit'><span>Upload credentials</span><span aria-hidden='true'>→</span></button></form></section>"
    "<footer>Powered by <strong>EZConnect</strong></footer></main>"
    "<script>var pass=document.getElementById('pass');var toggle=document.getElementById('passwordToggle');toggle.addEventListener('click',function(){var visible=pass.type==='text';pass.type=visible?'password':'text';toggle.textContent=visible?'Show':'Hide';toggle.setAttribute('aria-pressed',String(!visible));pass.focus();});"
    "document.querySelectorAll('input[name=ssid]').forEach(function(r){r.addEventListener('change',function(){pass.focus();});});"
    "document.getElementById('wifiForm').addEventListener('submit',function(e){var n=document.querySelector('input[name=ssid]:checked');if(!n){e.preventDefault();alert('Please select a Wi-Fi network.');return;}var b=document.getElementById('uploadBtn');if(b.disabled){e.preventDefault();return;}b.disabled=true;b.innerHTML='<span class=\"spinner\" aria-hidden=\"true\"></span><span>Uploading credentials…</span>';});</script>"
    "</body></html>");

  _webServer.send(200, "text/html; charset=utf-8", page);
}

void EZConnect::handlePortalConnect() {
  String ssid = _webServer.arg("ssid");
  String pass = _webServer.arg("pass");
  ssid.trim();
  if (ssid.length() == 0 || ssid.length() > 32 || pass.length() > 64) {
    _webServer.send(400, "text/plain; charset=utf-8", "Invalid Wi-Fi name or password length. Return to the setup page and try again.");
    return;
  }

  _prefs.begin("wifi", false);
  _prefs.putString("ssid", ssid);
  _prefs.putString("pass", pass);
  _prefs.end();

  String page = F("<!doctype html><html lang='en'><head><meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<meta charset='utf-8'><meta name='theme-color' content='#0b1020'><title>Connecting · EZConnect</title><style>"
    "*{box-sizing:border-box}body{margin:0;min-height:100vh;display:grid;place-items:center;font-family:system-ui,-apple-system,Segoe UI,sans-serif;"
    "background:radial-gradient(ellipse at 20% 0%,#25346a 0,transparent 45%),#0b1020;color:#f4f7ff;padding:20px}"
    ".card{width:100%;max-width:440px;text-align:center;background:#141c30;border:1px solid #ffffff20;border-radius:24px;padding:32px 24px;line-height:1.6;box-shadow:0 24px 70px #0005}"
    ".spinner{width:48px;height:48px;border:4px solid #283550;border-top-color:#36d6c5;border-radius:50%;margin:0 auto 22px;animation:spin .8s linear infinite}"
    "@keyframes spin{to{transform:rotate(360deg)}}h2{margin:0 0 10px;font-size:23px}p{color:#aab6cc;font-size:14px}.steps{margin:22px 0 0;padding:13px;border-radius:13px;background:#0b1425;color:#b8c6df;font-size:13px}footer{margin-top:22px;color:#74819b;font-size:12px}</style></head><body><main class='card'>"
    "<div class='spinner' role='status' aria-label='Loading'></div><h2>Credentials received</h2>"
    "<p>Your Wi-Fi details have been saved successfully. EZConnect is restarting and attempting to connect to your selected network.</p>"
    "<div class='steps'>Please wait a moment. The setup hotspot may disappear when the device restarts. You can reconnect your phone to its usual Wi-Fi network.</div>"
    "<footer>Powered by <strong>EZConnect</strong></footer></main></body></html>");
  _webServer.send(200, "text/html; charset=utf-8", page);
  if (_debug) Serial.println("[EZConnect] -> Portal credentials saved; restarting to connect to the selected network.");
  delay(800);
  ESP.restart();
}

void EZConnect::blink(uint32_t onTime, uint32_t offTime) {
  if (_ledPin == 0) return;
  static uint32_t lastChange = 0;
  static bool state = false;
  uint32_t interval = state ? onTime : offTime;
  if (millis() - lastChange >= interval) {
    lastChange = millis();
    state = !state;
    digitalWrite(_ledPin, state ? HIGH : LOW);
  }
}
