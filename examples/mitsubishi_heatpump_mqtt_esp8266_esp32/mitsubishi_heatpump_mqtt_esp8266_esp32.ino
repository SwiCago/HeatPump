
#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <HeatPump.h>

#include "mitsubishi_heatpump_mqtt_esp8266_esp32.h"

#ifdef OTA
#ifdef ESP32
#include <WiFiUdp.h>
#include <ESPmDNS.h>
#else
#include <ESP8266mDNS.h>
#endif
#include <ArduinoOTA.h>
#endif

// wifi, mqtt and heatpump client instances
WiFiClient espClient;
PubSubClient mqtt_client(espClient);
HeatPump hp;
unsigned long lastTempSend;

// debug mode, when true, will send all packets received from the heatpump to topic heatpump_debug_topic
// this can also be set by sending "on" to heatpump_debug_set_topic
bool _debugMode = true;


void setup() {
  pinMode(redLedPin, OUTPUT);
  digitalWrite(redLedPin, HIGH);
  pinMode(blueLedPin, OUTPUT);
  digitalWrite(blueLedPin, HIGH);

  WiFi.hostname(client_id);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    // wait 500ms, flashing the blue LED to indicate WiFi connecting...
    digitalWrite(blueLedPin, LOW);
    delay(250);
    digitalWrite(blueLedPin, HIGH);
    delay(250);
  }

  // startup mqtt connection
  mqtt_client.setServer(mqtt_server, mqtt_port);
  mqtt_client.setCallback(mqttCallback);
  mqttConnect();

  // connect to the heatpump. Callbacks first so that the hpPacketDebug callback is available for connect()
  hp.setSettingsChangedCallback(hpSettingsChanged);
  hp.setStatusChangedCallback(hpStatusChanged);
  hp.setPacketCallback(hpPacketDebug);

#ifdef OTA
  ArduinoOTA.setHostname(client_id);
  ArduinoOTA.setPassword(ota_password);
  ArduinoOTA.begin();
#endif

  hp.connect(&Serial);

  lastTempSend = millis();
}

void hpSettingsChanged() {
  const size_t bufferSize = JSON_OBJECT_SIZE(6);
  DynamicJsonDocument root(bufferSize);

  heatpumpSettings currentSettings = hp.getSettings();

  root["power"]       = currentSettings.power;
  root["mode"]        = currentSettings.mode;
  root["temperature"] = currentSettings.temperature;
  root["fan"]         = currentSettings.fan;
  root["vane"]        = currentSettings.vane;
  root["wideVane"]    = currentSettings.wideVane;
  //root["iSee"]        = currentSettings.iSee;

  char buffer[512];
  serializeJson(root, buffer);

  bool retain = true;
  if (!mqtt_client.publish(heatpump_topic, buffer, retain)) {
    mqtt_client.publish(heatpump_debug_topic, "failed to publish to heatpump topic");
  }
}

void hpStatusChanged(heatpumpStatus currentStatus) {
  // send room temp and operating info
  const size_t bufferSizeInfo = JSON_OBJECT_SIZE(2);
  DynamicJsonDocument rootInfo(bufferSizeInfo);

  rootInfo["roomTemperature"] = currentStatus.roomTemperature;
  rootInfo["operating"]       = currentStatus.operating;

  char bufferInfo[512];
  serializeJson(rootInfo, bufferInfo);

  if (!mqtt_client.publish(heatpump_status_topic, bufferInfo, true)) {
    mqtt_client.publish(heatpump_debug_topic, "failed to publish to room temp and operation status to heatpump/status topic");
  }

  // send the timer info
  const size_t bufferSizeTimers = JSON_OBJECT_SIZE(5);
  DynamicJsonDocument rootTimers(bufferSizeTimers);

  rootTimers["mode"]          = currentStatus.timers.mode;
  rootTimers["onMins"]        = currentStatus.timers.onMinutesSet;
  rootTimers["onRemainMins"]  = currentStatus.timers.onMinutesRemaining;
  rootTimers["offMins"]       = currentStatus.timers.offMinutesSet;
  rootTimers["offRemainMins"] = currentStatus.timers.offMinutesRemaining;

  char bufferTimers[512];
  serializeJson(rootTimers, bufferTimers);

  if (!mqtt_client.publish(heatpump_timers_topic, bufferTimers, true)) {
    mqtt_client.publish(heatpump_debug_topic, "failed to publish timer info to heatpump/status topic");
  }
}

void hpPacketDebug(byte* packet, unsigned int length, char* packetDirection) {
  if (_debugMode) {
    String message;
    for (int idx = 0; idx < length; idx++) {
      if (packet[idx] < 16) {
        message += "0"; // pad single hex digits with a 0
      }
      message += String(packet[idx], HEX) + " ";
    }

    const size_t bufferSize = JSON_OBJECT_SIZE(6);
    DynamicJsonDocument root(bufferSize);

    root[packetDirection] = message;

    char buffer[512];
    serializeJson(root, buffer);

    if (!mqtt_client.publish(heatpump_debug_topic, buffer)) {
      mqtt_client.publish(heatpump_debug_topic, "failed to publish to heatpump/debug topic");
    }
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Copy payload into message buffer
  char message[length + 1];
  for (int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';

  if (strcmp(topic, heatpump_set_topic) == 0) { //if the incoming message is on the heatpump_set_topic topic...
    // Parse message into JSON
    const size_t bufferSize = JSON_OBJECT_SIZE(6);
    DynamicJsonDocument root(bufferSize);
    DeserializationError error = deserializeJson(root, message);

    if (error) {
      mqtt_client.publish(heatpump_debug_topic, "!root.success(): invalid JSON on heatpump_set_topic...");
      return;
    }

    // Step 3: Retrieve the values
    if (root.containsKey("power")) {
      const char* power = root["power"];
      hp.setPowerSetting(power);
    }

    if (root.containsKey("mode")) {
      const char* mode = root["mode"];
      hp.setModeSetting(mode);
    }

    if (root.containsKey("temperature")) {
      float temperature = root["temperature"];
      hp.setTemperature(temperature);
    }

    if (root.containsKey("fan")) {
      const char* fan = root["fan"];
      hp.setFanSpeed(fan);
    }

    if (root.containsKey("vane")) {
      const char* vane = root["vane"];
      hp.setVaneSetting(vane);
    }

    if (root.containsKey("wideVane")) {
      const char* wideVane = root["wideVane"];
      hp.setWideVaneSetting(wideVane);
    }

    if (root.containsKey("remoteTemp")) {
      float remoteTemp = root["remoteTemp"];
      hp.setRemoteTemperature(remoteTemp);
    }
    else if (root.containsKey("custom")) {
      String custom = root["custom"];

      // copy custom packet to char array
      char buffer[(custom.length() + 1)]; // +1 for the NULL at the end
      custom.toCharArray(buffer, (custom.length() + 1));

      byte bytes[20]; // max custom packet bytes is 20
      int byteCount = 0;
      char *nextByte;

      // loop over the byte string, breaking it up by spaces (or at the end of the line - \n)
      nextByte = strtok(buffer, " ");
      while (nextByte != NULL && byteCount < 20) {
        bytes[byteCount] = strtol(nextByte, NULL, 16); // convert from hex string
        nextByte = strtok(NULL, "   ");
        byteCount++;
      }

      // dump the packet so we can see what it is. handy because you can run the code without connecting the ESP to the heatpump, and test sending custom packets
      hpPacketDebug(bytes, byteCount, "customPacket");

      hp.sendCustomPacket(bytes, byteCount);
    }
    else {
      bool result = hp.update();

      if (!result) {
        mqtt_client.publish(heatpump_debug_topic, "heatpump: update() failed");
      }
    }

  } else if (strcmp(topic, heatpump_debug_set_topic) == 0) { //if the incoming message is on the heatpump_debug_set_topic topic...
    if (strcmp(message, "on") == 0) {
      _debugMode = true;
      mqtt_client.publish(heatpump_debug_topic, "debug mode enabled");
    } else if (strcmp(message, "off") == 0) {
      _debugMode = false;
      mqtt_client.publish(heatpump_debug_topic, "debug mode disabled");
    }
  } else {//should never get called, as that would mean something went wrong with subscribe
    mqtt_client.publish(heatpump_debug_topic, "heatpump: wrong topic received");
  }
}

void mqttConnect() {
  // Loop until we're reconnected
  while (!mqtt_client.connected()) {
    // Attempt to connect
    if (mqtt_client.connect(client_id, mqtt_username, mqtt_password)) {
      mqtt_client.subscribe(heatpump_set_topic);
      mqtt_client.subscribe(heatpump_debug_set_topic);
    } else {
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  if (!mqtt_client.connected()) {
    mqttConnect();
  }

  hp.sync();

  if (millis() > (lastTempSend + SEND_ROOM_TEMP_INTERVAL_MS)) { // only send the temperature every 60s
    hpStatusChanged(hp.getStatus());
    lastTempSend = millis();
  }

  mqtt_client.loop();

#ifdef OTA
  ArduinoOTA.handle();
#endif
}
