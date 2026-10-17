#include <EZConnect.h>
#include <WiFi.h>

// BLE provisioning example using custom BLE UUIDs.
EZConnect ez(EZCONNECT_BLE, "EZConnect-Device", 2, 0);

void configureUUIDs() {
  // Use these UUIDs in your matching BLE client/application.
  // This must be called BEFORE ez.begin() so EZConnect starts with these UUIDs.
  ez.setUUIDs("12345678-1234-1234-1234-1234567890AB",
              "87654321-4321-4321-4321-BA0987654321");
}

void setup() {
  Serial.begin(115200);

  configureUUIDs();
  ez.showDebugmsg();  // Enable diagnostic output in Serial Monitor.
  ez.begin();         // Start BLE provisioning.

  // Retrieve the Wi-Fi credentials saved by EZConnect.
  String ssid = ez.getSSID();  // Saved network name.
  String pass = ez.getPass();  // Saved network password.

  // Connect using the retrieved credentials.
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
  ez.check();  // Keep EZConnect checks active.
}
