
#include <HeatPump.h>

void setup() {
  // put your setup code here, to run once:
 HeatPump hp;
 hp.connect(&Serial);
 hp.update();  //   power mode  temp fan vane dir 
 String settings[6]={"ON","FAN","26","4","3","|"};
 hp.setSettings(settings);
 hp.update();
}

void loop() {
  // put your main code here, to run repeatedly:

}
