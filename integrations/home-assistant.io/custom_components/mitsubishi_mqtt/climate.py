"""
Support for Mitsubishi heatpumps using https://github.com/SwiCago/HeatPump over MQTT.

For more details about this platform, please refer to the documentation at
https://github.com/lekobob/mitsu_mqtt
"""

import logging

import voluptuous as vol

from homeassistant.components.mqtt import (
    CONF_STATE_TOPIC, CONF_COMMAND_TOPIC, CONF_QOS, CONF_RETAIN)

from homeassistant.components.mqtt.climate import (
    CONF_TEMP_STATE_TOPIC)

from homeassistant.components.climate import (
    ClimateDevice)

from homeassistant.components.climate.const import (
    SUPPORT_TARGET_TEMPERATURE, SUPPORT_OPERATION_MODE,
    SUPPORT_FAN_MODE, SUPPORT_SWING_MODE,
	STATE_AUTO, STATE_COOL, STATE_DRY, STATE_HEAT, STATE_FAN_ONLY)

from homeassistant.const import (
    CONF_NAME, CONF_VALUE_TEMPLATE, TEMP_CELSIUS, ATTR_TEMPERATURE, STATE_OFF)

import homeassistant.components.mqtt as mqtt
import homeassistant.helpers.config_validation as cv
from homeassistant.util.temperature import convert as convert_temp
from numbers import Number
import json

_LOGGER = logging.getLogger(__name__)

DEPENDENCIES = ['mqtt']

DEFAULT_NAME = 'MQTT Climate'

SUPPORT_FLAGS = SUPPORT_TARGET_TEMPERATURE | SUPPORT_OPERATION_MODE | SUPPORT_FAN_MODE | SUPPORT_SWING_MODE

PLATFORM_SCHEMA = mqtt.MQTT_RW_PLATFORM_SCHEMA.extend({
    vol.Optional(CONF_NAME, default=DEFAULT_NAME): cv.string,
    vol.Optional(CONF_TEMP_STATE_TOPIC): mqtt.valid_subscribe_topic
})

TARGET_TEMPERATURE_STEP = 1

ha_to_me = {STATE_AUTO: 'AUTO', STATE_COOL: 'COOL', STATE_DRY: 'DRY', STATE_HEAT: 'HEAT', STATE_FAN_ONLY: 'FAN', STATE_OFF: 'OFF'}
me_to_ha = {v: k for k, v in ha_to_me.items()}

# pylint: disable=unused-argument
def setup_platform(hass, config, add_devices, discovery_info=None):
    """Setup the MQTT climate device."""
    value_template = config.get(CONF_VALUE_TEMPLATE)
    if value_template is not None:
        value_template.hass = hass
    add_devices([MqttClimate(
        hass,
        config.get(CONF_NAME),
        config.get(CONF_STATE_TOPIC),
        config.get(CONF_TEMP_STATE_TOPIC),
        config.get(CONF_COMMAND_TOPIC),
        config.get(CONF_QOS),
        config.get(CONF_RETAIN),
    )])


class MqttClimate(ClimateDevice):
    """Representation of a Mistsubishi Minisplit Heatpump controlled over MQTT."""


    def __init__(self, hass, name, state_topic, temperature_state_topic, command_topic, qos, retain):
        """Initialize the MQTT Heatpump."""
        self._state = False
        self._hass = hass
        self.hass = hass
        self._name = name
        self._state_topic = state_topic
        self._temperature_state_topic = temperature_state_topic
        self._command_topic = command_topic
        self._qos = qos
        self._retain = retain
        self._current_temperature = None
        self._target_temperature = None
        self._fan_list = ["AUTO", "QUIET", "1", "2", "3", "4"]
        self._current_fan_mode = None
        self._operation_list = ["AUTO", "COOL", "DRY", "HEAT", "FAN", "OFF"]
        self._current_operation = None
        self._current_power = None
        self._swing_list = ["AUTO", "1", "2", "3", "4", "5", "SWING"]
        self._current_swing_mode = None

        def message_received(msg):
            """A new MQTT message has been received."""
            parsed = json.loads(msg.payload)
            if msg.topic == self._state_topic:
                self._target_temperature = float(parsed['temperature'])
                self._current_fan_mode = parsed['fan']
                self._current_swing_mode = parsed['vane']
                if parsed['power'] == "OFF":
                    _LOGGER.debug("Power Off")
                    self._current_operation = "OFF"
                    self._current_power = "OFF"
                else:
                    _LOGGER.debug("Power On")
                    self._current_operation = parsed['mode']
                    self._current_power = "ON"
            elif msg.topic == self._temperature_state_topic:
                _LOGGER.debug('Room Temp: {0}'.format(parsed['roomTemperature']))
                self._current_temperature = float(parsed['roomTemperature'])
            else:
                print("unknown topic")
            self.schedule_update_ha_state()
            _LOGGER.debug("Power=%s, Operation=%s", self._current_power, self._current_operation)

        for topic in [self._state_topic, self._temperature_state_topic]:
            mqtt.subscribe(
                hass, topic, message_received, self._qos)

    @property
    def supported_features(self):
        """Return the list of supported features."""
        return SUPPORT_FLAGS

    @property
    def target_temperature_step(self):
        """Return the target temperature step."""
        return TARGET_TEMPERATURE_STEP

    @property
    def should_poll(self):
        """Polling not needed for a demo climate device."""
        return False

    @property
    def name(self):
        """Return the name of the climate device."""
        return self._name

    @property
    def temperature_unit(self):
        """Return the unit of measurement."""
        return TEMP_CELSIUS

    @property
    def target_temperature(self):
        """Return the temperature we try to reach."""
        return self._target_temperature

    @property
    def current_temperature(self):
        """Return the current temperature."""
        return self._current_temperature

    @property
    def current_fan_mode(self):
        """Return the fan setting."""
        return self._current_fan_mode.capitalize()

    @property
    def fan_list(self):
        """List of available fan modes."""
        return [k.capitalize() for k in self._fan_list]

    @property
    def current_operation(self):
        """Return current operation ie. heat, cool, idle."""
        if self._current_power == "OFF":
            return STATE_OFF
        else:
            return me_to_ha[self._current_operation]

    @property
    def operation_list(self):
        """List of available operation modes."""
        return [me_to_ha[k] for k in self._operation_list]

    @property
    def current_swing_mode(self):
        """Return the swing setting."""
        return self._current_swing_mode.capitalize()

    @property
    def swing_list(self):
        """List of available swing modes."""
        return [k.capitalize() for k in self._swing_list]

    def set_temperature(self, **kwargs):
        """Set new target temperatures."""
        if kwargs.get(ATTR_TEMPERATURE) is not None:
            # This is also be set via the mqtt callback
            self._target_temperature = kwargs.get(ATTR_TEMPERATURE)
        self._publish_temperature()
        self.schedule_update_ha_state()

    def set_fan_mode(self, fan):
        """Set new fan mode."""
        if fan is not None:
            self._current_fan_mode = fan.upper()
            payload = '{"fan":"' + self._current_fan_mode + '"}'
            mqtt.publish(self.hass, self._command_topic, payload,
                self._qos, self._retain)
            self.schedule_update_ha_state()

    def set_operation_mode(self, operation_mode):
        """Set new operating mode."""
        self._current_operation = ha_to_me[operation_mode]
        if self._current_operation == "OFF":
            payload = '{"power":"OFF"}'
            self._current_power = "OFF"
        else:
            payload = '{"power":"ON","mode":"' + self._current_operation + '"}'
            self._current_power = "ON"
        mqtt.publish(self.hass, self._command_topic, payload,
            self._qos, self._retain)
        self.schedule_update_ha_state()

    def set_swing_mode(self, swing):
        """Set new swing mode."""
        if swing is not None:
            self._current_swing_mode = swing.upper()
            payload = '{"vane":"' + self._current_swing_mode + '"}'
            mqtt.publish(self.hass, self._command_topic, payload,
                self._qos, self._retain)
            self.schedule_update_ha_state()

    def _publish_temperature(self):
        if self._target_temperature is None:
            return
        unencoded = '{"temperature":' + str(round(self._target_temperature * 2) / 2.0) + '}'
        mqtt.publish(self.hass, self._command_topic, unencoded,
                     self._qos, self._retain)
