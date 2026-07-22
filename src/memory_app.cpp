#include <Preferences.h>
#include <variables_app.h>
#include "memory_module_eer_app.h"

Preferences preferences;

void saveFirstVariables();
void getVariablesDevice();
void getVariablesWifi();

// Инициализация Preferences
void begin()
{
    preferences.begin("appData", false);
}

void beginWrite()
{
    preferences.begin("appData", false);
    delay(10);
}
void beginRead()
{
    preferences.begin("appData", true);
}
// Завершение работы с Preferences
void end()
{
    preferences.end();
    delay(10);
}

void saveFalseIsNotGenKA()
{
    beginWrite();
    preferences.putBool("isNotGenKA", false);
    end();
}

void getIsFirstStart()
{
    Serial.println("Preferences getIsFirstStart: ");
    isNotGenKeyArr = preferences.getBool("isNotGenKA", true);
}

void getVariables()
{
    Serial.println("Preferences getVariables: ");


    // if (isNotGenKeyArr)
    // {
    //     saveFirstVariables();
    //     setFirstInitEEProm();
    // }
    // else
    // {
    //     getVariablesDevice();
    //     getVariablesWifi();
    // }

    beginRead();
    getIsFirstStart();
    getVariablesDevice();
    getVariablesWifi();
    end();

    delay(100);
}

void saveFirstVariablesDevice()
{
    preferences.putString("deviceName", "Машина супер пупер");
    preferences.putULong64("phone1", 0);
    preferences.putULong64("phone2", 0);
    preferences.putULong64("phone3", 0);
    preferences.putULong64("phone4", 0);
    preferences.putULong64("phone5", 0);
    preferences.putULong64("phone6", 0);
    preferences.putULong64("phone7", 0);
    preferences.putULong64("phone8", 0);

    preferences.putBool("isSleep", false);
    preferences.putInt("sensorSS", 55);
    preferences.putBool("sensorShock", 1);
    preferences.putBool("sensorReedS", false);
    preferences.putBool("sensorInfr", false);
}

void getVariablesDevice()
{
    delay(10);
    deviceSettings.deviceName = preferences.getString("deviceName", "");

    deviceSettings.phone[0] = preferences.getULong64("phone1", 0);
    deviceSettings.phone[1] = preferences.getULong64("phone2", 0);
    deviceSettings.phone[2] = preferences.getULong64("phone3", 0);
    deviceSettings.phone[3] = preferences.getULong64("phone4", 0);
    deviceSettings.phone[4] = preferences.getULong64("phone5", 0);
    deviceSettings.phone[5] = preferences.getULong64("phone6", 0);
    deviceSettings.phone[6] = preferences.getULong64("phone7", 0);
    deviceSettings.phone[7] = preferences.getULong64("phone8", 0);

    isSleep = preferences.getBool("isSleep", false);
    deviceSettings.sensorShockSense = preferences.getInt("sensorSS", 55);
    deviceSettings.sensorShock = preferences.getBool("sensorShock", 1);
    deviceSettings.sensorReedSwitch = preferences.getBool("sensorReedS", false);
    deviceSettings.gpsFrequency = preferences.getInt("gpsFrequency", 3);
    deviceSettings.sensorInfrared = preferences.getBool("sensorInfr", false);
    deviceNameBle = preferences.getString("deviceNameBle", deviceNameBle);

    deviceSettings.relay1 = preferences.getInt("relay1", 0);
    deviceSettings.relay2 = preferences.getInt("relay2", 0);
    deviceSettings.relay3 = preferences.getInt("relay3", 0);
    deviceSettings.relay4 = preferences.getInt("relay4", 0);
}

void savePidAndKey(String pId, String key, int index)
{
    beginWrite();
    delay(10);
    String mid = "aesPId" + (String)index;
    preferences.putString(mid.c_str(), pId);

    String mk = "aesKey" + (String)index;
    preferences.putString(mk.c_str(), key);
    end();
}


void saveKeyMemory(String pId, String key, int index)
{
    beginWrite();
    delay(10);
    String mk = "aesKey" + (String)index;
    preferences.putString(mk.c_str(), key);
    end();
}

void saveDeviceNameBle(String deviceNameBle)
{
    beginWrite();
    delay(10);
    preferences.putString("deviceNameBle", deviceNameBle);
    end();
}

String getAesKeyMemory(int index)
{
    String mk = "aesKey" + (String)index;
    return preferences.getString(mk.c_str(), "");
}

String getAesPidMemory(int index)
{
    String mid = "aesPId" + (String)index;
    return preferences.getString(mid.c_str(), "");
}

void saveVariablesWiFi(byte ind, String ssid, String pass)
{
    String ssidS = "wifiS" + (String)ind;
    String passS = "wifiP" + (String)ind;
    preferences.putString(ssidS.c_str(), ssid);
    preferences.putString(passS.c_str(), pass);
}

void saveFirstVariablesWiFi()
{
    preferences.putLong("freqSend", 60000);
    preferences.putInt("isfreqSend", 1);
    preferences.putString("urlKeys", "");
    preferences.putString("urlData", "");

    preferences.putString("wifiS1", "TP-LINK_4044");
    preferences.putString("wifiP1", "92666429");
    preferences.putString("wifiS2", "");
    preferences.putString("wifiP2", "");
    preferences.putString("wifiS3", "");
    preferences.putString("wifiP3", "");
    preferences.putString("wifiS4", "");
    preferences.putString("wifiP4", "");
    preferences.putString("wifiS5", "");
    preferences.putString("wifiP5", "");
    preferences.putString("wifiS6", "");
    preferences.putString("wifiP6", "");
    preferences.putString("wifiS7", "");
    preferences.putString("wifiP7", "");
    preferences.putString("wifiS8", "");
    preferences.putString("wifiP8", "");
    preferences.putString("wifiS9", "");
    preferences.putString("wifiP9", "");
    preferences.putString("wifiS10", "");
    preferences.putString("wifiP10", "");
    preferences.putString("wifiS11", "");
    preferences.putString("wifiP11", "");
    preferences.putString("wifiS12", "");
    preferences.putString("wifiP12", "");
    preferences.putString("wifiS13", "");
    preferences.putString("wifiP13", "");
    preferences.putString("wifiS14", "");
    preferences.putString("wifiP14", "");
    preferences.putString("wifiS15", "");
    preferences.putString("wifiP15", "");
    preferences.putString("wifiS16", "");
    preferences.putString("wifiP16", "");
    preferences.putString("wifiS17", "");
    preferences.putString("wifiP17", "");
    preferences.putString("wifiS18", "");
    preferences.putString("wifiP18", "");
    preferences.putString("wifiS19", "");
    preferences.putString("wifiP19", "");
    preferences.putString("wifiS20", "");
    preferences.putString("wifiP20", "");
}

void saveFrequencySending(long val){
   beginWrite();
   preferences.putLong("freqSend", val);
   end();
}

void getVariablesWifi()
{
    deviceSettings.wifiFrequency = preferences.getLong("freqSend", 60);
    deviceSettings.isWifiFrequencySending = preferences.getInt("isfreqSend", 1);

    urlKeys = preferences.getString("urlKeys", "");
    urlData = preferences.getString("urlData", "");

    // SSID хранятся в переменных, пароли читаются через getWifiPassword()
    deviceSettings.wifiSsid[0] = preferences.getString("wifiS1", "TP-LINK_4044");
    deviceSettings.wifiSsid[1] = preferences.getString("wifiS2", "");
    deviceSettings.wifiSsid[2] = preferences.getString("wifiS3", "");
    deviceSettings.wifiSsid[3] = preferences.getString("wifiS4", "");
    deviceSettings.wifiSsid[4] = preferences.getString("wifiS5", "");
    deviceSettings.wifiSsid[5] = preferences.getString("wifiS6", "");
    deviceSettings.wifiSsid[6] = preferences.getString("wifiS7", "");
    deviceSettings.wifiSsid[7] = preferences.getString("wifiS8", "");
    deviceSettings.wifiSsid[8] = preferences.getString("wifiS9", "");
    deviceSettings.wifiSsid[9] = preferences.getString("wifiS10", "");
    deviceSettings.wifiSsid[10] = preferences.getString("wifiS11", "");
    deviceSettings.wifiSsid[11] = preferences.getString("wifiS12", "");
    deviceSettings.wifiSsid[12] = preferences.getString("wifiS13", "");
    deviceSettings.wifiSsid[13] = preferences.getString("wifiS14", "");
    deviceSettings.wifiSsid[14] = preferences.getString("wifiS15", "");
    deviceSettings.wifiSsid[15] = preferences.getString("wifiS16", "");
    deviceSettings.wifiSsid[16] = preferences.getString("wifiS17", "");
    deviceSettings.wifiSsid[17] = preferences.getString("wifiS18", "");
    deviceSettings.wifiSsid[18] = preferences.getString("wifiS19", "");
    deviceSettings.wifiSsid[19] = preferences.getString("wifiS20", "");
}

void saveVariablesIsSleap(bool val)
{
    beginWrite();
    preferences.putBool("isSleap", val);
    end();
}

void saveVariablesDeviceName(String val)
{
    beginWrite();
    preferences.putString("deviceName", val);
    end();
}

void saveVariablesUrlKeys(String val)
{
    delay(100);
    preferences.putString("urlKeys", val);
}
void saveVariablesUrlData(String val)
{
    delay(100);
    preferences.putString("urlData", val);
}
void saveVariablesFreqSend(long val)
{
    beginWrite();
    preferences.putLong("freqSend", val);
    end();
}
void saveVariablesIsfreqSend(bool val)
{
    beginWrite();
    preferences.putBool("isfreqSend", val);
    end();
}

void saveVariablesSensorShock(bool val)
{
    preferences.putBool("sensorShock", val);
}
void saveVariablesSensorShockSense(byte val)
{
    preferences.putInt("sensorSS", val);
}

void saveGpsFrequency(long val)
{ 
    preferences.putInt("gpsFrequency", val);
}

void saveRaley(byte ind, byte val)
{
    String ssidS = "relay" + (String)ind;
    preferences.putInt(ssidS.c_str(), val);
}

void saveVariablessensorReedSwitch(bool val)
{
    preferences.putBool("sensorReedS", val);
}
void saveVariablesSensorInfrared(bool val)
{
    preferences.putBool("sensorInfr", val);
}

void saveVariablesWiFi1(byte ind, String ssid, String pass)
{
    String ssidS = "wifiS" + (String)ind;
    String passS = "wifiP" + (String)ind;
    preferences.putString(ssidS.c_str(), ssid);
    preferences.putString(passS.c_str(), pass);
}

void saveVariablesPhones(byte ind, uint64_t phone)
{
    delay(20);
    String phoneK = "phone" + (String)ind;
    preferences.putULong64(phoneK.c_str(), phone);
}

void saveFirstVariables()
{
    Serial.println("Preferences saveVariables: ");
    beginWrite();

    preferences.putBool("isNotGenKA", true);
    saveFirstVariablesWiFi();
    saveFirstVariablesDevice();

    end();
}

// Функция для получения пароля WiFi из памяти по индексу
String getWifiPassword(byte index)
{
    String passKey = "wifiP" + String(index);
    return preferences.getString(passKey.c_str(), "");
}
