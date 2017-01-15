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

const byte HeatPump::SETTINGS_INFO_PACKET[]  = {0xfc, 0x42, 0x01, 0x30, 0x10, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7b};
const byte HeatPump::ROOMTEMP_INFO_PACKET[]  = {0xfc, 0x42, 0x01, 0x30, 0x10, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7a};

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
unsigned long HeatPump::lastSendTime;
byte HeatPump::infoPacketType;
bool HeatPump::lastUpdateSuccessful;
// Constructors ////////////////////////////////////////////////////////////////

HeatPump::HeatPump() {}

// Public Methods //////////////////////////////////////////////////////////////

void HeatPump::connect(HardwareSerial *serial) {
  _HardSerial = serial;
  _HardSerial->begin(2400, SERIAL_8E1);
  
  infoPacketType = 0;
  lastUpdateSuccessful = false;
  
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

bool HeatPump::update() {
  byte packet[22] = {};
  createPacket(packet, HeatPump::wantedSettings);
  for (int i = 0; i < 22; i++) {
    _HardSerial->write((uint8_t)packet[i]);
  }
  lastUpdateSuccessful = false;
  delay(1000);
  
  checkForUpdate();
  
  if(lastUpdateSuccessful) {
      for(int i=0;i<6;i++) {
          HeatPump::currentSettings[i] = HeatPump::wantedSettings[i];
      }
  }
  
  return lastUpdateSuccessful;
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
  HeatPump::wantedSettings[4] = findValueByString(HeatPump::VANE_MAP, 7, setting) > -1 ? setting : HeatPump::VANE_MAP[0];
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

String HeatPump::findStringValueFromByteValue(const String str_values[], const byte byte_values[], int len, byte value) {
  for (int i = 0; i < len; i++) {
    if (byte_values[i] == value) {
      return str_values[i];
    }
  }
  return str_values[0];
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

void HeatPump::requestSettings() {
    for (int i = 0; i < 22; i++) {
        _HardSerial->write((uint8_t)SETTINGS_INFO_PACKET[i]);
    }
    delay(100);
}

void HeatPump::requestTemperature() {
    for (int i = 0; i < 22; i++) {
        _HardSerial->write((uint8_t)ROOMTEMP_INFO_PACKET[i]);
    }
    delay(100);
}

/* alternates requesting of settings/temperature information every 2.5 seconds, useful to keep the currentSettings updated */
void HeatPump::requestInfoAlternate() {
    unsigned long currentMillis = millis();
    
    if(currentMillis - lastSendTime > 2500 || ((currentMillis < lastSendTime) && currentMillis > 2500))
    {
        if(infoPacketType == 0) {
            requestSettings();
            
            infoPacketType = 1;
        }
        else if(infoPacketType == 1) {
            requestTemperature();
            
            infoPacketType = 0;
        }
        lastSendTime = millis();
    }
}

int HeatPump::checkForUpdate() {
  byte header[5] = {};
  byte data[32] = {};
  bool found_start = false;
  int data_sum = 0;
  byte checksum = 0;
  byte data_len = 0;
  
  if( _HardSerial->available() > 0)
  {
    // read until we get start byte 0xfc
    while( _HardSerial->available() > 0 && !found_start)
    {
      header[0] =  _HardSerial->read();
      if(header[0] == 0xFC) {
        found_start = true;
        delay(100); // found that this delay increases accuracy when reading, might not be needed though
      }
    }

    if(!found_start)
    {
      return -1;
    }
    
    //read header
    for(int i=1;i<5;i++) {
      header[i] =  _HardSerial->read();
    }
    
    //check header
    if(header[0] == 0xFC && header[2] == 0x01 && header[3] == 0x30)
    {
      data_len = header[4];
      
      for(int i=0;i<data_len;i++) {
        data[i] =  _HardSerial->read();
      }
  
      // read checksum byte
      data[data_len] =  _HardSerial->read();
  
      for (int i = 0; i < 5; i++) {
        data_sum += header[i];
      }
      for (int i = 0; i < data_len; i++) {
        data_sum += data[i];
      }
  
      checksum = (0xfc - data_sum) & 0xff;
      
      if(data[data_len] == checksum) {
        if(data[0] == 0x02 && header[1] == 0x62)  //settings information
        {
            
            currentSettings[0] = findStringValueFromByteValue(HeatPump::POWER_MAP, HeatPump::POWER, 2, data[3]);
            currentSettings[1] = findStringValueFromByteValue(HeatPump::MODE_MAP, HeatPump::MODE, 5, data[4]);
            currentSettings[2] = findStringValueFromByteValue(HeatPump::TEMP_MAP, HeatPump::TEMP, 16, data[5]);
            currentSettings[3] = findStringValueFromByteValue(HeatPump::FAN_MAP, HeatPump::FAN, 6, data[6]);
            currentSettings[4] = findStringValueFromByteValue(HeatPump::VANE_MAP, HeatPump::VANE, 7, data[7]);
            currentSettings[5] = findStringValueFromByteValue(HeatPump::DIR_MAP, HeatPump::DIR, 7, data[10]);
            
            return 1;
          
        } 
        else if(data[0] == 0x03 && header[1] == 0x62) //Room temperature reading         
        {   
          currentSettings[6] = findStringValueFromByteValue(HeatPump::ROOM_TEMP_MAP, HeatPump::ROOM_TEMP, 32, data[3]);
          return 2;
        }
        else if(header[1] == 0x61)
        {
            //Last update was successful
            lastUpdateSuccessful = true;
            return 3;
        }
      }
    }
  }

  header[0] = 0x00;
  
  return -1;
}