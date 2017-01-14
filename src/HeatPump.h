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
    static const byte CONNECT[];
    static const byte HEADER[];
    static const byte SETTINGS_INFO_PACKET[];
    static const byte ROOMTEMP_INFO_PACKET[];
    
    static const byte PAD[];
    static const byte POWER[];
    static const byte MODE[];
    static const byte TEMP[];
    static const byte FAN[];
    static const byte VANE[];
    static const byte DIR[];
    static const byte ROOM_TEMP[];
    static const byte CONTROL_PACKET_VALUES[];
    static const String CONTROL_PACKET_VALUES_MAP[];
    static const int CONTROL_PACKET_POSITIONS[];
    static const String CONTROL_PACKET_POSITIONS_MAP[];

    static String currentSettings[];
    static String wantedSettings[];

    static void createPacket(byte *packet, String settings[]);
    static int findValueByByte(const byte values[], int len, byte value);
    static int findValueByString(const String values[], int len, String value);
    static byte checkSum(byte bytes[], int len);

    static HardwareSerial * _HardSerial;
    
    static unsigned long lastSendTime;
    static byte infoPacketType;
    static bool lastUpdateSuccessful;

  public:
    static const String POWER_MAP[];
    static const String MODE_MAP[];
    static const String TEMP_MAP[];
    static const String FAN_MAP[];
    static const String VANE_MAP[];
    static const String DIR_MAP[];
    static const String ROOM_TEMP_MAP[];
    HeatPump();
    void connect(HardwareSerial *serial);
    bool update();
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
    int checkForUpdate();
    String findStringValueFromByteValue(const String str_values[], const byte byte_values[], int len, byte value);
    void requestInfoAlternate();
    void requestSettings();
    void requestTemperature();
};