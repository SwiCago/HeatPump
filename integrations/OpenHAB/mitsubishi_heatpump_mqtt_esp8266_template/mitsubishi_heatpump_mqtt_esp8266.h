
// OTA
#define OTA
const char* OTAPass = "abcd";

// wifi settings
const char* ssid     = "ssid";
const char* password = "pass";



// mqtt server settings
const char* mqtt_server   = "192.168.xxx.yyy";
const int mqtt_port       = 1883;
const char* mqtt_username = "<YOUR MQTT USERNAME GOES HERE>";
const char* mqtt_password = "<YOUR MQTT PASSWORD GOES HERE>";

// mqtt client settings
const char* client_id                   = "heatpump-controller-2"; // Must be unique on the MQTT network
const char* heatpump_topic              = "home/heatpump";  //contains current settings
const char* heatpump_set_topic          = "home/heatpump/set"; //listens for commands
const char* heatpump_status_topic       = "home/heatpump/status"; //sends room temp and operation status
const char* heatpump_timers_topic       = "home/heatpump/timers"; //timers

const char* heatpump_debug_topic        = "home/heatpump/debug"; //debug messages
const char* heatpump_debug_set_topic    = "home/heatpump/debug/set"; //enable/disable debug messages

// pinouts
const int redLedPin  = 0; // Onboard LED = digital pin 0 (red LED on adafruit ESP8266 huzzah)
const int blueLedPin = 2; // Onboard LED = digital pin 0 (blue LED on adafruit ESP8266 huzzah)

// sketch settings
const unsigned int SEND_ROOM_TEMP_INTERVAL_MS = 60000;
