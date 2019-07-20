"""
Support for Mitsubishi heatpumps using https://github.com/SwiCago/HeatPump
over MQTT.

For more details about this platform, please refer to the documentation at
https://github.com/lekobob/mitsu_mqtt
"""

import json
import logging

import voluptuous as vol

from homeassistant.components.mqtt import (
    CONF_STATE_TOPIC, CONF_COMMAND_TOPIC, CONF_QOS, CONF_RETAIN)

from homeassistant.components.mqtt.climate import (
    CONF_TEMP_STATE_TOPIC)

from homeassistant.components.climate import (
    ClimateDevice)

from homeassistant.components.climate.const import (
    SUPPORT_TARGET_TEMPERATURE, SUPPORT_FAN_MODE, SUPPORT_SWING_MODE,
    HVAC_MODE_AUTO, HVAC_MODE_COOL, HVAC_MODE_DRY, HVAC_MODE_HEAT,
    HVAC_MODE_FAN_ONLY, HVAC_MODE_OFF, CURRENT_HVAC_OFF,
    CURRENT_HVAC_HEAT, CURRENT_HVAC_COOL, CURRENT_HVAC_DRY,
    CURRENT_HVAC_IDLE)

from homeassistant.const import (
    CONF_NAME, CONF_VALUE_TEMPLATE, TEMP_CELSIUS, ATTR_TEMPERATURE)

import homeassistant.components.mqtt as mqtt
import homeassistant.helpers.config_validation as cv

_LOGGER = logging.getLogger(__name__)

DEPENDENCIES = ['mqtt']

DEFAULT_NAME = 'MQTT Climate'

SUPPORT_FLAGS = SUPPORT_TARGET_TEMPERATURE | SUPPORT_FAN_MODE | \
    SUPPORT_SWING_MODE

PLATFORM_SCHEMA = mqtt.MQTT_RW_PLATFORM_SCHEMA.extend({
    vol.Optional(CONF_NAME, default=DEFAULT_NAME): cv.string,
    vol.Optional(CONF_TEMP_STATE_TOPIC): mqtt.valid_subscribe_topic
})

TARGET_TEMPERATURE_STEP = 1

HA_TO_ME = {
    HVAC_MODE_AUTO: 'AUTO',
    HVAC_MODE_COOL: 'COOL',
    HVAC_MODE_DRY: 'DRY',
    HVAC_MODE_HEAT: 'HEAT',
    HVAC_MODE_FAN_ONLY: 'FAN',
    HVAC_MODE_OFF: 'OFF'
}

ME_TO_HA = {v: k for k, v in HA_TO_ME.items()}

FAN_MODES = ['AUTO', 'QUIET', '1', '2', '3', '4']
HVAC_MODES = ['AUTO', 'COOL', 'DRY', 'HEAT', 'FAN', 'OFF']
SWING_MODES = ['AUTO', '1', '2', '3', '4', '5', 'SWING']


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


# pylint: disable=too-many-instance-attributes,abstract-method
class MqttClimate(ClimateDevice):
    """
    Representation of a Mistsubishi Minisplit Heatpump controlled
    over MQTT.
    """

    # pylint: disable=too-many-arguments
    def __init__(self, hass, name, state_topic, temperature_state_topic,
                 command_topic, qos, retain):
        """Initialize the MQTT Heatpump."""
        self._state = False

        self.hass = hass
        self._name = name
        self._state_topic = state_topic
        self._temperature_state_topic = temperature_state_topic
        self._command_topic = command_topic
        self._qos = qos
        self._retain = retain

        self._current_temperature = None
        self._target_temperature = None
        self._current_fan_mode = None
        self._current_operation = None
        self._current_status = False
        self._current_power = None
        self._current_swing_mode = None

        def message_received(msg):
            """A new MQTT message has been received."""
            parsed = json.loads(msg.payload)

            if msg.topic == self._state_topic:
                self._target_temperature = float(parsed['temperature'])
                self._current_fan_mode = parsed['fan']
                self._current_swing_mode = parsed['vane']
                if parsed['power'] == 'OFF':
                    _LOGGER.debug('Power Off')
                    self._current_operation = 'OFF'
                    self._current_power = 'OFF'
                else:
                    _LOGGER.debug('Power On')
                    self._current_operation = parsed['mode']
                    self._current_power = 'ON'
            elif msg.topic == self._temperature_state_topic:
                _LOGGER.debug('Room Temp: %s', parsed['roomTemperature'])
                self._current_temperature = float(parsed['roomTemperature'])
                self._current_status = bool(parsed['operating'])
            else:
                _LOGGER.debug('Unknown topic: %s', msg.topic)

            self.schedule_update_ha_state()

            _LOGGER.debug('Power=%s, Operation=%s', self._current_power,
                          self._current_operation)

        for topic in (self._state_topic, self._temperature_state_topic):
            mqtt.subscribe(hass, topic, message_received, self._qos)

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
        """Polling not needed."""
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
        return self._current_fan_mode.capitalize()

    @property
    def fan_modes(self):
        """List of available fan modes."""
        return [k.capitalize() for k in FAN_MODES]

    @property
    def hvac_action(self):
        if self._current_power == 'OFF':
            return CURRENT_HVAC_OFF

        if self._current_status:
            if self._current_operation == 'HEAT':
                return CURRENT_HVAC_HEAT
            if self._current_operation == 'COOL':
                return CURRENT_HVAC_COOL
            if self._current_operation == 'DRY':
                return CURRENT_HVAC_DRY

        return CURRENT_HVAC_IDLE

    @property
    def hvac_mode(self):
        """Return current operation ie. heat, cool, idle."""
        if self._current_power == 'OFF':
            return HVAC_MODE_OFF

        return ME_TO_HA.get(self._current_operation)

    @property
    def hvac_modes(self):
        """List of available operation modes."""
        return [ME_TO_HA[k] for k in HVAC_MODES]

    @property
    def swing_mode(self):
        """Return the swing setting."""
        return self._current_swing_mode.capitalize()

    @property
    def swing_modes(self):
        """List of available swing modes."""
        return [k.capitalize() for k in SWING_MODES]

    def set_temperature(self, **kwargs):
        """Set new target temperatures."""
        if kwargs.get(ATTR_TEMPERATURE) is not None:
            # This is also be set via the mqtt callback
            self._target_temperature = kwargs.get(ATTR_TEMPERATURE)

        if self._target_temperature is not None:
            temperature = round(self._target_temperature * 2) / 2.0

            payload = {'temperature': str(temperature)}

            mqtt.publish(self.hass, self._command_topic, json.dumps(payload),
                         self._qos, self._retain)

        self.schedule_update_ha_state()

    def set_fan_mode(self, fan_mode):
        """Set new fan mode."""
        if fan_mode is not None:
            self._current_fan_mode = fan_mode.upper()

            payload = {'fan': self._current_fan_mode}

            mqtt.publish(self.hass, self._command_topic, json.dumps(payload),
                         self._qos, self._retain)
            self.schedule_update_ha_state()

    def set_hvac_mode(self, hvac_mode):
        """Set new operating mode."""
        self._current_operation = HA_TO_ME[hvac_mode]

        if self._current_operation == 'OFF':
            payload = {'power': 'OFF'}
            self._current_power = 'OFF'
        else:
            payload = {'power': 'ON', 'mode': self._current_operation}
            self._current_power = 'ON'

        mqtt.publish(self.hass, self._command_topic, json.dumps(payload),
                     self._qos, self._retain)
        self.schedule_update_ha_state()

    def set_swing_mode(self, swing_mode):
        """Set new swing mode."""
        if swing_mode is not None:
            self._current_swing_mode = swing_mode.upper()

            payload = {'vane': self._current_swing_mode}

            mqtt.publish(self.hass, self._command_topic, json.dumps(payload),
                         self._qos, self._retain)
            self.schedule_update_ha_state()
