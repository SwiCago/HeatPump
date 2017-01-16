/*
  HeatPump.cpp - Mitsubishi Heat Pump control library for Arduino
  Copyright (c) 2017 Al Betschart.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "HeatPump.h"

// Initialize Class Variables //////////////////////////////////////////////////

unsigned int lastSend = 0;
boolean info_mode = false;
// Constructors ////////////////////////////////////////////////////////////////

HeatPump::HeatPump() {
  String defaultSettings[7] = {POWER_MAP[0],MODE_MAP[3],TEMP_MAP[9],FAN_MAP[2],VANE_MAP[1],DIR_MAP[2],ROOM_TEMP_MAP[12]};
  for(int i = 0; i < 6; i++) {
    currentSettings[i] = defaultSettings[i];
  }
  for(int i = 0; i < 5; i++) {
    wantedSettings[i] = defaultSettings[i];
  }
}

// Public Methods //////////////////////////////////////////////////////////////

void HeatPump::connect(HardwareSerial *serial) {
  _HardSerial = serial;
  _HardSerial->begin(2400, SERIAL_8E1);
  delay(2000);
  for(int cnt = 0; cnt < 2; cnt++) {
    for(int i = 0; i < 8; i++) {
      _HardSerial->write((uint8_t)CONNECT[i]);
    }
    delay(1100);
  }
}

void HeatPump::update() {
  while(!canSend()) { delay(10); }
    byte packet[22] = {};
    createPacket(packet, wantedSettings);
    for (int i = 0; i < 22; i++) {
      _HardSerial->write((uint8_t)packet[i]);
    }
    lastSend = millis();
  
}

void HeatPump::sync() {
  if(canSend()) {
    byte packet[22] = {};
    createInfoPacket(packet);
    for (int i = 0; i < 22; i++) {
       _HardSerial->write((uint8_t)packet[i]);
    }
    lastSend = millis();
  }
  getData();
}

void HeatPump::getSettings(String *settings) {
  settings[0] = getPowerSetting();
  settings[1] = getModeSetting();
  settings[2] = getTemperature();
  settings[3] = getFanSpeed();
  settings[4] = getVaneSetting();
  settings[5] = getDirectionSetting();
}

void HeatPump::setSettings(String settings[]) {
  setPowerSetting(settings[0]);
  setModeSetting(settings[1]);
  setTemperature(settings[2]);
  setFanSpeed(settings[3]);
  setVaneSetting(settings[4]);
  setDirectionSetting(settings[5]);
}

boolean HeatPump::getPowerSettingBool() {
  return currentSettings[0] == POWER_MAP[1] ? true : false;
}

void HeatPump::setPowerSetting(boolean setting) {
  wantedSettings[0] = findValueByString(POWER_MAP, 2, POWER_MAP[setting ? 1 : 0]) > -1 ? POWER_MAP[setting ? 1 : 0] : POWER_MAP[0];
}

String HeatPump::getPowerSetting() {
  return currentSettings[0];
}

void HeatPump::setPowerSetting(String setting) {
  wantedSettings[0] = findValueByString(POWER_MAP, 2, setting) > -1 ? setting : POWER_MAP[0];
}

String HeatPump::getModeSetting() {
  return currentSettings[1];
}

void HeatPump::setModeSetting(String setting) {
  wantedSettings[1] = findValueByString(MODE_MAP, 5, setting) > -1 ? setting : MODE_MAP[0];
}


unsigned int HeatPump::getTemperatureAsInt() {
  return atoi(currentSettings[2].c_str());
}

void HeatPump::setTemperature(unsigned int setting) {
  char* c;
  itoa(setting, c, 10);
  String s = String(c);
  wantedSettings[2] = findValueByString(TEMP_MAP, 16, s) > -1 ? s : TEMP_MAP[0];
}

String HeatPump::getTemperature() {
  return currentSettings[2];
}

void HeatPump::setTemperature(String setting) {
  wantedSettings[2] = findValueByString(TEMP_MAP, 16, setting) > -1 ? setting : TEMP_MAP[0];
}

String HeatPump::getFanSpeed() {
  return currentSettings[3];
}

void HeatPump::setFanSpeed(String setting) {
  wantedSettings[3] = findValueByString(FAN_MAP, 6, setting) > -1 ? setting : FAN_MAP[0];
}

String HeatPump::getVaneSetting() {
  return currentSettings[4];
}

void HeatPump::setVaneSetting(String setting) {
  wantedSettings[4] = findValueByString(VANE_MAP, 6, setting) > -1 ? setting : VANE_MAP[0];
}

String HeatPump::getDirectionSetting() {
  return currentSettings[5];
}

void HeatPump::setDirectionSetting(String setting) {
  wantedSettings[5] = findValueByString(DIR_MAP, 7, setting) > -1 ? setting : DIR_MAP[0];
}

unsigned int HeatPump::getRoomTemperatureAsInt() {
  return atoi(currentSettings[6].c_str());
}

String HeatPump::getRoomTemperature() {
  return currentSettings[6];
}

unsigned int HeatPump::FahrenheitToCelius(unsigned int tempF) {
  double temp = (tempF - 32) / 1.8;                //round up if heat, down if cool or any other mode
  return currentSettings[2] == MODE_MAP[0] ? ceil(temp) : floor(temp);
}

unsigned int HeatPump::CeliusToFahrenheit(unsigned int tempC) {
  double temp = (tempC * 1.8) + 32;                //round up if heat, down if cool or any other mode
  return currentSettings[2] == MODE_MAP[0] ? ceil(temp) : floor(temp);
}

// Private Methods //////////////////////////////////////////////////////////////

int HeatPump::findValueByByte(const byte values[], int len, byte value) {
  for (int i = 0; i < len; i++) {
    if (values[i] == value) {
      return i;
    }
  }
  return -1;
}
int HeatPump::findValueByString(const String values[], int len, String value) {
  for (int i = 0; i < len; i++) {
    if (values[i] == value) {
      return i;
    }
  }
  return -1;
}

boolean HeatPump::canSend() {
  return millis() - 1000 > lastSend;
}  

byte HeatPump::checkSum(byte bytes[], int len) {
  byte sum = 0;
  for (int i = 0; i < len; i++) {
    sum += bytes[i];
  }
  return (0xfc - sum) & 0xff;
}

void HeatPump::createPacket(byte *packet, String settings[]) {
  byte data[21] = {};
  for (int i = 0; i < 8; i++) {
    data[i] = HEADER[i];
  }
  data[8]  = POWER[findValueByString(POWER_MAP, 2, settings[0])];
  data[9]  = MODE[findValueByString(MODE_MAP, 5, settings[1])];
  data[10] = TEMP[findValueByString(TEMP_MAP, 16, settings[2])];
  data[11] = FAN[findValueByString(FAN_MAP, 6, settings[3])];
  data[12] = VANE[findValueByString(VANE_MAP, 7, settings[4])];
  data[13] = 0x00;
  data[14] = 0x00;
  data[15] = DIR[findValueByString(DIR_MAP, 7, settings[5])];
  for (int i = 0; i < 5; i++) {
    data[i + 16] = 0x00;
  }
  byte chkSum = checkSum(data, 21);
  for (int i = 0; i < 21; i++) {
    packet[i] = data[i];
  }
  packet[21] = chkSum;
}

void HeatPump::createInfoPacket(byte *packet) {
  byte data[21] = {};
  for (int i = 0; i < 5; i++) {
    data[i] = INFOHEADER[i];
  }
  data[5]  = INFOMODE[info_mode ? 1 : 0];
  info_mode = !info_mode;
  for (int i = 0; i < 15; i++) {
    data[i + 6] = 0x00;
  }
  byte chkSum = checkSum(data, 21);
  for (int i = 0; i < 21; i++) {
    packet[i] = data[i];
  }
  packet[21] = chkSum;
}

void HeatPump::getData() {
  if(_HardSerial->available()) {
    byte data[22] = {};
    int packetSize = _HardSerial->readBytes(data,22);
    if(packetSize == 22) {
      if(data[0] == 0xfc) {
        byte chkSum = checkSum(data, 21);
        if(data[21] == chkSum) {          
          if(data[5] == INFOMODE[0]) { //Set packet
            currentSettings[0] = POWER_MAP[findValueByByte(POWER, 2, data[8])];
            currentSettings[1] = MODE_MAP[findValueByByte(MODE, 5, data[9])];
            currentSettings[2] = TEMP_MAP[findValueByByte(TEMP, 16, data[10])];
            currentSettings[3] = FAN_MAP[findValueByByte(FAN, 6, data[11])];
            currentSettings[4] = VANE_MAP[findValueByByte(VANE, 7, data[12])];
            currentSettings[5] = DIR_MAP[findValueByByte(DIR, 7, data[15])];
          }
          else if(data[5] == INFOMODE[1]) { //Temp packet
            currentSettings[6] = ROOM_TEMP_MAP[findValueByByte(ROOM_TEMP, 32, data[8])];
          }
        }
      }
    }
  }
}