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

struct heatpumpSettings {
  String power;
  String mode;
  int temperature;
  String fan;
  String vane; //vertical vane, up/down
  String wideVane; //horizontal vane, left/right
  int roomTemperature; //TODO: this isn't a "setting" as such, shouldn't be in struct. Not compared in comparison operators below
};

bool operator==(const heatpumpSettings& lhs, const heatpumpSettings& rhs);
bool operator!=(const heatpumpSettings& lhs, const heatpumpSettings& rhs);

class HeatPump
{
  private:
    static const byte CONNECT[];
    static const int CONNECT_LEN;
    static const byte HEADER[];
    static const int HEADER_LEN;
    static const byte SETTINGS_INFO_PACKET[];
    static const byte ROOMTEMP_INFO_PACKET[];
    
    static const byte PAD[];
    static const byte POWER[];
    static const byte MODE[];
    static const byte TEMP[];
    static const byte FAN[];
    static const byte VANE[];
    static const byte WIDEVANE[];
    static const byte ROOM_TEMP[];
    static const byte CONTROL_PACKET_VALUES[];
    static const String CONTROL_PACKET_VALUES_MAP[];
    static const int CONTROL_PACKET_POSITIONS[];
    static const String CONTROL_PACKET_POSITIONS_MAP[];

    static heatpumpSettings currentSettings;
    static heatpumpSettings wantedSettings;

    static void createPacket(byte *packet, heatpumpSettings settings);
    static int lookupByteMapIndex(const String valuesMap[], int len, String lookupValue);
    static int lookupByteMapIndex(const int valuesMap[], int len, int lookupValue);
    static byte checkSum(byte bytes[], int len);

    static HardwareSerial * _HardSerial;
    
    static unsigned long lastSendTime;
    static byte infoPacketType;
    static bool lastUpdateSuccessful;

  public:
    static const String POWER_MAP[];
    static const String MODE_MAP[];
    static const int TEMP_MAP[];
    static const String FAN_MAP[];
    static const String VANE_MAP[];
    static const String WIDEVANE_MAP[];
    static const int ROOM_TEMP_MAP[];
    HeatPump();
    void connect(HardwareSerial *serial);
    bool update();
    heatpumpSettings getSettings();
    void setSettings(heatpumpSettings settings);
    void setPowerSetting(boolean setting);
    boolean getPowerSettingBool(); 
    String getPowerSetting();
    void setPowerSetting(String setting);
    String getModeSetting();
    void setModeSetting(String setting);
    int getTemperature();
    void setTemperature(int setting);
    String getFanSpeed();
    void setFanSpeed(String setting);
    String getVaneSetting();
    void setVaneSetting(String setting);
    String getWideVaneSetting();
    void setWideVaneSetting(String setting);
    int getRoomTemperature();
    int checkForUpdate();
    String lookupByteMapValue(const String valuesMap[], const byte byteMap[], int len, byte byteValue);
    int lookupByteMapValue(const int valuesMap[], const byte byteMap[], int len, byte byteValue);
    void requestInfoAlternate();
    void requestSettings();
    void requestTemperature();
};
