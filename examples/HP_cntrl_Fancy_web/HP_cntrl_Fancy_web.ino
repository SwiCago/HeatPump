#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <HeatPump.h>
#include "HP_cntrl_Fancy_web.h"

const char* ssid = "HEATPUMP";


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


void handleNotFound() {
  server.send ( 200, "text/plain", "URI Not Found" );
}

void handle_root() {
  heatpumpSettings settings = hp.getSettings();
  settings = change_states(settings);
  String toSend = html;
  toSend.replace("_RATE_", "60");
  toSend.replace("_ROOMTEMP_", String(hp.getRoomTemperature()));
  toSend.replace("_POWER_", strcmp(settings.power, "ON") == 0 ? "checked" : "");
  if(strcmp(settings.mode, "HEAT") == 0) {
    toSend.replace("_MODE_H_","checked");
  }
  else if(strcmp(settings.mode, "DRY") == 0) {
    toSend.replace("_MODE_D_","checked");
  }
  else if(strcmp(settings.mode, "COOL") == 0) {
    toSend.replace("_MODE_C_","checked");
  }
  else if(strcmp(settings.mode, "FAN") == 0) {
    toSend.replace("_MODE_F_","checked");
  }
  else if(strcmp(settings.mode, "AUTO") == 0) {
    toSend.replace("_MODE_A_","checked");
  }
  if(strcmp(settings.fan, "AUTO") == 0) {
    toSend.replace("_FAN_A_","checked");
  }
  else if(strcmp(settings.fan, "QUIET") == 0) {
    toSend.replace("_FAN_Q_","checked");
  }
  else if(strcmp(settings.fan, "1") == 0) {
    toSend.replace("_FAN_1_","checked");
  }
  else if(strcmp(settings.fan, "2") == 0) {
    toSend.replace("_FAN_2_","checked");
  }
  else if(strcmp(settings.fan, "3") == 0) {
    toSend.replace("_FAN_3_","checked");
  }
  else if(strcmp(settings.fan, "4") == 0) {
    toSend.replace("_FAN_4_","checked");
  }
 
  toSend.replace("_VANE_V_",settings.vane);
  if(strcmp(settings.vane, "AUTO") == 0) {
    toSend.replace("_VANE_C_","rotate0");
    toSend.replace("_VANE_T_","AUTO");
  }
  else if(strcmp(settings.vane, "1") == 0) {
    toSend.replace("_VANE_C_","rotate0");
    toSend.replace("_VANE_T_","&#10143;");
  }
  else if(strcmp(settings.vane, "2") == 0) {
    toSend.replace("_VANE_C_","rotate22");
    toSend.replace("_VANE_T_","&#10143;");
  }
  else if(strcmp(settings.vane, "3") == 0) {
    toSend.replace("_VANE_C_","rotate45");
    toSend.replace("_VANE_T_","&#10143;");
  }
  else if(strcmp(settings.vane, "4") == 0) {
    toSend.replace("_VANE_C_","rotate67");
    toSend.replace("_VANE_T_","&#10143;");
  }
  else if(strcmp(settings.vane, "5") == 0) {
    toSend.replace("_VANE_C_","rotate90");
    toSend.replace("_VANE_T_","&#10143;");
  }
  else if(strcmp(settings.vane, "SWING") == 0) {
    toSend.replace("_VANE_C_","rotateV");
    toSend.replace("_VANE_T_","&#10143;");
  }
  toSend.replace("_WIDEVANE_V_",settings.wideVane);
  if(strcmp(settings.wideVane, "<<") == 0) {
    toSend.replace("_WIDEVANE_C_","rotate157");
    toSend.replace("_WIDEVANE_T_","&#10143;");
  }
  else if(strcmp(settings.wideVane, "<") == 0) {
    toSend.replace("_WIDEVANE_C_","rotate124");
    toSend.replace("_WIDEVANE_T_","&#10143;");
  }
  else if(strcmp(settings.wideVane, "|") == 0) {
    toSend.replace("_WIDEVANE_C_","rotate90");
    toSend.replace("_WIDEVANE_T_","&#10143;");
  }
  else if(strcmp(settings.wideVane, ">") == 0) {
    toSend.replace("_WIDEVANE_C_","rotate57");
    toSend.replace("_WIDEVANE_T_","&#10143;");
  }
  else if(strcmp(settings.wideVane, ">>") == 0) {
    toSend.replace("_WIDEVANE_C_","rotate22");
    toSend.replace("_WIDEVANE_T_","&#10143;");
  }
  else if(strcmp(settings.wideVane, "<>") == 0) {
    toSend.replace("_WIDEVANE_C_","");
    toSend.replace("_WIDEVANE_T_","<div class='rotate124'>&#10143;</div>&nbsp;<div class='rotate57'>&#10143;</div>");
  }
  else if(strcmp(settings.wideVane, "SWING") == 0) {
    toSend.replace("_WIDEVANE_C_","rotateH");
    toSend.replace("_WIDEVANE_T_","&#10143;");
  }
  toSend.replace("_TEMP_", String(hp.getTemperature()));
  server.send(200, "text/html", toSend);
  delay(100);
}

heatpumpSettings change_states(heatpumpSettings settings) {
  if (server.hasArg("CONNECT")) {
    hp.connect(&Serial);
  }
  else {
    bool update = false;
    if (server.hasArg("PWRCHK")) {
      settings.power=server.hasArg("POWER") ? "ON" : "OFF";
      update = true;
    }
    if (server.hasArg("MODE")) {
      settings.mode=server.arg("MODE").c_str();
      update = true;
    }
    if (server.hasArg("TEMP")) {
      settings.temperature=server.arg("TEMP").toInt();
      update = true;
    }
    if (server.hasArg("FAN")) {
      settings.fan=server.arg("FAN").c_str();
      update = true;
    }
    if (server.hasArg("VANE")) {
      settings.vane=server.arg("VANE").c_str();
      update = true;
    }
    if (server.hasArg("WIDEVANE")) {
      settings.wideVane=server.arg("WIDEVANE").c_str();
      update = true;
    }
    if(update) {
      hp.setSettings(settings);
      hp.update();
    } 
  }
  return settings;
}
