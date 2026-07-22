#ifndef VARIABLES_APP_H
#define VARIABLES_APP_H

#include <Arduino.h>
#include <model/classes_app.h>
#include <model/LocationsStore.h>
#include <map>
#include <string>
#include <model/FlatRegistry.h>
#include <model/DataStore.h>

// pins
#define BATTERY_VOLTAGE_PIN 36

#define SENSOR_SHOCK_PIN 35
#define GERKON_PIN 34

#define RELE_PIN1 12
#define RELE_PIN2 13
#define RELE_PIN3 14
// #define RELE_PIN4 15  // для будущего 4-го реле

// Подключение DS1307
#define SDA_PIN 21
#define SCL_PIN 22

// Каналы

extern String testEsp;

extern char versionBleDataP4;
extern bool isNotGenKeyArr;
extern bool isProtect;
extern volatile bool isSleep;
extern int protectFromSeconds;
extern volatile byte alarmState;
extern byte keysMinuteVisibleTakt;
extern volatile byte workAntenna;
extern volatile bool motionDetected;
extern volatile bool gerkonDetected;
extern volatile bool isSendAlarms;
extern volatile bool isUpdateKeys;

extern bool isClearBLE;
extern String blePass;
extern int countPassArr;
extern byte errorInputPass;
extern byte errorInputKey;

extern byte keyArrNumberArr[32];

extern String urlKeys;
extern String urlData;
extern volatile byte isSendWifi;

extern String deviceNameBle;
#include <model/device_settings_model.h>

extern DeviceSettingModel deviceSettings;
extern volatile bool isSettingsChangedBle;
extern volatile bool isSettingsPhoneChangedBle;
extern volatile bool isSettingsWifiChangedBle;

//gps
// Settings are now managed via deviceSettings struct

extern Notification notificationsAlarmLast;
extern String aesIv;
extern FlatRegistry<String> aesRegistry;
extern LocationsStore locStore;
extern DataStore<Notification> notifyStore; // массив уведомлений



#endif