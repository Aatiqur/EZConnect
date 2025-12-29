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
