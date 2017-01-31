
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <HeatPump.h>


// wifi settings
const char* ssid     = "<YOUR WIFI SSID GOES HERE>";
const char* password = "<YOUR WIFI PASSWORD GOES HERE>";

// mqtt server settings
const char* mqtt_server   = "<YOUR MQTT BROKER IP/HOSTNAME GOES HERE>";
const char* mqtt_username = "<YOUR MQTT USERNAME GOES HERE>";
const char* mqtt_password = "<YOUR MQTT PASSWORD GOES HERE>";

// mqtt client settings
const char* client_id                   = "heatpump-controller-1"; // Must be unique on the MQTT network
const char* heatpump_topic              = "heatpump";
const char* heatpump_set_topic          = "heatpump/set";
const char* heatpump_temperature_topic  = "heatpump/temperature";
const char* heatpump_debug_topic        = "heatpump/debug";

// pinouts
const int redLedPin  = 0; // Onboard LED = digital pin 0 (red LED on adafruit ESP8266 huzzah)
const int blueLedPin = 2; // Onboard LED = digital pin 0 (blue LED on adafruit ESP8266 huzzah)

// wifi, mqtt and heatpump client instances
WiFiClient espClient;
PubSubClient mqtt_client(espClient);
HeatPump hp;

void setup() {
  pinMode(redLedPin, OUTPUT);
  digitalWrite(redLedPin, HIGH);
  pinMode(blueLedPin, OUTPUT);
  digitalWrite(blueLedPin, HIGH);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    // wait 500ms, flashing the blue LED to indicate WiFi connecting...
    digitalWrite(blueLedPin, LOW);
    delay(250);
    digitalWrite(blueLedPin, HIGH);
    delay(250);
  }

  // startup mqtt connection
  mqtt_client.setServer(mqtt_server, 1883);
  mqtt_client.setCallback(mqttCallback);

  // connect to the heatpump
  hp.connect(&Serial);
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  if(strcmp(topic, heatpump_set_topic) == 0) { //if the incoming message is on the heatpump_set_topic topic...
    //
    // Step 1: Copy payload into message buffer
    //
    char message[length + 1];
    for (int i = 0; i < length; i++) {
      message[i] = (char)payload[i];
    }
    message[length] = '\0';
  
    //
    // Step 2: Parse message into JSON
    //
    const size_t bufferSize = JSON_OBJECT_SIZE(6);
    DynamicJsonBuffer jsonBuffer(bufferSize);
    JsonObject& root = jsonBuffer.parseObject(message);
  
    if (!root.success())
    {
      mqtt_client.publish(heatpump_debug_topic, "!root.success(): invalid JSON...");
      return;
    }
  
    //
    // Step 3: Retrieve the values
    //

    if (root.containsKey("power")) {
      String power = root["power"];
      hp.setPowerSetting(power);
    }
    
    if (root.containsKey("mode")) {
      String mode = root["mode"];
      hp.setModeSetting(mode);
    }
    
    if (root.containsKey("temperature")) {
      int temperature = root["temperature"];
      hp.setTemperature(temperature);
    }
    
    if (root.containsKey("fan")) {
      String fan = root["fan"];
      hp.setFanSpeed(fan);
    }
    
    if (root.containsKey("vane")) {
      String vane = root["vane"];
      hp.setVaneSetting(vane);
    }
    
    if (root.containsKey("windVane")) {
      String windVane = root["windVane"];
      hp.setWideVaneSetting(windVane);
    }
    
    bool result = hp.update();
  
    if(!result) {
      mqtt_client.publish(heatpump_debug_topic, "heatpump: update() failed");
    }
  } else {
    mqtt_client.publish(heatpump_debug_topic, strcat("heatpump: wrong mqtt topic: ", topic));
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqtt_client.connected()) {
    // Attempt to connect
    if (mqtt_client.connect(client_id, mqtt_username, mqtt_password)) {
      mqtt_client.subscribe(heatpump_set_topic);
    } else {
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


uint last_temp_send = 0;
heatpumpSettings lastSettings;

void loop() {
  if (!mqtt_client.connected()) {
    reconnect();
  }

  int result = hp.checkForUpdate();
  hp.requestInfoAlternate();

  if(result > 0) {
    heatpumpSettings currentSettings = hp.getSettings();
    
    const size_t bufferSize = JSON_OBJECT_SIZE(6);
    DynamicJsonBuffer jsonBuffer(bufferSize);
    
    JsonObject& root = jsonBuffer.createObject();
    
    if(result == 1 && lastSettings != currentSettings) { // only publish the settings if they have changed since last time
      root["power"]       = currentSettings.power;
      root["mode"]        = currentSettings.mode;
      root["temperature"] = currentSettings.temperature;
      root["fan"]         = currentSettings.fan;
      root["vane"]        = currentSettings.vane;
      root["wideVane"]    = currentSettings.wideVane;

      char buffer[512];
      root.printTo(buffer, sizeof(buffer));

      bool retain = true;
      mqtt_client.publish(heatpump_topic, buffer, retain);

      lastSettings = currentSettings;
    } else if(result == 2) {
      if(millis() > (last_temp_send + 60000)) { // only send the temperature every 60s
        root["roomTemperature"] = currentSettings.roomTemperature;

        char buffer[512];
        root.printTo(buffer, sizeof(buffer));
    
        mqtt_client.publish(heatpump_temperature_topic, buffer);

        last_temp_send = millis();
      }
    } else if(result == 3) {
      root["lastUpdate"] = true;
    }
  }

  mqtt_client.loop();
}



