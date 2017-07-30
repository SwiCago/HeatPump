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
#ifndef __HeatPump_H__
#define __HeatPump_H__
#include <stdint.h>
#include <math.h>
#include <HardwareSerial.h>
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

/*
 * Callback function definitions. Code differs for the ESP8266 platform, which requires the functional library.
 * Based on callback implementation in the Arduino Client for MQTT library (https://github.com/knolleary/pubsubclient)
 */
#ifdef ESP8266
#include <functional>
#define ON_CONNECT_CALLBACK_SIGNATURE std::function<void()> onConnectCallback
#define SETTINGS_CHANGED_CALLBACK_SIGNATURE std::function<void()> settingsChangedCallback
#define STATUS_CHANGED_CALLBACK_SIGNATURE std::function<void(heatpumpStatus newStatus)> statusChangedCallback
#define PACKET_CALLBACK_SIGNATURE std::function<void(byte* packet, unsigned int length, char* packetDirection)> packetCallback
#define ROOM_TEMP_CHANGED_CALLBACK_SIGNATURE std::function<void(float currentRoomTemperature)> roomTempChangedCallback
#else
#define ON_CONNECT_CALLBACK_SIGNATURE void (*onConnectCallback)()
#define SETTINGS_CHANGED_CALLBACK_SIGNATURE void (*settingsChangedCallback)()
#define STATUS_CHANGED_CALLBACK_SIGNATURE void (*statusChangedCallback)(heatpumpStatus newStatus)
#define PACKET_CALLBACK_SIGNATURE void (*packetCallback)(byte* packet, unsigned int length, char* packetDirection)
#define ROOM_TEMP_CHANGED_CALLBACK_SIGNATURE void (*roomTempChangedCallback)(float currentRoomTemperature)
#endif

typedef uint8_t byte;

struct heatpumpSettings {
  const char* power;
  const char* mode;
  float temperature;
  const char* fan;
  const char* vane; //vertical vane, up/down
  const char* wideVane; //horizontal vane, left/right
  bool iSee;   //iSee sensor, at the moment can only detect it, not set it
};

bool operator==(const heatpumpSettings& lhs, const heatpumpSettings& rhs);
bool operator!=(const heatpumpSettings& lhs, const heatpumpSettings& rhs);

struct heatpumpTimers {
  const char* mode;
  int onMinutesSet;
  int onMinutesRemaining;
  int offMinutesSet;
  int offMinutesRemaining;
};

bool operator==(const heatpumpTimers& lhs, const heatpumpTimers& rhs);
bool operator!=(const heatpumpTimers& lhs, const heatpumpTimers& rhs);

struct heatpumpStatus {
  float roomTemperature;
  bool operating; // if true, the heatpump is operating to reach the desired temperature
  heatpumpTimers timers;
};

class HeatPump
{   
  private:
    static const int PACKET_LEN = 22;
    static const int PACKET_SENT_INTERVAL_MS = 1000;
    static const int PACKET_INFO_INTERVAL_MS = 2000;
    static const int PACKET_TYPE_DEFAULT = 99;

    static const int CONNECT_LEN = 8;
    const byte CONNECT[CONNECT_LEN] = {0xfc, 0x5a, 0x01, 0x30, 0x02, 0xca, 0x01, 0xa8};
    static const int HEADER_LEN  = 8;
    const byte HEADER[HEADER_LEN]  = {0xfc, 0x41, 0x01, 0x30, 0x10, 0x01, 0x00, 0x00};

    const byte INFOHEADER[5]  = {0xfc, 0x42, 0x01, 0x30, 0x10};
    static const int INFOHEADER_LEN  = 5;

    static const int INFOMODE_LEN = 6;
    const byte INFOMODE[INFOMODE_LEN] = {
      0x02, // request a settings packet - RQST_PKT_SETTINGS
      0x03, // request the current room temp - RQST_PKT_ROOM_TEMP
      0x04, // unknown
      0x05, // request the timers - RQST_PKT_TIMERS
      0x06, // request status - RQST_PKT_STATUS
      0x09  // request standby mode (maybe?) RQST_PKT_STANDBY
    };

    const int RCVD_PKT_FAIL            = 0;
    const int RCVD_PKT_CONNECT_SUCCESS = 1;
    const int RCVD_PKT_SETTINGS        = 2;
    const int RCVD_PKT_ROOM_TEMP       = 3;
    const int RCVD_PKT_UPDATE_SUCCESS  = 4;
    const int RCVD_PKT_STATUS          = 5;
    const int RCVD_PKT_TIMER           = 6;

    static const int TIMER_INCREMENT_MINUTES = 10;

    // these settings will be initialised in connect()
    heatpumpSettings currentSettings;
    heatpumpSettings wantedSettings;

    heatpumpStatus currentStatus;

    HardwareSerial * _HardSerial;
    unsigned long lastSend;
    int infoMode;
    unsigned long lastRecv;
    bool connected = false;
    bool autoUpdate;
    bool firstRun;
    bool tempMode;
    bool externalUpdate;

    bool canSend(bool isInfo);
    byte checkSum(byte bytes[], int len);
    void createPacket(byte *packet, heatpumpSettings settings);
    void createInfoPacket(byte *packet, byte packetType);
    int readPacket();
    void writePacket(byte *packet, int length);

    // callbacks
    ON_CONNECT_CALLBACK_SIGNATURE;
    SETTINGS_CHANGED_CALLBACK_SIGNATURE;
    STATUS_CHANGED_CALLBACK_SIGNATURE;
    PACKET_CALLBACK_SIGNATURE;
    ROOM_TEMP_CHANGED_CALLBACK_SIGNATURE;

  public:
    // indexes for INFOMODE array (public so they can be optionally passed to sync())
    static const int RQST_PKT_SETTINGS  = 0;
    static const int RQST_PKT_ROOM_TEMP = 1;
    static const int RQST_PKT_TIMERS    = 3;
    static const int RQST_PKT_STATUS    = 4;
    static const int RQST_PKT_STANDBY   = 5;
    
    static const byte CONTROL_PACKET_1[5];
    static const byte CONTROL_PACKET_2[1];
    
    static const byte POWER[2];
    static const char* POWER_MAP[2];
    static const byte MODE[5];
    static const char* MODE_MAP[5];
    static const byte TEMP[16];
    static const int TEMP_MAP[16];
    static const byte FAN[6];
    static const char* FAN_MAP[6];
    static const byte VANE[7];
    static const char* VANE_MAP[7];
    static const byte WIDEVANE[7];
    static const char* WIDEVANE_MAP[7];
    static const byte ROOM_TEMP[32];
    static const int ROOM_TEMP_MAP[32];
    
    static const byte TIMER_MODE[4];
    static const char* TIMER_MODE_MAP[4];

    static const char* lookupByteMapValue(const char* valuesMap[], const byte byteMap[], int len, byte byteValue);
    static int lookupByteMapValue(const int valuesMap[], const byte byteMap[], int len, byte byteValue);
    static bool lookupByteMapValue(const bool valuesMap[], const byte byteMap[], int len, byte byteValue);
    static int lookupByteMapIndex(const char* valuesMap[], int len, const char* lookupValue);
    static int lookupByteMapIndex(const int valuesMap[], int len, int lookupValue);
    static int lookupByteMapIndex(const bool valuesMap[], int len, bool lookupValue);

    // general
    HeatPump();
    bool connect(HardwareSerial *serial);
    bool update();
    void sync(byte packetType = PACKET_TYPE_DEFAULT);
    void enableExternalUpdate();
    void enableAutoUpdate();
    void disableAutoUpdate();

    // settings
    heatpumpSettings getSettings();
    void setSettings(heatpumpSettings settings);
    void setPowerSetting(bool setting);
    bool getPowerSettingBool();
    const char* getPowerSetting();
    void setPowerSetting(const char* setting);
    const char* getModeSetting();
    void setModeSetting(const char* setting);
    float getTemperature();
    void setTemperature(float setting);
    void setRemoteTemperature(float setting);
    const char* getFanSpeed();
    void setFanSpeed(const char* setting);
    const char* getVaneSetting();
    void setVaneSetting(const char* setting);
    const char* getWideVaneSetting();
    void setWideVaneSetting(const char* setting);
    bool getIseeBool();

    // status
    heatpumpStatus getStatus();
    float getRoomTemperature();
    bool getOperating();

    // helpers
    float FahrenheitToCelsius(int tempF);
    int CelsiusToFahrenheit(float tempC);

    // callbacks
    void setOnConnectCallback(ON_CONNECT_CALLBACK_SIGNATURE);
    void setSettingsChangedCallback(SETTINGS_CHANGED_CALLBACK_SIGNATURE);
    void setStatusChangedCallback(STATUS_CHANGED_CALLBACK_SIGNATURE);
    void setPacketCallback(PACKET_CALLBACK_SIGNATURE);
    void setRoomTempChangedCallback(ROOM_TEMP_CHANGED_CALLBACK_SIGNATURE); // need to deprecate this, is available from setStatusChangedCallback

    // expert users only!
    void sendCustomPacket(byte data[], int len);
};

#endif
