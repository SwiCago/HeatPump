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

// Structures //////////////////////////////////////////////////////////////////

bool operator==(const heatpumpSettings& lhs, const heatpumpSettings& rhs) {
  return lhs.power == rhs.power && 
         lhs.mode == rhs.mode && 
         lhs.temperature == rhs.temperature && 
         lhs.fan == rhs.fan &&
         lhs.vane == rhs.vane &&
         lhs.wideVane == rhs.wideVane;
}

bool operator!=(const heatpumpSettings& lhs, const heatpumpSettings& rhs) {
  return lhs.power != rhs.power || 
         lhs.mode != rhs.mode || 
         lhs.temperature != rhs.temperature || 
         lhs.fan != rhs.fan ||
         lhs.vane != rhs.vane ||
         lhs.wideVane != rhs.wideVane;
}

// Initialize Class Variables //////////////////////////////////////////////////

bool HeatPump::lastUpdateSuccessful = false;
unsigned int HeatPump::lastSend = 0;
bool HeatPump::info_mode = false;

// Constructor /////////////////////////////////////////////////////////////////

HeatPump::HeatPump() {
  heatpumpSettings defaultSettings = {POWER_MAP[0],MODE_MAP[3],TEMP_MAP[9],FAN_MAP[2],VANE_MAP[1],WIDEVANE_MAP[2],ROOM_TEMP_MAP[12]};
  currentSettings = defaultSettings;

  // Set wanted settings to current settings. This way if a single setting is
  // changed going forward (e.g. setVaneSetting()), the other settings won't
  // revert to any default wanted settings, but remain as they currently are
  wantedSettings = currentSettings;
}

// Public Methods //////////////////////////////////////////////////////////////

void HeatPump::connect(HardwareSerial *serial) {
  _HardSerial = serial;
  _HardSerial->begin(2400, SERIAL_8E1);
  
  lastUpdateSuccessful = false;
  
  delay(2000);
  for(int cnt = 0; cnt < 2; cnt++) {
    for(int i = 0; i < 8; i++) {
      _HardSerial->write((uint8_t)CONNECT[i]);
    }
    delay(1100);
  }
}

bool HeatPump::update() {
  while(!canSend()) { delay(10); }
  byte packet[22] = {};
  createPacket(packet, wantedSettings);
  for (int i = 0; i < 22; i++) {
    _HardSerial->write((uint8_t)packet[i]);
  }
  lastUpdateSuccessful = false;
  delay(1000);
  
  getData();
  
  if(lastUpdateSuccessful) {
    currentSettings = wantedSettings;
  }
  
  lastSend = millis();

  return lastUpdateSuccessful;
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


heatpumpSettings HeatPump::getSettings() {
  return currentSettings;
}

void HeatPump::setSettings(heatpumpSettings settings) {
  HeatPump::setPowerSetting(settings.power);
  HeatPump::setModeSetting(settings.mode);
  HeatPump::setTemperature(settings.temperature);
  HeatPump::setFanSpeed(settings.fan);
  HeatPump::setVaneSetting(settings.vane);
  HeatPump::setWideVaneSetting(settings.wideVane);
}

bool HeatPump::getPowerSettingBool() {
  return currentSettings.power == POWER_MAP[1] ? true : false;
}

void HeatPump::setPowerSetting(bool setting) {
  wantedSettings.power = lookupByteMapIndex(POWER_MAP, 2, POWER_MAP[setting ? 1 : 0]) > -1 ? POWER_MAP[setting ? 1 : 0] : POWER_MAP[0];
}

String HeatPump::getPowerSetting() {
  return currentSettings.power;
}

void HeatPump::setPowerSetting(String setting) {
  wantedSettings.power = lookupByteMapIndex(POWER_MAP, 2, setting) > -1 ? setting : POWER_MAP[0];
}

String HeatPump::getModeSetting() {
  return currentSettings.mode;
}

void HeatPump::setModeSetting(String setting) {
  wantedSettings.mode = lookupByteMapIndex(MODE_MAP, 5, setting) > -1 ? setting : MODE_MAP[0];
}

int HeatPump::getTemperature() {
  return currentSettings.temperature;
}

void HeatPump::setTemperature(int setting) {
  wantedSettings.temperature = lookupByteMapIndex(TEMP_MAP, 16, setting) > -1 ? setting : TEMP_MAP[0];
}

String HeatPump::getFanSpeed() {
  return currentSettings.fan;
}

void HeatPump::setFanSpeed(String setting) {
  wantedSettings.fan = lookupByteMapIndex(FAN_MAP, 6, setting) > -1 ? setting : FAN_MAP[0];
}

String HeatPump::getVaneSetting() {
  return currentSettings.vane;
}

void HeatPump::setVaneSetting(String setting) {
  wantedSettings.vane = lookupByteMapIndex(VANE_MAP, 7, setting) > -1 ? setting : VANE_MAP[0];
}

String HeatPump::getWideVaneSetting() {
  return currentSettings.wideVane;
}

void HeatPump::setWideVaneSetting(String setting) {
  wantedSettings.wideVane = lookupByteMapIndex(WIDEVANE_MAP, 7, setting) > -1 ? setting : WIDEVANE_MAP[0];
}

int HeatPump::getRoomTemperature() {
  return currentSettings.roomTemperature;
}

unsigned int HeatPump::FahrenheitToCelsius(unsigned int tempF) {
  double temp = (tempF - 32) / 1.8;                //round up if heat, down if cool or any other mode
  return currentSettings.mode == MODE_MAP[0] ? ceil(temp) : floor(temp);
}

unsigned int HeatPump::CelsiusToFahrenheit(unsigned int tempC) {
  double temp = (tempC * 1.8) + 32;                //round up if heat, down if cool or any other mode
  return currentSettings.mode == MODE_MAP[0] ? ceil(temp) : floor(temp);
}

void HeatPump::setSettingsChangedCallback(SETTINGS_CHANGED_CALLBACK_SIGNATURE) {
  this->settingsChangedCallback = settingsChangedCallback;
}

void HeatPump::setPacketReceivedCallback(PACKET_RECEIVED_CALLBACK_SIGNATURE) {
  this->packetReceivedCallback = packetReceivedCallback;
}

// Private Methods //////////////////////////////////////////////////////////////

int HeatPump::lookupByteMapIndex(const int valuesMap[], int len, int lookupValue) {
  for (int i = 0; i < len; i++) {
    if (valuesMap[i] == lookupValue) {
      return i;
    }
  }
  return -1;
}

int HeatPump::lookupByteMapIndex(const String valuesMap[], int len, String lookupValue) {
  for (int i = 0; i < len; i++) {
    if (valuesMap[i] == lookupValue) {
      return i;
    }
  }
  return -1;
}


String HeatPump::lookupByteMapValue(const String valuesMap[], const byte byteMap[], int len, byte byteValue) {
  for (int i = 0; i < len; i++) {
    if (byteMap[i] == byteValue) {
      return valuesMap[i];
    }
  }
  return valuesMap[0];
}

int HeatPump::lookupByteMapValue(const int valuesMap[], const byte byteMap[], int len, byte byteValue) {
  for (int i = 0; i < len; i++) {
    if (byteMap[i] == byteValue) {
      return valuesMap[i];
    }
  }
  return valuesMap[0];
}

bool HeatPump::canSend() {
  return millis() - 1000 > lastSend;
}  

byte HeatPump::checkSum(byte bytes[], int len) {
  byte sum = 0;
  for (int i = 0; i < len; i++) {
    sum += bytes[i];
  }
  return (0xfc - sum) & 0xff;
}

void HeatPump::createPacket(byte *packet, heatpumpSettings settings) {
  byte data[21] = {};
  for (int i = 0; i < HEADER_LEN; i++) {
    data[i] = HEADER[i];
  }
  data[8]  = POWER[lookupByteMapIndex(POWER_MAP, 2, settings.power)];
  data[9]  = MODE[lookupByteMapIndex(MODE_MAP, 5, settings.mode)];
  data[10] = TEMP[lookupByteMapIndex(TEMP_MAP, 16, settings.temperature)];
  data[11] = FAN[lookupByteMapIndex(FAN_MAP, 6, settings.fan)];
  data[12] = VANE[lookupByteMapIndex(VANE_MAP, 7, settings.vane)];
  data[13] = 0x00;
  data[14] = 0x00;
  data[15] = WIDEVANE[lookupByteMapIndex(WIDEVANE_MAP, 7, settings.wideVane)];
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

int HeatPump::getData() {
  byte header[5] = {};
  byte data[32] = {};
  bool found_start = false;
  int data_sum = 0;
  byte checksum = 0;
  byte data_len = 0;
  
  if( _HardSerial->available() > 0)
  {
    // read until we get start byte 0xfc
    while(_HardSerial->available() > 0 && !found_start)
    {
      header[0] = _HardSerial->read();
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
        data[i] = _HardSerial->read();
      }
  
      // read checksum byte
      data[data_len] = _HardSerial->read();
  
      for (int i = 0; i < 5; i++) {
        data_sum += header[i];
      }
      for (int i = 0; i < data_len; i++) {
        data_sum += data[i];
      }
  
      checksum = (0xfc - data_sum) & 0xff;
      
      if(data[data_len] == checksum) {
        if(packetReceivedCallback) {
          packetReceivedCallback(data, data_len);
        }

        if(header[1] == 0x62 && data[0] == 0x02)  //settings information
        {
          heatpumpSettings receivedSettings;
          receivedSettings.power       = lookupByteMapValue(POWER_MAP, POWER, 2, data[3]);
          receivedSettings.mode        = lookupByteMapValue(MODE_MAP, MODE, 5, data[4]);
          receivedSettings.temperature = lookupByteMapValue(TEMP_MAP, TEMP, 16, data[5]);
          receivedSettings.fan         = lookupByteMapValue(FAN_MAP, FAN, 6, data[6]);
          receivedSettings.vane        = lookupByteMapValue(VANE_MAP, VANE, 7, data[7]);
          receivedSettings.wideVane    = lookupByteMapValue(WIDEVANE_MAP, WIDEVANE, 7, data[10]);

          if(settingsChangedCallback && receivedSettings != currentSettings) {
            currentSettings = receivedSettings;
            settingsChangedCallback();
          } else {
            currentSettings = receivedSettings;
          }

          return 1;
        } 
        else if(header[1] == 0x62 && data[0] == 0x03) //Room temperature reading         
        {   
          currentSettings.roomTemperature = lookupByteMapValue(ROOM_TEMP_MAP, ROOM_TEMP, 32, data[3]);
          return 2;
        }
        else if(header[1] == 0x61) //Last update was successful
        {
          lastUpdateSuccessful = true;
          return 3;
        }
      }
    }
  }

  header[0] = 0x00;
  
  return -1;
}

