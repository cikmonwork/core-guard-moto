#ifndef MEMORY_APP_H
#define MEMORY_APP_H

#include <Preferences.h>
#include <variables_app.h>

extern Preferences preferences;

void begin();
void beginWrite();
void beginRead();
void end();
void getIsFirstStart();
void getVariables();
void saveFirstVariablesDevice();
void saveFirstVariablesWiFi();
void saveVariablesWiFi(byte ind, String ssid, String pass);
void saveVariablesPhones(byte ind, uint64_t phone);
void saveFirstVariables();
void saveVariablesSensorShock(bool val);
void saveVariablesSensorShockSense(byte val);
void saveVariablessensorReedSwitch(bool val);
void saveVariablesSensorInfrared(bool val);
void saveVariablesUrlKeys(String val);
void saveVariablesUrlData(String val);
void saveGpsFrequency(long val);
void saveVariablesFreqSend(long val);
void saveVariablesIsfreqSend(int val);
void saveVariablesDeviceName(String val);
void saveVariablesIsSleap(bool val);
void saveFalseIsNotGenKA();
void getVariablesDevice();
void getVariablesWifi();
void saveDeviceNameBle(String deviceNameBle);
void saveFrequencySending(long val);

void saveRaley(byte ind, byte val);

void savePidAndKey(String pId, String key, int index);
String getAesKeyMemory(int index);
String getAesPidMemory(int index);
void saveKeyMemory(String pId, String key, int index);
String getWifiPassword(byte index);

#endif