"""
Support for Mitsubishi heatpumps using https://github.com/SwiCago/HeatPump over MQTT.

For more details about this platform, please refer to the documentation at
https://github.com/lekobob/mitsu_mqtt
"""

import logging

import voluptuous as vol

from homeassistant.components.mqtt import (
    CONF_STATE_TOPIC, CONF_COMMAND_TOPIC, CONF_QOS, CONF_RETAIN, MqttAttributes, MqttAvailability,
    subscription)

from homeassistant.components.mqtt.climate import (
    CONF_TEMP_STATE_TOPIC, CONF_MODE_LIST)
from homeassistant.components.climate import (
    ClimateDevice)
from homeassistant.components.climate.const import (
    SUPPORT_TARGET_TEMPERATURE,
    SUPPORT_FAN_MODE, SUPPORT_SWING_MODE,
    HVAC_MODE_AUTO, HVAC_MODE_OFF, HVAC_MODE_COOL, HVAC_MODE_DRY, HVAC_MODE_HEAT, HVAC_MODE_FAN_ONLY)
from homeassistant.const import (
    CONF_NAME, CONF_VALUE_TEMPLATE, TEMP_CELSIUS, ATTR_TEMPERATURE)

import homeassistant.components.mqtt as mqtt
import homeassistant.helpers.config_validation as cv
from homeassistant.util.temperature import convert as convert_temp
from numbers import Number
import json

_LOGGER = logging.getLogger(__name__)

DEPENDENCIES = ['mqtt']

DEFAULT_NAME = 'MQTT Climate'

SUPPORT_FLAGS = SUPPORT_TARGET_TEMPERATURE | SUPPORT_FAN_MODE | SUPPORT_SWING_MODE

AVAILABLE_MODES = ["AUTO", "COOL", "DRY", "HEAT", "FAN", "OFF"]

PLATFORM_SCHEMA = mqtt.MQTT_RW_PLATFORM_SCHEMA.extend({
    vol.Optional(CONF_NAME, default=DEFAULT_NAME): cv.string,
    vol.Optional(CONF_TEMP_STATE_TOPIC): mqtt.valid_subscribe_topic,
    vol.Optional(CONF_MODE_LIST, default=AVAILABLE_MODES): cv.ensure_list,
})

TARGET_TEMPERATURE_STEP = 1

ha_to_me = {HVAC_MODE_AUTO: 'AUTO', HVAC_MODE_COOL: 'COOL', HVAC_MODE_DRY: 'DRY', HVAC_MODE_HEAT: 'HEAT', HVAC_MODE_FAN_ONLY: 'FAN', HVAC_MODE_OFF: 'OFF'}
me_to_ha = {v: k for k, v in ha_to_me.items()}

# pylint: disable=unused-argument
async def async_setup_platform(hass, config, async_add_devices, discovery_info=None):
    """Setup the MQTT climate device."""
    value_template = config.get(CONF_VALUE_TEMPLATE)
    if value_template is not None:
        value_template.hass = hass
    async_add_devices([MqttClimate(
        hass,
        config.get(CONF_NAME),
        config.get(CONF_STATE_TOPIC),
        config.get(CONF_TEMP_STATE_TOPIC),
        config.get(CONF_COMMAND_TOPIC),
        config.get(CONF_MODE_LIST),
        config.get(CONF_QOS),
        config.get(CONF_RETAIN),
    )])


class MqttClimate(ClimateDevice):
    """Representation of a Mistsubishi Minisplit Heatpump controlled over MQTT."""


    def __init__(self, hass, name, state_topic, temperature_state_topic, command_topic, modes, qos, retain):
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
        self._fan_modes = ["AUTO", "QUIET", "1", "2", "3", "4"]
        self._fan_mode = None
        self._hvac_modes = modes
        self._hvac_mode = None
        self._current_power = None
        self._swing_modes = ["AUTO", "1", "2", "3", "4", "5", "SWING"]
        self._swing_mode = None
        self._sub_state = None

    async def async_added_to_hass(self):
        """Handle being added to home assistant."""
        await super().async_added_to_hass()
        await self._subscribe_topics()

    async def _subscribe_topics(self):
        """(Re)Subscribe to topics."""
        topics = {}

        def add_subscription(topics, topic, msg_callback):
            if topic is not None:
                topics[topic] = {
                    'topic': topic,
                    'msg_callback': msg_callback,
                    'qos': self._qos}

        def message_received(msg):
            """A new MQTT message has been received."""
            topic = msg.topic
            payload = msg.payload
            parsed = json.loads(payload)
            if topic == self._state_topic:
                self._target_temperature = float(parsed['temperature'])
                self._fan_mode = parsed['fan']
                self._swing_mode = parsed['vane']
                if parsed['power'] == "OFF":
                    _LOGGER.debug("Power Off")
                    self._hvac_mode = "OFF"
                    self._current_power = "OFF"
                else:
                    _LOGGER.debug("Power On")
                    self._hvac_mode = parsed['mode']
                    self._current_power = "ON"
            elif topic == self._temperature_state_topic:
                _LOGGER.debug('Room Temp: {0}'.format(parsed['roomTemperature']))
                self._current_temperature = float(parsed['roomTemperature'])
            else:
                print("unknown topic")
            self.async_write_ha_state()
            _LOGGER.debug("Power=%s, Operation=%s", self._current_power, self._hvac_mode)

        for topic in [self._state_topic, self._temperature_state_topic]:
            add_subscription(topics, topic, message_received)

        self._sub_state = await subscription.async_subscribe_topics(
            self.hass, self._sub_state, topics)


    async def async_will_remove_from_hass(self):
        """Unsubscribe when removed."""
        self._sub_state = await subscription.async_unsubscribe_topics(
            self.hass, self._sub_state)
        await MqttAttributes.async_will_remove_from_hass(self)
        await MqttAvailability.async_will_remove_from_hass(self)

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
    def fan_mode(self):
        """Return the fan setting."""
        if self._fan_mode is None:
            return
        return self._fan_mode.capitalize()

    @property
    def fan_modes(self):
        """List of available fan modes."""
        return [k.capitalize() for k in self._fan_modes]

    @property
    def hvac_mode(self):
        """Return current operation ie. heat, cool, idle."""
        if self._hvac_mode is None:
            return HVAC_MODE_OFF
        if self._current_power == "OFF":
            return HVAC_MODE_OFF
        else:
            return me_to_ha[self._hvac_mode]

    @property
    def hvac_modes(self):
        """List of available operation modes."""
        return [me_to_ha[k] for k in self._hvac_modes]

    @property
    def swing_mode(self):
        """Return the swing setting."""
        if self._swing_mode is None:
            return
        return self._swing_mode.capitalize()

    @property
    def swing_modes(self):
        """List of available swing modes."""
        return [k.capitalize() for k in self._swing_modes]

    async def async_set_temperature(self, **kwargs):
        """Set new target temperatures."""
        if kwargs.get(ATTR_TEMPERATURE) is not None:
            # This is also be set via the mqtt callback
            self._target_temperature = kwargs.get(ATTR_TEMPERATURE)
        self._publish_temperature()
        self.async_write_ha_state()

    async def async_set_fan_mode(self, fan_mode):
        """Set new fan mode."""
        if fan_mode is not None:
            self._fan_mode = fan_mode.upper()
            payload = '{"fan":"' + self._fan_mode + '"}'
            mqtt.async_publish(self.hass, self._command_topic, payload,
                self._qos, self._retain)
            self.async_write_ha_state()

    async def async_set_hvac_mode(self, hvac_mode):
        """Set new operating mode."""
        if hvac_mode is not None:
            self._hvac_mode = ha_to_me[hvac_mode]
            if self._hvac_mode == "OFF":
                payload = '{"power":"OFF"}'
                self._current_power = "OFF"
            else:
                payload = '{"power":"ON","mode":"' + self._hvac_mode + '"}'
                self._current_power = "ON"
            mqtt.async_publish(self.hass, self._command_topic, payload,
                self._qos, self._retain)
            self.async_write_ha_state()

    async def async_set_swing_mode(self, swing_mode):
        """Set new swing mode."""
        if swing_mode is not None:
            self._swing_mode = swing_mode.upper()
            payload = '{"vane":"' + self._swing_mode + '"}'
            mqtt.async_publish(self.hass, self._command_topic, payload,
                self._qos, self._retain)
            self.async_write_ha_state()

    def _publish_temperature(self):
        if self._target_temperature is None:
            return
        unencoded = '{"temperature":' + str(round(self._target_temperature * 2) / 2.0) + '}'
        mqtt.async_publish(self.hass, self._command_topic, unencoded,
                     self._qos, self._retain)
