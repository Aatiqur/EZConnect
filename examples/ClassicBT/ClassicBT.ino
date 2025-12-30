/*
🔴 CRITICAL STEP FOR GENERIC ESP32 DEV BOARDS - DO NOT SKIP! 🔴

After installing the library, you MUST change the partition scheme to avoid compilation or runtime errors.

Required Steps:
In Arduino IDE, go to Tools → Partition Scheme
Change from: Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)
Change to: Huge APP (3MB No OTA/1MB SPIFFS)
Why is this necessary?
This library uses both WiFi and Bluetooth simultaneously, which requires more program space than the default partition scheme allows. Without this change, your sketch may fail to compile or upload.


WiFi Credentials Format:
Send WiFi credentials using one of these formats:

   Format              |  Example
   --------------------|------------------------
   Newline separator   |  MyWiFi\n12345678
   Pipe separator      |  MyWiFi|12345678
   Tab separator       |  MyWiFi	12345678
   Comma separator     |  MyWiFi,12345678

Replace 'MyWiFi' with your SSID and '12345678' with your password.

For more information read the documentation in the gihub repo: https://github.com/Aatiqur/EZConnect/

*/



// Include the EZConnect library
#include <EZConnect.h>

// Initialize EZConnect with Classic Bluetooth mode
// Parameters: mode, device name, LED pin for WiFi status, LED pin for BT status
EZConnect ez(EZCONNECT_CLASSIC_BT,"EZConnect-Device",2,0);

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(115200);
  
  // Start the EZConnect service (begins Classic Bluetooth)
  ez.begin();
  
  // Enable debug messages to serial monitor
  ez.showDebugmsg();

  // Retrieve WiFi credentials received via Classic Bluetooth
  String ssid = ez.getSSID();
  String pass = ez.getPass();

  // Attempt to connect to WiFi using the received credentials
  WiFi.begin(ssid.c_str(), pass.c_str());

  // Check connectivity status
  // Parameters: timeout (ms), blink interval when connecting (ms), blink interval when connected (ms)
  ez.checkConnectivity(10000, 500, 500);
}

void loop() {
  // Continuously check for incoming Bluetooth data and maintain connection
  ez.check();
}

