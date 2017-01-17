
#include <HeatPump.h>

void setup() {
  // put your setup code here, to run once:
 HeatPump hp;
 hp.connect(&Serial);     //For ESP8266
 //hp.connect(&Serial1);  //Use UART1 or Arduino Micro Pro
 hp.update();  //   power mode  temp fan vane dir 
 String settings[6]={"ON","FAN","26","4","3","|"};
 hp.setSettings(settings);
 hp.update();
}

void loop() {
  // put your main code here, to run repeatedly:
  hp.sync();
}
