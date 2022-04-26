Support Chat here -> [![Join the chat at https://gitter.im/Mitsubishi-Heat-Pump](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/Mitsubishi-Heat-Pump?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)  <-Support Chat here

# HeatPump

Arduino library to control Mitsubishi Heat Pumps via connector CN105.

## Quick start

### Controlling the heat pump

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

You can make the library automatically send new settings to the heat pump by calling `enableAutoUpdate()`. When auto update is enabled the call to `update()` in the above example is not necessary, the new settings will be sent to the heat pump on the next call to `sync()` in `loop()`.

### Getting updates from the heat pump

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

By default the library ignores changes made from other sources (usually, the IR remote) and reverts them the next time `sync()` is called. This is the intendend behavior when the heat pump is fully controlled by automation.

If you want to also allow manual control and allow the library to update its settings from the current state of the heat pump you need to call `enableExternalUpdate()`. This will also enable automatic updates.

### Support for installer settings/functions
Important: This is only tested on PVA (P-Series air handler) units and is not known to work on any other models. 

You can refer to page 6 of this document to see the generic list of functions: https://www.mitsubishitechinfo.ca/sites/default/files/Installation_Manual_69-2426-01_0.pdf. Note that what each setting does is model specific. For example, this document lists the available codes and values for PVAs: https://www.mitsubishitechinfo.ca/sites/default/files/IM_PVA_A12_42AA7_PA79D213H09.pdf, page 22.

```c++
heatpumpFunctions functions = hp.getFunctions();

heatpumpFunctionCodes codes = functions.getAllCodes();
for (int i = 0; i < MAX_FUNCTION_CODE_COUNT; ++i) {
  if (codes.valid[i]) {
    int code = codes.code[i];
    int value = functions.getValue(code);
    // handle value
  }
}


if (!functions.setValue(code, value)) {
  // handle error
}

if (!hp.setFunctions(functions)) {
  // handle error
}
```

It is recommended to call `getFunctions()` every time when you need to make a change to the values in order to get a fresh `heatpumpFunctions`. Otherwise you might accidentally write out stale values and overwrite changes that might have happened through other sources.

### Callbacks

Instead of manually checking settings changes on each loop, you can set callback functions to be called when the current heat pump status or settings change:

```
void hpSettingsChanged() {
  // ...
}

void hpStatusChanged(heatpumpStatus currentStatus) {
  // ...
}

void setup() {
  hp.setSettingsChangedCallback(hpSettingsChanged);
  hp.setStatusChangedCallback(hpStatusChanged);

  hp.connect(&Serial);
}
```

The callbacks will be called as necessary by the `sync()` method.

You can see this in use in the [MQTT example](examples/mitsubishi_heatpump_mqtt_esp8266_esp32/mitsubishi_heatpump_mqtt_esp8266_esp32.ino).

## Contents

- sources
- sample usage code
- Demo circuit using ESP-01

## Installation

- PULL or download zip.
- Move contents into Arduino library directory
- Restart IDE, samples should be avaliable
- NOTE: Requires arduino json 6, older commits before 20190505 support json 5.
- If you find this all a little confusing, check out this write up blog/install by Chris Davis
    - https://chrdavis.github.io/hacking-a-mitsubishi-heat-pump-Part-1/
    - https://chrdavis.github.io/hacking-a-mitsubishi-heat-pump-Part-2/
        - Note: some people report problems with Wemos D1 due to usb serial pulling RX high when usb idle! Fix is to cut a trace, so maybe use an alternative, if following the above blog links.
## Notes

- Tested with ESP8266
- Tested with Arduino Micro Pro / Arduino Nano
- Tested with Mitsubishi HeatPump MSZ-FH/GE(wall units) and SEZ-KD (ducted units) [complete list](https://github.com/SwiCago/HeatPump/wiki/Supported-models)

## Demo Circuit

<img src="https://github.com/SwiCago/HeatPump/blob/master/CN105_ESP8266.png"/>

## Parts

### Parts required to make a CN105 female connector

- PAP-05V-S CONN HOUSING PA 5POS 2MM WHITE 
    - Digi-Key Part Number 	455-1489-ND 
    - <https://www.digikey.com/product-detail/en/jst-sales-america-inc/PAP-05V-S/455-1489-ND/759977>
- SPHD-002T-P0.5  CONN TERM PHD CRIMP 24-28AWG TIN  
    - Digi-Key Part Number 	455-1313-1-ND
    - <https://www.digikey.com/product-detail/en/jst-sales-america-inc/SPHD-002T-P0.5/455-1313-1-ND/608809>
- JUMPER SPHD-001T-P0.5 X2 12" (pre-crimped alternative to 455-1313-1-ND connectors)
    - Digi-Key Part Number    455-3086-ND
    - <https://www.digikey.co.uk/product-detail/en/jst-sales-america-inc/APAPA22K305/455-3086-ND/6009462>

### Other part suggestions

- Premade pigtails
    - https://nl.aliexpress.com/item/1005003232354177.html select 5P option
- ESP-01 module (4pk)
    - <https://www.amazon.com/gp/product/B01EA3UJJ4/>
- Cheap 5V to 3.3V regulator (10pk), for those that don't want to make one
    - <https://www.amazon.com/gp/product/B00XAGSHY2/>
- ESP-01 breakout with prewired level shifters and regulator
    - <https://www.amazon.com/gp/product/B01M09B43H/>


## Special thanks

... to Hadley in New Zealand. His blog post, describing baud rate and details of cn105, Raspberry Pi Python code:

<https://nicegear.co.nz/blog/hacking-a-mitsubishi-heat-pump-air-conditioner/>

Wayback machine link as the site no longer exists:
<https://web.archive.org/web/20171007190023/https://nicegear.co.nz/blog/hacking-a-mitsubishi-heat-pump-air-conditioner/>

## License

Licensed under the GNU Lesser General Public License.
https://www.gnu.org/licenses/lgpl-3.0.txt
