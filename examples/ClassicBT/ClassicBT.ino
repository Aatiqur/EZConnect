#include <EZConnect.h>
#include <WiFi.h>

// Classic Bluetooth provisioning example.
// IMPORTANT: Classic Bluetooth is supported only by the original ESP32,
// not ESP32-S2, ESP32-S3, ESP32-C3, or ESP32-C6.
// Constructor arguments: mode, Bluetooth device name, status LED pin, setup button pin.
EZConnect ez(EZCONNECT_CLASSIC_BT, "EZConnect-Device", 2, 0);

void setup() {
  Serial.begin(115200);

  // Print EZConnect status and connection diagnostics to the Serial Monitor.
  ez.showDebugmsg();

  // Start Classic Bluetooth provisioning.
  ez.begin();

  // Get the credentials that EZConnect saved during provisioning.
  String ssid = ez.getSSID();  // Wi-Fi network name (SSID).
  String pass = ez.getPass();  // Wi-Fi network password.

  // Start connecting the ESP32 to the saved Wi-Fi network.
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
  // Continue checking the EZConnect setup button and internal state.
  ez.check();
}
