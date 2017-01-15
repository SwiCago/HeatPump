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

const byte HeatPump::CONNECT[] = {0xfc, 0x5a, 0x01, 0x30, 0x02, 0xca, 0x01, 0xa8};
const byte HeatPump::HEADER[]  = {0xfc, 0x41, 0x01, 0x30, 0x10, 0x01, 0x9f, 0x00};
const byte HeatPump::PAD[]     = {0x00, 0x00, 0x00, 0x00, 0x00};
const byte HeatPump::POWER[]       = {0x00, 0x01};
const String HeatPump::POWER_MAP[] = {"OFF", "ON"};
const byte HeatPump::MODE[]       = {0x01,   0x02,  0x03, 0x07, 0x08};
const String HeatPump::MODE_MAP[] = {"HEAT", "DRY", "COOL", "FAN", "AUTO"};
const byte HeatPump::TEMP[]       = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
const String HeatPump::TEMP_MAP[] = {"31", "30", "29", "28", "27", "26", "25", "24", "23", "22", "21", "20", "19", "18", "17", "16"};
const byte HeatPump::FAN[]       = {0x00,  0x01,   0x02, 0x03, 0x05, 0x06};
const String HeatPump::FAN_MAP[] = {"AUTO", "QUIET", "1", "2", "3", "4"};
const byte HeatPump::VANE[]       = {0x00,  0x01, 0x02, 0x03, 0x04, 0x05, 0x07};
const String HeatPump::VANE_MAP[] = {"AUTO", "1", "2", "3", "4", "5", "SWING"};
const byte HeatPump::DIR[]        = {0x01, 0x02, 0x03, 0x04, 0x05, 0x08, 0x0c};
const String HeatPump::DIR_MAP[] = {"<<", "<", "|", ">", ">>", "<>", "SWING"};
const byte HeatPump::ROOM_TEMP[]       = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
                                          0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f};
const String HeatPump::ROOM_TEMP_MAP[] = {"10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23", "24", "25",
                                          "26", "27", "28", "29", "30", "31", "32", "33", "34", "35", "36", "37", "38", "39", "40", "41"};
                                          
const byte HeatPump::CONTROL_PACKET_VALUES[]       = {0x01,   0x02,  0x04,  0x08, 0x10,  0x80};
const String HeatPump::CONTROL_PACKET_VALUES_MAP[] = {"POWER", "MODE", "TEMP", "FAN", "VANE", "DIR"};
const int HeatPump::CONTROL_PACKET_POSITIONS[]        = {3,      4,     5,     6,    7,     10};
const String HeatPump::CONTROL_PACKET_POSITIONS_MAP[] = {"POWER", "MODE", "TEMP", "FAN", "VANE", "DIR"};

String HeatPump::currentSettings[] = {POWER_MAP[0], MODE_MAP[0], TEMP_MAP[0], FAN_MAP[0], VANE_MAP[0], DIR_MAP[0], ROOM_TEMP_MAP[0]};
String HeatPump::wantedSettings[]  = {POWER_MAP[0], MODE_MAP[0], TEMP_MAP[0], FAN_MAP[0], VANE_MAP[0], DIR_MAP[0]};

HardwareSerial * HeatPump::_HardSerial;

// Constructors ////////////////////////////////////////////////////////////////

HeatPump::HeatPump() {}

// Public Methods //////////////////////////////////////////////////////////////

void HeatPump::connect(HardwareSerial *serial) {
  _HardSerial = serial;
  _HardSerial->begin(2400, SERIAL_8E1);
  delay(2000);
  for (int i = 0; i < 8; i++) {
    _HardSerial->write((uint8_t)CONNECT[i]);
  }
  delay(1100);
  for (int i = 0; i < 8; i++) {
    _HardSerial->write((uint8_t)CONNECT[i]);
  }
  delay(2000);
}

void HeatPump::update() {
  byte packet[22] = {};
  createPacket(packet, HeatPump::wantedSettings);
  for (int i = 0; i < 22; i++) {
    _HardSerial->write((uint8_t)packet[i]);
  }
  delay(1000);
}

void HeatPump::getSettings(String *settings) {
  settings = HeatPump::currentSettings;
}

void HeatPump::setSettings(String settings[]) {
  HeatPump::setPowerSetting(settings[0]);
  HeatPump::setModeSetting(settings[1]);
  HeatPump::setTemperature(settings[2]);
  HeatPump::setFanSpeed(settings[3]);
  HeatPump::setVaneSetting(settings[4]);
  HeatPump::setDirectionSetting(settings[5]);
}

boolean HeatPump::getPowerSettingBool() {
  return HeatPump::currentSettings[0] == HeatPump::POWER_MAP[1] ? true : false;
}
void HeatPump::setPowerSetting(boolean setting) {
  HeatPump::wantedSettings[0] = findValueByString(HeatPump::POWER_MAP, 2, HeatPump::POWER_MAP[setting ? 1 : 0]) > -1 ? HeatPump::POWER_MAP[setting ? 1 : 0] : HeatPump::POWER_MAP[0];
}

String HeatPump::getPowerSetting() {
  return HeatPump::currentSettings[0];
}

void HeatPump::setPowerSetting(String setting) {
  HeatPump::wantedSettings[0] = findValueByString(HeatPump::POWER_MAP, 2, setting) > -1 ? setting : HeatPump::POWER_MAP[0];
}

String HeatPump::getModeSetting() {
  return HeatPump::currentSettings[1];
}

void HeatPump::setModeSetting(String setting) {
  HeatPump::wantedSettings[1] = findValueByString(HeatPump::MODE_MAP, 5, setting) > -1 ? setting : HeatPump::MODE_MAP[0];
}


unsigned int HeatPump::getTemperatureAsInt() {
  return atoi(HeatPump::currentSettings[2].c_str());
}

void HeatPump::setTemperature(unsigned int setting) {
  char* c;
  itoa(setting, c, 10);
  String s = String(c);
  HeatPump::wantedSettings[2] = findValueByString(HeatPump::TEMP_MAP, 16, s) > -1 ? s : HeatPump::TEMP_MAP[0];
}


String HeatPump::getTemperature() {
  return HeatPump::currentSettings[2];
}

void HeatPump::setTemperature(String setting) {
  HeatPump::wantedSettings[2] = findValueByString(HeatPump::TEMP_MAP, 16, setting) > -1 ? setting : HeatPump::TEMP_MAP[0];
}

String HeatPump::getFanSpeed() {
  return HeatPump::currentSettings[3];
}

void HeatPump::setFanSpeed(String setting) {
  HeatPump::wantedSettings[3] = findValueByString(HeatPump::FAN_MAP, 6, setting) > -1 ? setting : HeatPump::FAN_MAP[0];
}

String HeatPump::getVaneSetting() {
  return HeatPump::currentSettings[4];
}

void HeatPump::setVaneSetting(String setting) {
  HeatPump::wantedSettings[4] = findValueByString(HeatPump::VANE_MAP, 6, setting) > -1 ? setting : HeatPump::VANE_MAP[0];
}

String HeatPump::getDirectionSetting() {
  return HeatPump::currentSettings[5];
}

void HeatPump::setDirectionSetting(String setting) {
  HeatPump::wantedSettings[5] = findValueByString(HeatPump::DIR_MAP, 7, setting) > -1 ? setting : HeatPump::DIR_MAP[0];
}

unsigned int HeatPump::getRoomTemperatureAsInt() {
  return atoi(HeatPump::currentSettings[6].c_str());
}

String HeatPump::getRoomTemperature() {
  return HeatPump::currentSettings[6];
}

unsigned int HeatPump::FahrenheitToCelius(unsigned int tempF) {
  double temp = (tempF - 32) / 1.8;                //round up if heat, down if cool or any other mode
  return HeatPump::currentSettings[2] == HeatPump::MODE_MAP[0] ? ceil(temp) : floor(temp);
}

unsigned int HeatPump::CeliusToFahrenheit(unsigned int tempC) {
  double temp = (tempC * 1.8) + 32;                //round up if heat, down if cool or any other mode
  return HeatPump::currentSettings[2] == HeatPump::MODE_MAP[0] ? ceil(temp) : floor(temp);
}

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
  data[8]  = HeatPump::POWER[findValueByString(HeatPump::POWER_MAP, 2, settings[0])];
  data[9]  = HeatPump::MODE[findValueByString(HeatPump::MODE_MAP, 5, settings[1])];
  data[10] = HeatPump::TEMP[findValueByString(HeatPump::TEMP_MAP, 16, settings[2])];
  data[11] = HeatPump::FAN[findValueByString(HeatPump::FAN_MAP, 6, settings[3])];
  data[12] = HeatPump::VANE[findValueByString(HeatPump::VANE_MAP, 7, settings[4])];
  data[13] = 0x00;
  data[14] = 0x00;
  data[15] = HeatPump::DIR[findValueByString(HeatPump::DIR_MAP, 7, settings[5])];
  for (int i = 0; i < 5; i++) {
    data[i + 16] = PAD[i];
  }
  byte chkSum = checkSum(data, 21);
  for (int i = 0; i < 21; i++) {
    packet[i] = data[i];
  }
  packet[21] = chkSum;
}