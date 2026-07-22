#include <Arduino.h>
#include <variables_app.h>
#include "date_time_app.h"
#include "ble_manager_app.h"
#include "sim_manager_app.h"
#include <variables_app.h>
#include "memory_module_eer_app.h"

void addNotifications(int8_t event);
Location getLastLocation();
void releON();
void releOff();

char generateRandomCharKey()
{
  const char *charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
  int charsetLength = strlen(charset);
  int randomIndex = random(charsetLength);
  return charset[randomIndex];
}

String generateRandomString(int8_t count)
{
  String out = "";
  out.reserve(count);
  const char *charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
  int charsetLength = strlen(charset);

  for (int i = 0; i < count; i++)
  {
    out += charset[random(charsetLength)];
  }
  return out;
}

String generateRandomString(int length)
{
  String randomString = "";
  const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"; // Символы для генерации строки
  int charsetSize = sizeof(charset) - 1;                                                   // Размер символов (без учета завершающего нуля)

  for (int i = 0; i < length; i++)
  {
    // Генерируем случайный индекс
    int randomIndex = esp_random() % charsetSize;
    randomString += charset[randomIndex]; // Добавляем случайный символ к строке
  }

  return randomString; // Возвращаем сгенерированную строку
}

bool isErrorProt()
{
  if (errorInputPass > 300)
  {
    return true;
  }
  if (isProtect == 1)
  {
    return true;
  }
  return false;
}

byte getBatteryVoltagePercent()
{
  byte value = (int8_t)round((float)analogRead(BATTERY_VOLTAGE_PIN) / 20.0);
  if (value == 100)
    value = 99;
  return value;
}

String getBatteryVoltagePercentString() {
    // Читаем значение (на C3 это 0 - 4095)
    int raw = analogRead(BATTERY_VOLTAGE_PIN);

    // Масштабируем 0-4095 в диапазон 0-99
    // 4095 / 41.37 ≈ 99
    int value = raw / 41; 

    // Жесткий ограничитель, чтобы не выйти за пределы 2 символов
    if (value > 99) value = 99;
    if (value < 0)  value = 0;

    // Форматирование: %02d гарантирует ведущий ноль (0 -> "00", 9 -> "09")
    char buf[3]; 
    snprintf(buf, sizeof(buf), "%02d", value); 
    
    return String(buf);
}

void startProtect()
{
  isProtect = 1;
  addNotifications(31);
  updatePC4AndVersion();
  digitalWrite(2, HIGH);
}

void stopProtect()
{
  isProtect = 0;
  isSendAlarms = false;
  alarmState = 0;
  protectFromSeconds = -1;
  addNotifications(32);
  updatePC4AndVersion();
  digitalWrite(2, LOW);
}


void addNotifications(int8_t event)
{
  uint32_t notif = 100000000;
  notif += event * 1000000;
  notif += isProtect * 100000;
  notif += deviceSettings.sensorReedSwitch * 10000 + deviceSettings.sensorInfrared * 1000 + deviceSettings.sensorShock * 100;
  notif += getBatteryVoltagePercent();

  notifyStore.enqueue(Notification(getCurrentTime(), notif, locStore.getLastIndex()));
}


void addNotificationsAlarm(int8_t event)
{
  uint32_t notif = 100000000;
  notif += event * 1000000;
  notif += isProtect * 100000;
  notif += deviceSettings.sensorReedSwitch * 10000 + deviceSettings.sensorInfrared * 1000 + deviceSettings.sensorShock * 100;
  notif += getBatteryVoltagePercent();

  Notification notificationTmp = Notification(getCurrentTime(), notif, locStore.getLastIndex());

  Location lastLocation =  getLastLocation();

  writeLastAlarm(notificationTmp.notyfication);
  writeLastAlarmDT(notificationTmp.time);
  writeLastAlarmLocationLat(lastLocation.lat);
  writeLastAlarmLocationLon(lastLocation.lon);

  notificationsAlarmLast = notificationTmp;
  notifyStore.enqueue(Notification(getCurrentTime(), notif, locStore.getLastIndex()));
}


String getCurStatus()
{
  String out = "";

  if (alarmState != 1)
  {
    out += notifyStore.getLast().notyfication;
  }
  else
  {
    uint32_t notif = 100000000;
    notif += 10 * 1000000;
    notif += isProtect * 100000;
    notif += deviceSettings.sensorReedSwitch * 10000 + deviceSettings.sensorInfrared * 1000 + deviceSettings.sensorShock * 100;
    notif += getBatteryVoltagePercent();
    out += (String)notif;
  }

  return out;
}


float toRadians(float degrees)
{
  return degrees * (PI / 180.0);
}

float haversine(float lat1, float lon1, float lat2, float lon2)
{
  float R = 6371.0;

  float latDistance = toRadians(lat2 - lat1);
  float lonDistance = toRadians(lon2 - lon1);

  float a = sin(latDistance / 2) * sin(latDistance / 2) + cos(toRadians(lat1)) * cos(toRadians(lat2)) * sin(lonDistance / 2) * sin(lonDistance / 2);
  float c = 2 * atan2(sqrt(a), sqrt(1 - a));

  return R * c * 1000; // Расстояние в метрах
}

void checkSensors()
{
  if (isProtect)
  {
    if (deviceSettings.sensorShock && motionDetected)
    {
      Serial.println("Датчик удара сработал!");
      motionDetected = false;
      if (!isSendAlarms)
      {
        addNotificationsAlarm(12);
        alarmPhones();
        isSendAlarms = true;
      }
    }

    if (deviceSettings.sensorReedSwitch && gerkonDetected)
    {
      Serial.println("Датчик геркона сработал!");
      gerkonDetected = false;
      if (!isSendAlarms)
      {
        addNotificationsAlarm(11);
        alarmPhones();
        isSendAlarms = true;
      }
    }
  }
}

// вкл/выкл мотор для прогрева
void motorWork(bool isWork)
{
  if (isWork)
  {
    digitalWrite(RELE_PIN1, HIGH);
  }
  else
  {
    digitalWrite(RELE_PIN1, LOW);
  }
}

// Управление реле в зависимости от события
// eventType: 1 - постановка на сигнализацию, 2 - сработала сигнализация, 3 - прогрев двигателя
void handleRelayEvent(byte eventType)
{
  // Проверяем каждое реле и его настройку
  if (deviceSettings.relay1 == eventType && deviceSettings.relay1 != 0) {
    digitalWrite(RELE_PIN1, HIGH);
  }
  
  if (deviceSettings.relay2 == eventType && deviceSettings.relay2 != 0) {
    digitalWrite(RELE_PIN2, HIGH);
  }
  
  if (deviceSettings.relay3 == eventType && deviceSettings.relay3 != 0) {
    digitalWrite(RELE_PIN3, HIGH);
  }
  
  // relay4 будет добавлено когда понадобится
  // if (deviceSettings.relay4 == eventType && deviceSettings.relay4 != 0) {
  //   digitalWrite(RELE_PIN4, HIGH);
  // }
}

// Выключение всех реле
void releOff()
{
  digitalWrite(RELE_PIN1, LOW);
  digitalWrite(RELE_PIN2, LOW);
  digitalWrite(RELE_PIN3, LOW);
  // digitalWrite(RELE_PIN4, LOW); // для будущего реле
}

// Включение реле (управление по событиям)
// eventType: 1 - постановка на сигнализацию, 2 - сработала сигнализация, 3 - прогрев двигателя
void releON(byte eventType)
{
  // Сначала выключаем все реле
  releOff();
  
  // Затем включаем те, которые настроены на это событие
  handleRelayEvent(eventType);
}

void updateVersionBleDataP4() {
  if (versionBleDataP4 == '9') {
    versionBleDataP4 = 'a'; // Переход от цифр к маленьким буквам
  } 
  else if (versionBleDataP4 == 'z') {
    versionBleDataP4 = 'A'; // Переход от маленьких к заглавным буквам
  } 
  else if (versionBleDataP4 == 'Z') {
    versionBleDataP4 = '0'; // Сброс в начало цикла
  } 
  else {
    versionBleDataP4++;     // Просто увеличиваем на 1 (внутри текущего диапазона)
  }
}

void printSystemConfig() {
  Serial.println(F("--- START CONFIG DIAGNOSTIC ---"));

  // 1. Системные переменные
  Serial.println(F("[SYSTEM]"));
    Serial.print(F("dt: ")); Serial.println(getNowString());
  Serial.print(F("testEsp: ")); Serial.println(testEsp);
  Serial.print(F("Data Version P4: ")); Serial.println(versionBleDataP4);
  Serial.print(F("Is Protect: ")); Serial.println(isProtect ? "YES" : "NO");
  Serial.print(F("Is Sleep: ")); Serial.println(isSleep ? "YES" : "NO");
  Serial.print(F("Alarm State: ")); Serial.println(alarmState);
  Serial.print(F("Work Antenna: ")); Serial.println(workAntenna); // 0-none, 1-BLE, 2-WiFi
  Serial.print(F("Motion/Gerkon: ")); Serial.print(motionDetected); Serial.print("/"); Serial.println(gerkonDetected);
  Serial.print(F("Raley 1-2: ")); Serial.print(deviceSettings.relay1); Serial.print("/"); Serial.println(deviceSettings.relay2);
  Serial.print(F("Raley 3-4: ")); Serial.print(deviceSettings.relay3); Serial.print("/"); Serial.println(deviceSettings.relay4);


  // 2. BLE Настройки
  Serial.println(F("\n[BLE]"));
  Serial.print(F("Device Name: ")); Serial.println(deviceNameBle);
  Serial.print(F("Pass: ")); Serial.println(blePass);
  Serial.print(F("Errors (Pass/Key): ")); Serial.print(errorInputPass); Serial.print("/"); Serial.println(errorInputKey);

  // 3. WiFi и URL
  Serial.println(F("\n[WIFI & CLOUD]"));
  Serial.print(F("Freq Sending: ")); Serial.print( deviceSettings.wifiFrequency); Serial.println("s");
  Serial.print(F("URL Keys: ")); Serial.println(urlKeys);
  Serial.print(F("URL Data: ")); Serial.println(urlData);
  
  // Вывод списка SSID (циклом для экономии места в коде)
  String ssids[] = {deviceSettings.wifiSsid[0], deviceSettings.wifiSsid[1], deviceSettings.wifiSsid[2], deviceSettings.wifiSsid[3]}; // и так далее...
  Serial.print(F("Main SSID: ")); Serial.println(deviceSettings.wifiSsid[0]);

  // 5. Данные устройства и Телефоны
  Serial.println(F("\n[DEVICE INFO]"));
  Serial.print(F("Full Name: ")); Serial.println(deviceSettings.deviceName);
  Serial.print(F("ID: ")); Serial.println(deviceSettings.deviceId);
  Serial.print(F("Type: ")); Serial.println(deviceSettings.deviceType);
  
  // Вывод uint64_t (нюанс: Serial.print не всегда умеет в 64-бит напрямую)
  Serial.print(F("Phone 1: ")); Serial.println(deviceSettings.phone[0]); 

  // 6. Датчики
  Serial.println(F("\n[SENSORS]"));
  Serial.print(F("Shock: ")); Serial.print(deviceSettings.sensorShock); Serial.print(F(" Sense: ")); Serial.println(deviceSettings.sensorShockSense);
  Serial.print(F("Reed (Gerkon): ")); Serial.println(deviceSettings.sensorReedSwitch);
  Serial.print(F("Infrared: ")); Serial.println(deviceSettings.sensorInfrared);

  Serial.print(F("Gps: ")); Serial.println(deviceSettings.gpsFrequency);


// 2. Хранилища и Реестры (используем getSize())
  Serial.println(F("\n[STORAGE & REGISTRY]"));
  Serial.print(F("Coordinates (locStore): ")); 
  Serial.print(locStore.getSize()); Serial.println(F(" / 500"));
  
  Serial.print(F("AES Registry (aesRegistry): ")); 
  Serial.print(aesRegistry.size()); Serial.println(F(" / 50"));
  
  Serial.print(F("Notifications (notifyStore): ")); 
  Serial.print(notifyStore.getSize()); Serial.println(F(" / 100"));

  Serial.println(F("--- END CONFIG DIAGNOSTIC ---\n"));
}

void printStatus() {
  Serial.println(F("--- START Status ---"));
  Serial.println(F("\n[MEMORY]"));

  Serial.print(F("Свободно: ")); Serial.println(ESP.getFreeHeap());
  Serial.print(F("Минимум: ")); Serial.println(ESP.getMinFreeHeap());

  Serial.print(F("Рандом стр: ")); Serial.println((String)(esp_random() % 100000));

  Serial.println(F("--- END Status ---\n"));
}