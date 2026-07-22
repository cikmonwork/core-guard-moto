#include "wifi_manager_app.h"
#include <Arduino.h>
#include <variables_app.h>
#include "functions_app.h"
#include <StringUtils.h>
#include "date_time_app.h"
#include <model/classes_app.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "ble_manager_app.h"
#include <ArduinoJson.h>
#include "crypt_app.h"
#include "memory_app.h"

uint32_t countSendWiFi = 0;

WiFiClientSecure wifiClient;
int numNetworks = 0; // Количество доступных сетей
String urlPB = "/.json?print=silent";

// Функция для получения SSID по индексу
String getWifiSsid(byte index)
{
  if (index >= 1 && index <= 20) {
    return deviceSettings.wifiSsid[index - 1];
  }
  return "";
}

// Функция для подключения к Wi-Fi
bool connectToWiFi()
{
  // Открываем Preferences для чтения
  preferences.begin("appData", true);

  // Сканируем доступные сети
  int n = WiFi.scanNetworks();
  Serial.print("Found ");
  Serial.print(n);
  Serial.println(" networks");

  // Проверяем доступные сети
  for (int i = 0; i < n; i++)
  {
    String ssid = WiFi.SSID(i);

    // Проверяем все сохраненные SSID
    for (byte idx = 1; idx <= 20; idx++)
    {
      String savedSsid = getWifiSsid(idx);

      if (ssid == savedSsid && savedSsid != "")
      {
        // Получаем пароль из памяти
        String password = getWifiPassword(idx);

        if (password != "")
        {
          Serial.print("Connecting to: ");
          Serial.println(ssid);
          WiFi.begin(ssid.c_str(), password.c_str());

          // Ждем подключение с таймаутом
          int timeout = 0;
          while (WiFi.status() != WL_CONNECTED && timeout < 30)
          {
            delay(500);
            Serial.print(".");
            timeout++;
          }

          preferences.end();

          if (WiFi.status() == WL_CONNECTED)
          {
            Serial.println("\nWiFi connected!");
            return true;
          }

          // Если не подключились, снова открываем Preferences
          preferences.begin("appData", true);
        }
      }
    }
  }

  preferences.end();
  return false; // Возвращаем false, если не удалось подключиться
}

// Функция для подключения к конкретной сети по SSID
bool connectToWiFiBySsid(String ssid)
{
  // Открываем Preferences для чтения
  preferences.begin("appData", true);

  // Ищем индекс SSID в сохраненных сетях
  for (byte idx = 1; idx <= 20; idx++)
  {
    if (ssid == getWifiSsid(idx))
    {
      String password = getWifiPassword(idx);

      if (password != "")
      {
        Serial.print("Connecting to: ");
        Serial.println(ssid);
        WiFi.begin(ssid.c_str(), password.c_str());

        // Ждем подключение с таймаутом
        int timeout = 0;
        while (WiFi.status() != WL_CONNECTED && timeout < 30)
        {
          delay(500);
          Serial.print(".");
          timeout++;
        }

        preferences.end();

        if (WiFi.status() == WL_CONNECTED)
        {
          Serial.println("\nWiFi connected!");
          return true;
        }

        // Если не подключились, снова открываем Preferences
        preferences.begin("appData", true);
      }
    }
  }

  preferences.end();
    return false;
}

byte sendSmartPatch(String fullUrl, String jsonBody)
{
  if (WiFi.status() != WL_CONNECTED)
    return 0;

  WiFiClientSecure client;
  client.setInsecure(); // Отключаем проверку сертификатов (как в Postman)

  HTTPClient http;

  // 1. Инициализация (просто вставляем URL)
  if (http.begin(client, fullUrl + urlPB))
  {

    // 2. Указываем тип контента (обязательно для JSON)
    http.addHeader("Content-Type", "application/json");

    // 3. Отправляем PATCH (как в твоем запросе)
    int httpCode = http.PATCH(jsonBody);

    // 4. Логируем ответ
    if (httpCode > 0)
    {
      Serial.print("HTTP Code: ");
      Serial.println(httpCode);

      http.end();
      return 1;
    }
    else
    {
      Serial.print("Error on sending PATCH: ");
      Serial.println(http.errorToString(httpCode).c_str());
      http.end();
      return 0;
    }
  }
  return 0;
}

String sendSmartGet(String fullUrl)
{
  if (WiFi.status() != WL_CONNECTED) return "";

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;

  // Добавляем .json, если его нет в конце URL (стандарт для Firebase)
  if (!fullUrl.endsWith(".json")) {
    fullUrl += "/.json";
  }

  if (http.begin(client, fullUrl)) 
  {
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK)
    {
      String payload = http.getString();
      http.end();
      return payload;
    }
    http.end();
  }
  return "";
}

byte sendGetHttp(String params)
{
  byte outStatus = 0;

  wifiClient.setInsecure();
  if (wifiClient.connect("script.google.com", 443))
  {
    String urlDataStr = "GET ";
    urlDataStr += urlData + params;

    Serial.println("Connection successful");
    wifiClient.println(urlDataStr);
    wifiClient.println(" HTTP/1.1");

    wifiClient.print("Host:");
    wifiClient.println("script.google.com");
    wifiClient.println("Connection: close");
    wifiClient.println();
    urlDataStr.clear();
    outStatus = 1;
  }
  else
  {
  }
  wifiClient.stop();
  return outStatus;
}

byte sendPostHttp(String body, bool isKeys = false)
{
  byte outStatus = 0;

  wifiClient.setInsecure();
  if (wifiClient.connect("script.google.com", 443))
  {
    wifiClient.println("POST " + String(!isKeys ? urlData : urlKeys) + " HTTP/1.1");
    wifiClient.println("Host: script.google.com");
    wifiClient.println("Content-Type: application/json");
    wifiClient.print("Content-Length: ");
    wifiClient.println(body.length());
    wifiClient.println();
    wifiClient.print(body);
    wifiClient.println("Connection: close");
    wifiClient.println();

    outStatus = 1;
  }
  else
  {
  }

  wifiClient.stop();
  return outStatus;
}

void sendDataWifi()
{
  Serial.println(testEsp);
  // if (urlKeys == "" || urlData == "" || isfrequencySending == 0)
  if (urlData == "")
  {
  }
  else
  {
    Serial.println("Wi-Fi start");
    Serial.println("urlData:" + urlData);

    delay(500);

    WiFi.mode(WIFI_STA);

    if (connectToWiFi())
    {
      Serial.println("Connect------------Wi-Fi");
      bool isSendLatency = false;

      isSendLatency = sendNotifications();
      delay(200);

      isSendLatency |= sendLocations();
      delay(200);
      isSendLatency |= sendLocations();
      delay(200);
      isSendLatency |= sendLocations();
      delay(200);

      if (!isSendLatency)
      {
        sendLatencyStatus();
      }
    }

    Serial.println("Wi-Fi stop");
  }
}


/**
 * @brief Отправляет пакет локаций в Firebase одним PATCH запросом.
 * Ключи формируются как [время_отправки + i]_[deviceId], 
 * что делает их уникальными и автоматически сортируемыми.
 */
bool sendLocations()
{
  if (locStore.isEmpty())
    return false;

  Serial.println("Wi-Fi sendLocations: Syncing logs and status...");

  int countToSend = (locStore.getSize() > 50) ? 50 : locStore.getSize();
  JsonDocument doc;

  // Базовое время для формирования уникальных ключей
  long baseTs = getCurrentTime();

  // 1. Формируем логи с уникальными ключами
  for (int i = 0; i < countToSend; i++)
  {
    Location tempLoc;
    if (locStore.peek(i, tempLoc))
    {
      unsigned long long uniqueId = (unsigned long long)baseTs * 100 + i;
      // Используем i для уникальности, даже если отправка происходит в одну секунду
      String logKey = "dCarL/" + String(uniqueId) + "_" + String(deviceSettings.deviceId);
      String logValue = String(tempLoc.timeFrom) + "|" + String(tempLoc.timeTo)+ "|" + tempLoc.getLetLon();
      doc[logKey] = logValue;
    }
  }

  // 2. Обновляем статус устройства
  Location lastLoc;
  if (locStore.peek(countToSend - 1, lastLoc))
  {
    String statusKey = "statusCarN/" + String(deviceSettings.deviceId);
    JsonArray statusArray = doc[statusKey].to<JsonArray>();
    statusArray.add(baseTs);       // Время отправки
    statusArray.add(getCurStatus()); // Текущий статус
    statusArray.add(lastLoc.getLetLon()); // Последняя координата
  }

  // 3. Отправляем всё одним PATCH
  String jsonString;
  serializeJson(doc, jsonString);

  if (sendSmartPatch(urlData, jsonString) == 1)
  {
    locStore.drop(countToSend);
    Serial.println("Success: Locations and status synced.");
    return true;
  }
  else
  {
    Serial.println("Error: Firebase Location sync failed.");
    return false;
  }
}


/**
 * @brief Отправляет пакет уведомлений в Firebase одним PATCH запросом.
 * Ключи формируются на основе текущего времени отправки, что позволяет
 * скрипту забирать только новые данные без использования очередей.
 */
bool sendNotifications()
{
  if (notifyStore.getSize() == 0)
    return false;

  Serial.println("Wi-Fi sendNotifications: Syncing events and status...");

  int countToSend = (notifyStore.getSize() > 50) ? 50 : notifyStore.getSize();
  JsonDocument doc;

  // Базовое время для формирования уникальных ключей
  long baseTs = getCurrentTime();
  String lastLocStr = "0.0,0.0";

  // 1. Формируем события
  for (int i = 0; i < countToSend; i++)
  {
    Notification tempNotif;
    if (notifyStore.peek(i, tempNotif))
    {
       unsigned long long uniqueId = (unsigned long long)baseTs * 100 + i;
      // Используем (baseTs + i) для создания уникального ключа
      String keyN = "dCarN/" + String(uniqueId) + "_" + String(deviceSettings.deviceId);
      
      Location loc;
      // Получаем координаты, соответствующие событию
      String currentLocStr = locStore.peek(tempNotif.curLocation, loc) ? loc.getLetLon() : locStore.getLast().getLetLon();
      
      // Значение: время события | уведомление | координаты
      String valueStr = String(tempNotif.time) + "|" + String(tempNotif.notyfication) + "|" + currentLocStr;
      doc[keyN] = valueStr;

      if (i == countToSend - 1)
      {
        lastLocStr = currentLocStr;
      }
    }
  }

  // 2. Обновляем статус устройства
  String statusKey = "statusCarN/" + String(deviceSettings.deviceId);
  JsonArray statusArray = doc[statusKey].to<JsonArray>();
  statusArray.add(baseTs);
  statusArray.add(getCurStatus());
  statusArray.add(lastLocStr);

  // 3. Сериализация и отправка (никаких очередей и маячков!)
  String jsonString;
  serializeJson(doc, jsonString);

  if (sendSmartPatch(urlData, jsonString) == 1)
  {
    notifyStore.drop(countToSend);
    Serial.println("Success: Notifications and status synced.");
    return true;
  }
  else
  {
    Serial.println("Error: Notification sync failed.");
    return false;
  }
}




bool sendLatencyStatus()
{
  String currentPos = "0.0,0.0";
  int currentSize = locStore.getSize();

  if (currentSize > 0)
  {
    Location lastLoc;
    // peek(currentSize - 1) — заглядываем в самый конец очереди
    if (locStore.peek(currentSize - 1, lastLoc))
    {
      currentPos = lastLoc.getLetLon();
    }
  }

  // ... далее формируешь JSON как раньше ...
  JsonDocument doc;
  String statusKey = "statusCarN/" + String(deviceSettings.deviceId);
  JsonArray statusArray = doc[statusKey].to<JsonArray>();

  statusArray.add(getCurrentTime());
  statusArray.add(getCurStatus());
  statusArray.add(currentPos); // Тут будет либо "0.0,0.0", либо свежий GPS

  String jsonString;
  serializeJson(doc, jsonString);

  return (sendSmartPatch(urlData, jsonString) == 1);
}

void sendKeys()
{
  if (isUpdateKeys)
  {
    Serial.println("Wi-Fi sendKeys");
    JsonDocument doc; // Adjust size as needed

    doc["dId"] = deviceSettings.deviceId;
    doc["type"] = "ks";
    doc["iv"] = aesIv;

    JsonObject elData = doc["data"].to<JsonObject>();

    JsonArray JsonArrPId = elData["pId"].to<JsonArray>();
    JsonArray JsonArrKeys = elData["keys"].to<JsonArray>();

    if (aesRegistry.size() > 0)
    {
      for (int i = 0; i < aesRegistry.size(); i++)
      {
        if (aesRegistry.getKeyAt(i) != "")
        {
          JsonArrPId.add(aesRegistry.getKeyAt(i));
          JsonArrKeys.add(aesRegistry.getKeyAt(i));
        }
        else
        {
          break;
        }
      }
    }

    String jsonString;
    serializeJson(doc, jsonString);

    Serial.println(jsonString);

    sendPostHttp(jsonString, true);
    isUpdateKeys = 0;

    doc.clear();
    jsonString.clear();
  }
}

void performHttpsRequest()
{
  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(10000);

  Serial.println("Connecting to Google...");
  if (client.connect("www.google.com", 443))
  {
    client.print("HEAD / HTTP/1.1\r\nHost: www.google.com\r\nConnection: close\r\n\r\n");
    while (client.connected() || client.available())
    {
      String line = client.readStringUntil('\n');
      if (line.startsWith("Date:"))
      {
        Serial.println("SERVER TIME: " + line.substring(23, 31));
        break;
      }
    }
    client.stop();
  }
  else
  {
    Serial.println("Connection failed");
  }
}


bool syncGeneralSettings()
{
    if (urlData == "") return false;

    String payload = sendSmartGet(urlData + "/deviceSettings/settings/" + deviceSettings.deviceId);
    if (payload == "" || payload == "null") return false;

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (!error)
    {
        deviceSettings.secondsStartSignalization = doc["secondsStartSignalization"] | deviceSettings.secondsStartSignalization;
        deviceSettings.deviceName = doc["deviceName"] | deviceSettings.deviceName;
        deviceSettings.deviceType = doc["deviceType"] | deviceSettings.deviceType;
        deviceSettings.sensorShock = doc["sensorShock"] | deviceSettings.sensorShock;
        deviceSettings.sensorShockSense = doc["sensorShockSense"] | deviceSettings.sensorShockSense;
        deviceSettings.sensorReedSwitch = doc["sensorReedSwitch"] | deviceSettings.sensorReedSwitch;
        deviceSettings.sensorInfrared = doc["sensorInfrared"] | deviceSettings.sensorInfrared;
        deviceSettings.gpsFrequency = doc["gpsFrequency"] | deviceSettings.gpsFrequency;
        deviceSettings.isGps = doc["isGps"] | deviceSettings.isGps;
        deviceSettings.relay1 = doc["relay1"] | deviceSettings.relay1;
        deviceSettings.relay2 = doc["relay2"] | deviceSettings.relay2;
        deviceSettings.relay3 = doc["relay3"] | deviceSettings.relay3;
        deviceSettings.relay4 = doc["relay4"] | deviceSettings.relay4;
        deviceSettings.isWifiFrequencySending = doc["isWifiFrequencySending"] | deviceSettings.isWifiFrequencySending;
        deviceSettings.wifiFrequency = doc["wifiFrequency"] | deviceSettings.wifiFrequency;

        // Сбрасываем флаг 's' в Firebase
        JsonDocument resetDoc;
        resetDoc["deviceSettingsUpdates/" + deviceSettings.deviceId + "/s"] = 0;
        String resetJson;
        serializeJson(resetDoc, resetJson);
        sendSmartPatch(urlData, resetJson);

        return true;
    }
    return false;
}

bool syncPhones()
{
    if (urlData == "") return false;

    String payload = sendSmartGet(urlData + "/deviceSettings/phones/" + deviceSettings.deviceId);
    if (payload == "" || payload == "null") return false;

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (!error)
    {
        for (int i = 0; i < 8; i++) {
            String key = "phone" + String(i + 1);
            deviceSettings.phone[i] = doc[key] | deviceSettings.phone[i];
        }

        // Сбрасываем флаг 'p' в Firebase
        JsonDocument resetDoc;
        resetDoc["deviceSettingsUpdates/" + deviceSettings.deviceId + "/p"] = 0;
        String resetJson;
        serializeJson(resetDoc, resetJson);
        sendSmartPatch(urlData, resetJson);

        return true;
    }
    return false;
}

bool syncWifi()
{
    if (urlData == "") return false;

    String payload = sendSmartGet(urlData + "/deviceSettings/wifi/" + deviceSettings.deviceId);
    if (payload == "" || payload == "null") return false;

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (!error)
    {
        for (int i = 0; i < 20; i++) {
            String sKey = "wifiSsid" + String(i + 1);
            String pKey = "wifiPassword" + String(i + 1);
            deviceSettings.wifiSsid[i] = doc[sKey] | deviceSettings.wifiSsid[i];
            deviceSettings.wifiPassword[i] = doc[pKey] | deviceSettings.wifiPassword[i];
        }

        // Сбрасываем флаг 'w' в Firebase
        JsonDocument resetDoc;
        resetDoc["deviceSettingsUpdates/" + deviceSettings.deviceId + "/w"] = 0;
        String resetJson;
        serializeJson(resetDoc, resetJson);
        sendSmartPatch(urlData, resetJson);

        return true;
    }
    return false;
}

bool syncSettingsFromApi()
{
    bool updated = false;
    updated |= syncGeneralSettings();
    updated |= syncPhones();
    updated |= syncWifi();
    return updated;
}

bool isUpdateSettingsApi()
{
    if (urlData == "") return false;

    String payload = sendSmartGet(urlData + "/deviceSettingsUpdates/" + deviceSettings.deviceId);
    if (payload == "") return false;

    if(payload == "null"){
      pushSettingsToApi();
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (!error)
    {
        bool s = doc["s"] | false;
        bool p = doc["p"] | false;
        bool w = doc["w"] | false;

        if (s) syncGeneralSettings();
        if (p) syncPhones();
        if (w) syncWifi();

        return (s || p || w);
    }
    return false;
}

bool pushSettingsToApi()
{
    if (WiFi.status() != WL_CONNECTED) return false;

    JsonDocument doc; 

    // Создаем корневой объект для всех обновлений, чтобы отправить одним PATCH запросом
    JsonObject root = doc.to<JsonObject>();

    // 1. Данные настроек: /deviceSettings/...
    JsonObject deviceSettingsRoot = root["deviceSettings"].to<JsonObject>();

    // /deviceSettings/settings/{deviceId}
    JsonObject settingsCat = deviceSettingsRoot["settings"].to<JsonObject>();
    JsonObject settings = settingsCat[deviceSettings.deviceId].to<JsonObject>();
    settings["deviceId"] = deviceSettings.deviceId;
    settings["secondsStartSignalization"] = deviceSettings.secondsStartSignalization;
    settings["deviceName"] = deviceSettings.deviceName;
    settings["mac"] = deviceSettings.mac;
    settings["deviceType"] = deviceSettings.deviceType;
    settings["sensorShock"] = deviceSettings.sensorShock;
    settings["sensorShockSense"] = deviceSettings.sensorShockSense;
    settings["sensorReedSwitch"] = deviceSettings.sensorReedSwitch;
    settings["sensorInfrared"] = deviceSettings.sensorInfrared;
    settings["gpsFrequency"] = deviceSettings.gpsFrequency;
    settings["isGps"] = deviceSettings.isGps;
    settings["relay1"] = deviceSettings.relay1;
    settings["relay2"] = deviceSettings.relay2;
    settings["relay3"] = deviceSettings.relay3;
    settings["relay4"] = deviceSettings.relay4;
    settings["isWifiFrequencySending"] = deviceSettings.isWifiFrequencySending;
    settings["wifiFrequency"] = deviceSettings.wifiFrequency;

    // /deviceSettings/phones/{deviceId}
    JsonObject phonesCat = deviceSettingsRoot["phones"].to<JsonObject>();
    JsonObject phones = phonesCat[deviceSettings.deviceId].to<JsonObject>();
    for (int i = 0; i < 8; i++) {
        phones["phone" + String(i + 1)] = deviceSettings.phone[i];
    }

    // /deviceSettings/wifi/{deviceId}
    JsonObject wifiCat = deviceSettingsRoot["wifi"].to<JsonObject>();
    JsonObject wifi = wifiCat[deviceSettings.deviceId].to<JsonObject>();
    for (int i = 0; i < 20; i++) {
        wifi["wifiSsid" + String(i + 1)] = deviceSettings.wifiSsid[i];
        wifi["wifiPassword" + String(i + 1)] = deviceSettings.wifiPassword[i];
    }

    // 2. Сброс флагов обновлений: /deviceSettingsUpdates/{deviceId}
    JsonObject updatesRoot = root["deviceSettingsUpdates"].to<JsonObject>();
    JsonObject updates = updatesRoot[deviceSettings.deviceId].to<JsonObject>();
    updates["s"] = 0;
    updates["p"] = 0;
    updates["w"] = 0;

    String jsonString;
    serializeJson(doc, jsonString);

    // Отправляем всё одним PATCH запросом в корень urlData
    if (sendSmartPatch(urlData, jsonString) == 1)
    {
        isSettingsChangedBle = false;
        isSettingsPhoneChangedBle = false;
        isSettingsWifiChangedBle = false;
        return true;
    }
    return false;
}

bool pushGeneralSettingsToApi()
{
    if (WiFi.status() != WL_CONNECTED) return false;

    JsonDocument doc;
    JsonObject root = doc.to<JsonObject>();

    JsonObject deviceSettingsRoot = root["deviceSettings"].to<JsonObject>();
    JsonObject settingsCat = deviceSettingsRoot["settings"].to<JsonObject>();
    JsonObject settings = settingsCat[deviceSettings.deviceId].to<JsonObject>();
    settings["deviceId"] = deviceSettings.deviceId;
    settings["mac"] = deviceSettings.mac;
    settings["secondsStartSignalization"] = deviceSettings.secondsStartSignalization;
    settings["deviceName"] = deviceSettings.deviceName;
    settings["deviceType"] = deviceSettings.deviceType;
    settings["sensorShock"] = deviceSettings.sensorShock;
    settings["sensorShockSense"] = deviceSettings.sensorShockSense;
    settings["sensorReedSwitch"] = deviceSettings.sensorReedSwitch;
    settings["sensorInfrared"] = deviceSettings.sensorInfrared;
    settings["gpsFrequency"] = deviceSettings.gpsFrequency;
    settings["isGps"] = deviceSettings.isGps;
    settings["relay1"] = deviceSettings.relay1;
    settings["relay2"] = deviceSettings.relay2;
    settings["relay3"] = deviceSettings.relay3;
    settings["relay4"] = deviceSettings.relay4;
    settings["isWifiFrequencySending"] = deviceSettings.isWifiFrequencySending;
    settings["wifiFrequency"] = deviceSettings.wifiFrequency;

    JsonObject updatesRoot = root["deviceSettingsUpdates"].to<JsonObject>();
    JsonObject updates = updatesRoot[deviceSettings.deviceId].to<JsonObject>();
    updates["s"] = 0;

    String jsonString;
    serializeJson(doc, jsonString);
    return (sendSmartPatch(urlData, jsonString) == 1);
}

bool pushPhonesToApi()
{
    if (WiFi.status() != WL_CONNECTED) return false;

    JsonDocument doc;
    JsonObject root = doc.to<JsonObject>();

    JsonObject deviceSettingsRoot = root["deviceSettings"].to<JsonObject>();
    JsonObject phonesCat = deviceSettingsRoot["phones"].to<JsonObject>();
    JsonObject phones = phonesCat[deviceSettings.deviceId].to<JsonObject>();
    for (int i = 0; i < 8; i++) {
        phones["phone" + String(i + 1)] = deviceSettings.phone[i];
    }

    JsonObject updatesRoot = root["deviceSettingsUpdates"].to<JsonObject>();
    JsonObject updates = updatesRoot[deviceSettings.deviceId].to<JsonObject>();
    updates["p"] = 0;

    String jsonString;
    serializeJson(doc, jsonString);
    return (sendSmartPatch(urlData, jsonString) == 1);
}

bool pushWifiToApi()
{
    if (WiFi.status() != WL_CONNECTED) return false;

    JsonDocument doc;
    JsonObject root = doc.to<JsonObject>();

    JsonObject deviceSettingsRoot = root["deviceSettings"].to<JsonObject>();
    JsonObject wifiCat = deviceSettingsRoot["wifi"].to<JsonObject>();
    JsonObject wifi = wifiCat[deviceSettings.deviceId].to<JsonObject>();
    for (int i = 0; i < 20; i++) {
        wifi["wifiSsid" + String(i + 1)] = deviceSettings.wifiSsid[i];
        wifi["wifiPassword" + String(i + 1)] = deviceSettings.wifiPassword[i];
    }

    JsonObject updatesRoot = root["deviceSettingsUpdates"].to<JsonObject>();
    JsonObject updates = updatesRoot[deviceSettings.deviceId].to<JsonObject>();
    updates["w"] = 0;

    String jsonString;
    serializeJson(doc, jsonString);
    return (sendSmartPatch(urlData, jsonString) == 1);
}
