/*
  HeatPump.h - Mitsubishi Heat Pump control library for Arduino
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
#include <stdint.h>
#include <WString.h>
#include <math.h>
#include <HardwareSerial.h>
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

typedef uint8_t byte;

class HeatPump
{
  private:
    const byte CONNECT[8] = {0xfc, 0x5a, 0x01, 0x30, 0x02, 0xca, 0x01, 0xa8};
    const byte HEADER[8]  = {0xfc, 0x41, 0x01, 0x30, 0x10, 0x01, 0x9f, 0x00};
    const byte INFOHEADER[5] = {0xfc, 0x42, 0x01, 0x30, 0x10};
    const byte INFOMODE[2]   = {0x02, 0x03};
    const byte POWER[2]   = {0x00, 0x01};
    const byte MODE[5]    = {0x01,   0x02,  0x03, 0x07, 0x08};
    const byte TEMP[16]   = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
    const byte FAN[6]     = {0x00,  0x01, 0x02, 0x03, 0x05, 0x06};
    const byte VANE[7]    = {0x00,  0x01, 0x02, 0x03, 0x04, 0x05, 0x07};
    const byte DIR[7]     = {0x01, 0x02, 0x03, 0x04, 0x05, 0x08, 0x0c};
    const byte ROOM_TEMP[32] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
                                0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f};
             
    String currentSettings[7];
    String wantedSettings[6];

    HardwareSerial * _HardSerial;
    unsigned int lastSend;
    boolean info_mode;

    int findValueByByte(const byte values[], int len, byte value);
    int findValueByString(const String values[], int len, String value);
    boolean canSend();
    byte checkSum(byte bytes[], int len);
    void createPacket(byte *packet, String settings[]);
    void createInfoPacket(byte *packet);
    void getData();

  public:
    const String POWER_MAP[2] = {"OFF", "ON"};
    const String MODE_MAP[5]  = {"HEAT", "DRY", "COOL", "FAN", "AUTO"};
    const String TEMP_MAP[16] = {"31", "30", "29", "28", "27", "26", "25", "24", "23", "22", "21", "20", "19", "18", "17", "16"};
    const String FAN_MAP[6]   = {"AUTO", "QUIET", "1", "2", "3", "4"};
    const String VANE_MAP[7]  = {"AUTO", "1", "2", "3", "4", "5", "SWING"};
    const String DIR_MAP[7]   = {"<<", "<", "|", ">", ">>", "<>", "SWING"};
    const String ROOM_TEMP_MAP[32] = {"10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23", "24", "25",
                                      "26", "27", "28", "29", "30", "31", "32", "33", "34", "35", "36", "37", "38", "39", "40", "41"};
                          
    HeatPump();
    void connect(HardwareSerial *serial);
    void update();
    void sync();
    void getSettings(String *settings);
    void setSettings(String settings[]);
    void setPowerSetting(boolean setting);
    boolean getPowerSettingBool(); 
    String getPowerSetting();
    void setPowerSetting(String setting);
    String getModeSetting();
    void setModeSetting(String setting);
    unsigned int getTemperatureAsInt();
    void setTemperature(unsigned int setting);
    String getTemperature();
    void setTemperature(String setting);
    String getFanSpeed();
    void setFanSpeed(String setting);
    String getVaneSetting();
    void setVaneSetting(String setting);
    String getDirectionSetting();
    void setDirectionSetting(String setting);
    unsigned int getRoomTemperatureAsInt();
    String getRoomTemperature();
    unsigned int FahrenheitToCelius(unsigned int tempF);
    unsigned int CeliusToFahrenheit(unsigned int tempC);
};