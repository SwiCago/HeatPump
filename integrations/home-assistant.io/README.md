

###Installation:

Copy custom_components folder to home assistant config directory.
Resulting folder structure should be <home assistant config directory>/custom_components/mitsubishi_mqtt/climate.py

Add the following to your configuration:
```c++
climate:
   - platform: mitsubishi_mqtt
     name: "Mistubishi Heatpump"
     command_topic: "heatpump/set"
     temperature_state_topic: "heatpump/status"
     state_topic: "heatpump"

```
