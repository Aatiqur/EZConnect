#include <EZConnect.h>

// Browser-based Wi-Fi provisioning example.
// Connect a phone or computer to the EZConnect-Setup hotspot, then open the
// setup page to select a Wi-Fi network and enter its password.
// EZConnect.h includes WiFi.h, so a separate #include <WiFi.h> is not needed.
//
// Constructor arguments:
//   1. Mode: EZCONNECT_WIFI_PORTAL enables browser-based provisioning.
//   2. Hotspot SSID: the Wi-Fi name broadcast by the ESP32 for setup.
//   3. Hotspot password: use a unique password with at least 8 characters.
//   4. Status LED pin: set to 0 if no status LED is connected.
//   5. Setup button pin: set to 0 if no setup button is connected.
//   6. Portal title: optional heading displayed on the setup page.
EZConnect ez(EZCONNECT_WIFI_PORTAL,
             "EZConnect",
             "12345678",  // Replace with a unique hotspot password (8+ characters).
             2,                 // Status LED pin; use 0 if no LED is connected.
             0,                 // Setup button pin; use 0 if no button is connected.
             "TITLE"); // Optional title; defaults to "EZConnect" if omitted.

void setup() {
  Serial.begin(115200);


  ez.begin();

  // Enable EZConnect diagnostic messages before begin(), so startup messages
  // are printed too. Messages use the format: [EZConnect] -> message
  ez.showDebugmsg();


  // Read the Wi-Fi credentials stored by EZConnect.
  String ssid = ez.getSSID();  // Saved Wi-Fi network name (SSID).
  String pass = ez.getPass();  // Saved Wi-Fi network password.

  // Start a Wi-Fi station connection using the retrieved credentials.
  // WiFi.begin() accepts C-style strings, so pass ssid.c_str() and pass.c_str().
  WiFi.begin(ssid.c_str(), pass.c_str());

  // Check Wi-Fi connectivity.
  // Parameters:
  //   wifiTimeout - maximum time to wait for connection, in milliseconds (10000 = 10 s).
  //   blinkOn     - status LED ON interval while waiting, in milliseconds (500 = 0.5 s).
  //   blinkOff    - status LED OFF interval while waiting, in milliseconds (500 = 0.5 s).
  // This call waits up to 10 seconds; afterward loop() keeps the portal responsive.
  ez.checkConnectivity(10000, 500, 500);
}

void loop() {
  // Required for portal mode: process DNS requests, serve the setup page,
  // and handle submitted Wi-Fi credentials.
  ez.check();
}
