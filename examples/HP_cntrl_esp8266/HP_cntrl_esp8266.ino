#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <HeatPump.h>

const char* ssid = "esp8266";

const char* html = "<html>\n<head>\n<meta name='viewport' content='width=device-width, initial-scale=2'/>\n"
                   "<style></style>\n"
                   "<body><h3>Heat Pump Demo</h3>TEMP: _ROOMTEMP_\n<form>\n<table>\n"
                   "<tr>\n<td>Power:</td>\n<td>\n_POWER_</td>\n</tr>\n"
                   "<tr>\n<td>Mode:</td>\n<td>\n_MODE_</td>\n</tr>\n"
                   "<tr>\n<td>Temp:</td>\n<td>\n_TEMP_</td>\n</tr>"
                   "<tr>\n<td>Fan:</td>\n<td>\n_FAN_</td>\n</tr>\n"
                   "<tr>\n<td>Vane:</td><td>\n_VANE_</td>\n</tr>\n"
                   "<tr>\n<td>Direction:</td>\n<td>\n_DIR_</td>\n</tr>\n"
                   "</table>\n<br/><input type='submit' value='Change Settings'/>\n</form><br/><br/>"
                   "<form><input type='submit' name='CONNECT' value='Re-Connect'/>\n</form>\n"
                   "</body>\n</html>\n";

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
IPAddress netMsk(255, 255, 255, 0);
DNSServer dnsServer;
ESP8266WebServer server(80);

HeatPump hp;
String wantedSettings[6] = {};

void setup() {
  hp.connect(&Serial);
  hp.getSettings(wantedSettings);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP(ssid);//, password);
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);
  server.on("/", handle_root);
  server.on("/generate_204", handle_root);
  server.onNotFound(handleNotFound);
  server.begin();
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
  hp.sync();
}

String encodeString(String toEncode) {
  toEncode.replace("<", "&lt;");
  toEncode.replace(">", "&gt;");
  toEncode.replace("|", "&vert;");
  return toEncode;
}

String createOptionSelector(String name, const String values[], int len, String value) {
  String str = "<select name='" + name + "'>\n";
  for (int i = 0; i < len; i++) {
    String encoded = encodeString(values[i]);
    str += "<option value='";
    str += String(i);
    str += "'";
    str += values[i] == value ? " selected" : "";
    str += ">";
    str += encoded;
    str += "</option>\n";
  }
  str += "</select>\n";
  return str;
}

void handleNotFound() {
  server.send ( 200, "text/plain", "URI Not Found" );
}

void handle_root() {
  change_states();
  String toSend = html;
  toSend.replace("_POWER_", createOptionSelector("POWER", hp.POWER_MAP, 2, wantedSettings[0]));
  toSend.replace("_MODE_", createOptionSelector("MODE", hp.MODE_MAP, 5, wantedSettings[1]));
  toSend.replace("_TEMP_", createOptionSelector("TEMP", hp.TEMP_MAP, 16, wantedSettings[2]));
  toSend.replace("_FAN_", createOptionSelector("FAN", hp.FAN_MAP, 6, wantedSettings[3]));
  toSend.replace("_VANE_", createOptionSelector("VANE", hp.VANE_MAP, 7, wantedSettings[4]));
  toSend.replace("_DIR_", createOptionSelector("DIR", hp.DIR_MAP, 7, wantedSettings[5]));
  toSend.replace("_ROOMTEMP_", hp.getRoomTemperature() + "°C");
  server.send(200, "text/html", toSend);
  delay(100);
}

void change_states() {
  if (server.hasArg("CONNECT")) {
    hp.connect(&Serial);
  }
  else {
    boolean update = false;
    if (server.hasArg("POWER")) {
      wantedSettings[0] = hp.POWER_MAP[server.arg("POWER").toInt()];
      hp.setPowerSetting(wantedSettings[0]);
      update = true;
    }
    if (server.hasArg("MODE")) {
      wantedSettings[1] = hp.MODE_MAP[server.arg("MODE").toInt()];
      hp.setModeSetting(wantedSettings[1]);
      update = true;
    }
    if (server.hasArg("TEMP")) {
      wantedSettings[2] = hp.TEMP_MAP[server.arg("TEMP").toInt()];
      hp.setTemperature(wantedSettings[2]);
      update = true;
    }
    if (server.hasArg("FAN")) {
      wantedSettings[3] = hp.FAN_MAP[server.arg("FAN").toInt()];
      hp.setFanSpeed(wantedSettings[3]);
      update = true;
    }
    if (server.hasArg("VANE")) {
      wantedSettings[4] = hp.VANE_MAP[server.arg("VANE").toInt()];
      hp.setVaneSetting(wantedSettings[4]);
      update = true;
    }
    if (server.hasArg("DIR")) {
      wantedSettings[5] = hp.DIR_MAP[server.arg("DIR").toInt()];
      hp.setDirectionSetting(wantedSettings[5]);
      update = true;
    }
    if(update) {
      hp.update(); }
  }
}