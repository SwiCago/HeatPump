
#include <HeatPump.h>
 HeatPump hp;
void setup() {
  // put your setup code here, to run once:

  hp.connect(&Serial);    //For ESP8266
//hp.connect(&Serial1);   //Use UART1 or Arduino Micro Pro
  heatpumpSettings mySettings = {
    "ON",  /* ON/OFF */
    "FAN", /* HEAT/COOL/FAN/DRY/AUTO */
    26,    /* Between 16 and 31 */
    "4",   /* Fan speed: 1-4, AUTO, or QUIET */
    "3",   /* Air direction (vertical): 1-5, SWING, or AUTO */
    "|"    /* Air direction (horizontal): <<, <, |, >, >>, <>, or SWING */
  };
  hp.setSettings(mySettings);
  hp.update();
}

void loop() {
  // put your main code here, to run repeatedly:
  hp.sync();
}