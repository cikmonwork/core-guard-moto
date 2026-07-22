#ifndef DEVICE_SETTINGS_MODEL_H
#define DEVICE_SETTINGS_MODEL_H

#include <Arduino.h>

struct DeviceSettingModel {
    String deviceId = "gkjSd3kGds2eAd0"; // device id - 15 символов
    String mac = "";
    int secondsStartSignalization = 30; 
    String deviceName = "";
    int deviceType = 1;  // 1 - мото, 2-дом, 3 сенсор, 4-лора сообщения, 5-счетчик фото
    bool sensorShock = true; // датчик удара
    int sensorShockSense = 0; // чувствительность датчика удара 0 - 100
    bool sensorReedSwitch = false; // гиркон
    bool sensorInfrared = false; // инфрокрастный датчик
    int gpsFrequency = 5; //частота получения и сохранения координат в минутах
    bool isGps = true;
    int relay1 = 0;// 0-отключен, 1-сработает при постановке на сигнализацию, 2-сработает при срабатывание сигнализации, 3-прогрев двигателя
    int relay2 = 0;
    int relay3 = 0;
    int relay4 = 0;
    int isWifiFrequencySending = 1; // нужно ли отправлять 0 запрет - 1 только вайфай, 2 только сим карта, 3 любой доступный способ приоритет вайфай, 4-лора, 5-espNaw
    int wifiFrequency = 180; // частота отправки данных сек
    u_int64_t phone[8] = {0};
    String wifiSsid[20] = {""};
    String wifiPassword[20] = {""};
};

#endif