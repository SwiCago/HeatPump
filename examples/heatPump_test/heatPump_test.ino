
#include <HeatPump.h>
 HeatPump hp;
void setup() {
  // put your setup code here, to run once:

  hp.connect(&Serial);    //For ESP8266
//hp.connect(&Serial1);   //Use UART1 or Arduino Micro Pro
  heatpumpSettings mySettings = hp.getSettings();
  mySettings.power = "ON";
  mySettings.mode = "FAN";
  mySettings.temperature = 26;
  mySettings.fan = "4";
  mySettings.vane = "3";
  mySettings.wideVane = "|";
  hp.setSettings(mySettings);
  hp.update();
}

void loop() {
  // put your main code here, to run repeatedly:
  hp.sync();
}