[![Join the chat at https://gitter.im/Mitsubishi-Heat-Pump](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/Mitsubishi-Heat-Pump?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

HeatPump
----------
Arduino library to control Mitsubishi Heat Pumps via connector cn105

Quick start
-----------

#### Controlling the heat pump

```c++
HeatPump hp;
hp.connect(&Serial);

heatpumpSettings settings = {
    "ON",  /* ON/OFF */
    "FAN", /* HEAT/COOL/FAN/DRY/AUTO */
    26,    /* Between 16 and 31 */
    "4",   /* Fan speed: 1-4, AUTO, or QUIET */
    "3",   /* Air direction (vertical): 1-5, SWING, or AUTO */
    "|"    /* Air direction (horizontal): <<, <, |, >, >>, <>, or SWING */
}; 

hp.setSettings(settings);
// OR individual settings
// hp.setModeSetting("COOL");

hp.update();
```

[See heatPump_test.ino](examples/heatPump_test/heatPump_test.ino)

#### Getting updates from the heat pump

```c++
void setup() {
  HeatPump hp;
  hp.connect(&Serial);
}

void loop() {
  hp.sync();

  /* get settings from heatpump, including room temperature in settings.roomTemperature */
  heatpumpSettings settings = hp.getSettings();
}

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
- Tested with Arduino Micro Pro / Arduino Nano
- Tested with Mitsubishi HeatPump MSZ-FH/GE(wall units) and SEZ-KD (ducted units) [complete list](https://github.com/SwiCago/HeatPump/wiki/Supported-models)

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
- Premade pigtails
  - http://www.usastore.revolectrix.com/Products_2/Cellpro-4s-Charge-Adapters_2/Cellpro-JST-PA-Battery-Pigtail-10-5-Position
- ESP-01 module (4pk)
  - https://www.amazon.com/gp/product/B01EA3UJJ4/
- Cheap 5V to 3.3V regulator (10pk), for those that don't want to make one
  - https://www.amazon.com/gp/product/B00XAGSHY2/
  
========================================  
Special thanks to Hadley in New Zealand 
  - His blog post, describing baud rate and details of cn105, Raspberry Pi Python code
  - https://nicegear.co.nz/blog/hacking-a-mitsubishi-heat-pump-air-conditioner/

# GNU Lesser General Public License
https://www.gnu.org/licenses/lgpl.html
