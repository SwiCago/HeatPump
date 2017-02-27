

###Installation:

copy mitsubishi_mqtt.py to <home assistant config directory>/custom_components/climate/

add the following to your configuration:
```c++
climate:
   - platform: mitsubishi_mqtt
     name: "Mistubishi Heatpump"
     command_topic: "heatpump/set"
     temperature_state_topic: "heatpump/temperature"
     state_topic: "heatpump"

```
