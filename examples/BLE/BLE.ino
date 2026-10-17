#include <EZConnect.h>
#include <WiFi.h>

// BLE provisioning example.
// EZConnect receives Wi-Fi credentials over Bluetooth Low Energy (BLE).
// Constructor arguments: mode, Bluetooth device name, status LED pin, setup button pin.
EZConnect ez(EZCONNECT_BLE, "EZConnect-Device", 2, 0);

void setup() {
  Serial.begin(115200);

  // Enable EZConnect diagnostic messages in the Serial Monitor.
  ez.showDebugmsg();

  // Start BLE provisioning. Configure Wi-Fi credentials with the EZConnect client/app.
  ez.begin();

  // Read the Wi-Fi credentials stored by EZConnect.
  String ssid = ez.getSSID();  // Saved Wi-Fi network name.
  String pass = ez.getPass();  // Saved Wi-Fi password.

  // Arduino WiFi.begin() expects C-style strings, so use c_str().
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
  // Keep EZConnect's button/configuration checks running.
  ez.check();
}
