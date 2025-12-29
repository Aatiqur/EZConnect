/*
🔴 CRITICAL STEP - DO NOT SKIP! 🔴

After installing the library, you MUST change the partition scheme to avoid compilation or runtime errors.

Required Steps:
In Arduino IDE, go to Tools → Partition Scheme
Change from: Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)
Change to: Huge APP (3MB No OTA/1MB SPIFFS)
Why is this necessary?
This library uses both WiFi and Bluetooth simultaneously, which requires more program space than the default partition scheme allows. Without this change, your sketch may fail to compile or upload.


For more information read the documentation in the gihub repo: https://github.com/Aatiqur/EZConnect/

*/


#include <EZConnect.h>

EZConnect ez(2, 0, "EZConnect_BT"); //LED,debugBtn,BTName

void setup() {
  Serial.begin(115200);

  ez.begin();
  String ssid = ez.getSSID();
  String pass = ez.getPass();

  WiFi.begin(ssid.c_str(), pass.c_str());
  Serial.println("Connecting to " + ssid);

  ez.checkConnectivity(10000,500,500);//wifiTime,onTime,offTime
}

void loop() {
  ez.check(); // check whether debugbtn pressed or not
}
