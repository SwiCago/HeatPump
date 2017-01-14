HeatPump
----------
Arduino library to control Mitsubishi Heat Pumps via connector cn105

Quick start
-----------

#### Controlling the heat pump

```c++
HeatPump hp;
hp.connect(&Serial);
String settings[6]={
    "ON",  /* ON/OFF */
	"FAN", /* HEAT/COOL/FAN/DRY/AUTO */
	"26",  /* Between 16 and 31 */
	"4",   /* Fan speed 1-4, AUTO, or SILENT */
	"3",   /* Air direction 1-5, SWING, or AUTO */
	"|"    /* Direction <<, <, |, >, >> */
}; //
hp.setSettings(settings);
hp.update();
```

[See heatPump_test.ino](examples/heatPump_test/heatPump_test.ino)

#### Getting updates from the heat pump

```c++
String settings[7]={}
HeatPump hp;
hp.connect(&Serial);

hp.requestSettings();
delay(1000);
hp.requestTemperature();
delay(1000);
hp.checkForUpdates();

hp.getSettings(settings);
/* settings now contains updated settings from heatpump, and room temperature in settings[6] */

/* you can also put this in your loop() function to automatically keep the settings/temperature updated: */

hp.checkForUpdates();
hp.requestInfoAlternate();

```

Contents
--------
- sources
- sample usage code
- Demo circuit using ESP-01

Installation
------------
- PULL or download zip.
- Move contents into Arduino library directory
- Restart IDE, samples should be avaliable

Notes
-----
- Tested with ESP8266
- Tested with Arduino Micro Pro
- Tested with Mitsubishi HeatPump MSZ-FH/GE(wall units) and SEZ-KD (ducted units)

Demo Circuit
------------
<img src="https://github.com/SwiCago/HeatPump/blob/master/CN105_ESP8266.png"/>

# Parts required to make a CN105 female connector
- PAP-05V-S CONN HOUSING PA 5POS 2MM WHITE 
  - Digi-Key Part Number 	455-1489-ND 
  - https://www.digikey.com/product-detail/en/jst-sales-america-inc/PAP-05V-S/455-1489-ND/759977
- SPHD-002T-P0.5  CONN TERM PHD CRIMP 24-28AWG TIN  
  - Digi-Key Part Number 	455-1313-1-ND 
  - https://www.digikey.com/product-detail/en/jst-sales-america-inc/SPHD-002T-P0.5/455-1313-1-ND/608809
- ESP-01 module (4pk)
  - https://www.amazon.com/gp/product/B01EA3UJJ4/
- Cheap 5V to 3.3V regulator (10pk), for those that don't want to make one
  - https://www.amazon.com/gp/product/B00XAGSHY2/
  
========================================  
Special thanks to Hadley in New Zealand 
  - His blog post, describing baud rate and details of cn105, Raspberry Pi Python code
  - https://nicegear.co.nz/blog/hacking-a-mitsubishi-heat-pump-air-conditioner/
