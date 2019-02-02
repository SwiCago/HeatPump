#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <HeatPump.h>

const char* ssid = "esp8266";

const char* html = "<html>\n<head>\n<meta name='viewport' content='width=device-width, initial-scale=2'/>\n"
                   "<meta http-equiv='refresh' content='_RATE_; url=/'/>\n"
                   "<style></style>\n"
                   "<body><h3>Heat Pump Demo</h3>TEMP: _ROOMTEMP_\n&deg;C<form autocomplete='off' method='post' action=''>\n<table>\n"
                   "<tr>\n<td>Power:</td>\n<td>\n_POWER_</td>\n</tr>\n"
                   "<tr>\n<td>Mode:</td>\n<td>\n_MODE_</td>\n</tr>\n"
                   "<tr>\n<td>Temp:</td>\n<td>\n_TEMP_</td>\n</tr>"
                   "<tr>\n<td>Fan:</td>\n<td>\n_FAN_</td>\n</tr>\n"
                   "<tr>\n<td>Vane:</td><td>\n_VANE_</td>\n</tr>\n"
                   "<tr>\n<td>WideVane:</td>\n<td>\n_WVANE_</td>\n</tr>\n"
                   "</table>\n<br/><input type='submit' value='Change Settings'/>\n</form><br/><br/>"
                   "<form><input type='submit' name='CONNECT' value='Re-Connect'/>\n</form>\n"
                   "</body>\n</html>\n";

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
IPAddress netMsk(255, 255, 255, 0);
DNSServer dnsServer;
ESP8266WebServer server(80);

HeatPump hp;

void setup() {
  hp.connect(&Serial);
  hp.setSettings({ //set some default settings
    "ON",  /* ON/OFF */
    "FAN", /* HEAT/COOL/FAN/DRY/AUTO */
    26,    /* Between 16 and 31 */
    "4",   /* Fan speed: 1-4, AUTO, or QUIET */
    "3",   /* Air direction (vertical): 1-5, SWING, or AUTO */
    "|"    /* Air direction (horizontal): <<, <, |, >, >>, <>, or SWING */
  });
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
    str += values[i];
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
  int rate = change_states() ? 0 : 60;
  String toSend = html;
  toSend.replace("_RATE_", String(rate));
  String power[2] = {"OFF", "ON"}; 
  toSend.replace("_POWER_", createOptionSelector("POWER", power, 2, hp.getPowerSetting()));
  String mode[5] = {"HEAT", "DRY", "COOL", "FAN", "AUTO"};
  toSend.replace("_MODE_", createOptionSelector("MODE", mode, 5, hp.getModeSetting()));
  String temp[16] = {"31", "30", "29", "28", "27", "26", "25", "24", "23", "22", "21", "20", "19", "18", "17", "16"};
  toSend.replace("_TEMP_", createOptionSelector("TEMP", temp, 16, String(hp.getTemperature()).substring(0,2)));
  String fan[6] = {"AUTO", "QUIET", "1", "2", "3", "4"};
  toSend.replace("_FAN_", createOptionSelector("FAN", fan, 6, hp.getFanSpeed()));
  String vane[7] = {"AUTO", "1", "2", "3", "4", "5", "SWING"};
  toSend.replace("_VANE_", createOptionSelector("VANE", vane, 7, hp.getVaneSetting()));
  String widevane[7] = {"<<", "<", "|", ">", ">>", "<>", "SWING"}; 
  toSend.replace("_WVANE_", createOptionSelector("WIDEVANE", widevane, 7, hp.getWideVaneSetting()));
  toSend.replace("_ROOMTEMP_", String(hp.getRoomTemperature()));
  server.send(200, "text/html", toSend);
  delay(100);
}

bool change_states() {
  bool updated = false;
  if (server.hasArg("CONNECT")) {
    hp.connect(&Serial);
  }
  else {
    if (server.hasArg("POWER")) {
      hp.setPowerSetting(server.arg("POWER").c_str());
      updated = true;
    }
    if (server.hasArg("MODE")) {
      hp.setModeSetting(server.arg("MODE").c_str());
      updated = true;
    }
    if (server.hasArg("TEMP")) {
      hp.setTemperature(server.arg("TEMP").toFloat());
      updated = true;
    }
    if (server.hasArg("FAN")) {
      hp.setFanSpeed(server.arg("FAN").c_str());
      updated = true;
    }
    if (server.hasArg("VANE")) {
      hp.setVaneSetting(server.arg("VANE").c_str());
      updated = true;
    }
    if (server.hasArg("DIR")) {
      hp.setWideVaneSetting(server.arg("WIDEVANE").c_str());
      updated = true;
    }
    hp.update(); 
  }
  return updated;
}
