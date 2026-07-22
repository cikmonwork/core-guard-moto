#include <Arduino.h>
#include <NimBLEDevice.h>
#include <variables_app.h>
#include "functions_app.h"
#include <StringUtils.h>
#include "date_time_app.h"
#include "crypt_app.h"
#include <ArduinoJson.h>
#include <memory_app.h>
#include "memory_module_eer_app.h"

// Константы для парсинга команд в setComand1Chenal
const uint8_t PID_START_POS = 0;
const uint8_t PID_LENGTH = 15;
const uint8_t COMMAND_LENGTH_START_POS = 17;
const uint8_t COMMAND_LENGTH_VAL_LENGTH = 4;
const uint8_t COMMAND_TYPE_VAL_LENGTH = 3;
const uint8_t VALUE_STR_START_POS = 26;

// Глобальные указатели NimBLE
NimBLEServer *pServer = nullptr;
NimBLECharacteristic *pChar1 = nullptr;
NimBLECharacteristic *pChar2 = nullptr;
NimBLECharacteristic *pChar3 = nullptr;
NimBLECharacteristic *pChar4 = nullptr;
NimBLECharacteristic *pChar5 = nullptr;

bool deviceConnected = false;
bool oldDeviceConnected = false;

// Прототипы функций
void setComand1Chenal(String val);
void setComand2Chenal(String val);
void setComand3Chenal(String val);
void setComand4Chenal(String val);
void setComand5Chenal(String val);
void updatePC4();
void updatePC4AndVersion();
void clearBle();

void setPhones(Text val);
void setWiFi(Text val);
void setURLS(Text val);

void setSensors(Text val);
void setReleys(Text val);

// --- ОБРАБОТКА СОБЫТИЙ СЕРВЕРА ---
class MyServerCallbacks : public NimBLEServerCallbacks
{
  void onConnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo) override
  {
    oldDeviceConnected = deviceConnected;
    deviceConnected = true;
    Serial.println("========================================");
    Serial.println(">>> DEVICE CONNECTED <<<");
    Serial.print("Central address: ");
    Serial.println(connInfo.getAddress().toString().c_str());
    Serial.print("Connection ID: ");
    Serial.println(connInfo.getConnHandle());

    pServer->updateConnParams(connInfo.getConnHandle(), 12, 12, 0, 200);

    Serial.println("========================================");
  }
  void onDisconnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo, int reason) override
  {
    oldDeviceConnected = deviceConnected;
    deviceConnected = false;
    Serial.println("========================================");
    Serial.println(">>> DEVICE DISCONNECTED <<<");
    Serial.print("Central address: ");
    Serial.println(connInfo.getAddress().toString().c_str());
    Serial.print("Disconnect reason: ");
    Serial.println(reason);

    Serial.println("========================================");
  }
} serverCallbacks;

// --- ОБРАБОТКА ЗАПИСИ (ТВОЯ ЛОГИКА) ---
class MyCharCallbacks : public NimBLECharacteristicCallbacks
{
  void onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo) override
  {
    Serial.println("=== onWrite CALLBACK TRIGGERED ===");

    String uuid = pCharacteristic->getUUID().toString().c_str();
    Serial.print("Received UUID: ");
    Serial.println(uuid);

    std::string value = pCharacteristic->getValue();
    String val = String((const char *)value.c_str(), value.length());

    Serial.print("Data (string): ");
    Serial.println(val);

    if (uuid == "beb5483e-36e1-4688-b7f5-ea07361b2108")
    { // Канал 1
      Serial.println(">>> bleCharWritten1 - Channel 1");
      if (val.length() < 10 || val.length() > 500)
      {
        Serial.print("Invalid length for channel 1: ");
        Serial.println(val.length());
        pCharacteristic->setValue((uint8_t *)"", 0);
        return;
      }
      setComand1Chenal(val);
    }
    else if (uuid == "cba1d466-344c-4be3-ab3f-189f80dd7218")
    { // Канал 2
      Serial.println(">>> bleCharWritten2 - Channel 2");
      if (val.length() < 10 || val.length() > 500)
      { // Добавлена проверка длины для Канала 2
        Serial.print("Invalid length for channel 2: ");
        Serial.println(val.length());
        pCharacteristic->setValue((uint8_t *)"", 0);
        return;
      }
      Serial.print("Channel 2 data: ");
      Serial.println(val);
      setComand2Chenal(val);
    }
    else if (uuid == "cba1d466-344c-4be3-ab3f-189f80dd7319")
    { // Канал 3
      Serial.println(">>> bleCharWritten3 - Channel 3");
      Serial.print("Channel 3 data: ");
      Serial.println(val);
      // setComand3Chenal(val);
    }
    else if (uuid == "cba1d466-344c-4be3-ab3f-189f80dd7417")
    { // Канал 4
      Serial.println(">>> bleCharWritten4 - Channel 4");
      Serial.print("Channel 4 data: ");
      Serial.println(val);
      // setComand4Chenal не вызывается в оригинале, только логирование
    }
    else if (uuid == "cba1d466-344c-4be3-ab3f-189f80dd7517")
    { // Канал 5
      Serial.println(">>> bleCharWritten5 - Channel 5");
      Serial.print("Channel 5 data: ");
      Serial.println(val);
      // setComand5Chenal не вызывается в оригинале, только логирование
    }
    else
    {
      Serial.print("!!! UNKNOWN UUID: ");
      Serial.println(uuid);
    }
    Serial.println("=== onWrite END ===");
  }
} charCallbacks;

// канал 1  id мобильника, 1002-1000 длинна строки параметров; команда; параметры  4fed8b4d49a396ed;1002,115;1,0
void setComand1Chenal(String val)
{
  Serial.println("setComand1Chenal");
  Serial.println(val);

  // Минимальная длина строки для pId, длины команды и типа команды
  if (val.length() < (PID_LENGTH + COMMAND_LENGTH_VAL_LENGTH + COMMAND_TYPE_VAL_LENGTH + 2))
  { // +2 для разделителей ';'
    Serial.println("Error: Input string too short for setComand1Chenal.");
    pChar1->setValue((uint8_t *)"", 0);
    return;
  }

  String pId = val.substring(PID_START_POS, PID_LENGTH);
  String spcv = decryptBleString(val, pId);
  Serial.println(spcv);

  // Проверка длины spcv после дешифрования
  if (spcv.length() < 7)
  { // +2 для разделителей ';'
    Serial.println("Error: Decrypted string too short for setComand1Chenal.");
    pChar1->setValue((uint8_t *)"", 0);
    return;
  }

  int lengthStr = spcv.substring(0, COMMAND_LENGTH_VAL_LENGTH).toInt() - 1000;
  CommandEnum commandEnum = (CommandEnum)spcv.substring(5, 8).toInt();

  String valueStr3 = (lengthStr > 0) ? spcv.substring(9, lengthStr) : "";
  Text valueStr(valueStr3);

  switch (commandEnum)
  {
  case ISP_ON:
    protectFromSeconds = valueStr.toInt();
    if(protectFromSeconds == 0){
      protectFromSeconds = -1;
      startProtect();
    }
    writeIsProtect(true);
    break;
  case ISP_OFF:
    stopProtect();
    writeIsProtect(false);
    break;
  case SENSOR:
    setSensors(valueStr);
    isSettingsChangedBle = true;
    break;
  case ISS_ON:
    isSleep = 1;
    saveVariablesIsSleap(isSleep);
    isSettingsChangedBle = true;
    break;
  case ISS_OFF:
    isSleep = 0;
    saveVariablesIsSleap(isSleep);
    isSettingsChangedBle = true;
    break;
  case DN:
    deviceSettings.deviceName = valueStr;
    saveVariablesDeviceName(valueStr);
    isSettingsChangedBle = true;
    break;
  case DNBLE:
    deviceNameBle = valueStr;
    saveDeviceNameBle(deviceNameBle);
    isSettingsChangedBle = true;
    break;
  case RELAY:
    setReleys(valueStr);
    isSettingsChangedBle = true;
    break;
  case PHONE:
    setPhones(valueStr);
    isSettingsPhoneChangedBle = true;
    break;
  case URLS:
    setURLS(valueStr);
    break;
  case CT:
    if (valueStr.length() > 5)
    {
      setDateTime(DateTime(valueStr.toInt()));
    }
    break;
  case WIFIPASS:
    setWiFi(valueStr);
    isSettingsWifiChangedBle = true;
    break;
  case FSD:
    if (valueStr.length() > 2)
    {
      Text parseVal[2];
      valueStr.split(parseVal, 2, ";");
      deviceSettings.wifiFrequency = parseVal[0].toInt();
      deviceSettings.isWifiFrequencySending = 1;
      saveFrequencySending(deviceSettings.wifiFrequency);
      saveIsFrequencySending(1);
      isSettingsChangedBle = true;
    }
    break;
  case SatelliteVal:
    break;
  case CLEAR:
    clearBle();
    break;
  default:
    errorInputKey++;
  }

  pChar1->setValue((uint8_t *)"", 0);
}

void setURLS(Text val)
{
  Serial.println("setURLS");
  Serial.println(val);

  Text parseVal[2];
  val.split(parseVal, 2, ";");

  Serial.println(parseVal[0]);
  Serial.println(parseVal[1]);

  beginWrite();

  switch (parseVal[0].toInt())
  {
  case 101:
    urlKeys = parseVal[1];
    saveVariablesUrlKeys(urlKeys);
    break;
  case 102:
    Serial.println("ble urlData:" + parseVal[1]);
    urlData = parseVal[1];
    saveVariablesUrlData(urlData);
    break;
  default:
    break;
  }
  end();
}

void setSensors(Text val)
{
  Text parseVal[8];
  val.split(parseVal, 8, ";");

  beginWrite();
  for (int i = 0; i < 8; i++)
  {
    if (parseVal[i].length() > 3)
    {
      Text parseObj[2];
      parseVal[i].split(parseObj, 2, "/");

      switch (parseObj[0].toInt())
      {
      case 101:
        deviceSettings.sensorShock = parseObj[1].toBool();
        saveVariablesSensorShock(deviceSettings.sensorShock);
        break;
      case 102:
        deviceSettings.sensorInfrared = parseObj[1].toBool();
        saveVariablesSensorInfrared(deviceSettings.sensorInfrared);
        break;
      case 103:
        deviceSettings.sensorReedSwitch = parseObj[1].toBool();
        saveVariablessensorReedSwitch(deviceSettings.sensorReedSwitch);
        break;
      case 104:
        deviceSettings.sensorShockSense = parseObj[1].toInt16();
        saveVariablesSensorShockSense(deviceSettings.sensorShockSense);
        break;
      case 105:
        deviceSettings.gpsFrequency = parseObj[1].toInt16();
        saveGpsFrequency(deviceSettings.gpsFrequency);
        break;
      default:
        break;
      }
    }
  }
  end();
}

void setReleys(Text val)
{
  Text parseVal[8];
  val.split(parseVal, 8, ";");

  beginWrite();
  for (int i = 0; i < 8; i++)
  {
    if (parseVal[i].length() > 3)
    {
      Text parseObj[2];
      parseVal[i].split(parseObj, 2, "/");

      switch (parseObj[0].toInt())
      {
      case 101:
        deviceSettings.relay1 = parseObj[1].toInt16();
        saveRaley(1, deviceSettings.relay1);
        break;
      case 102:
        deviceSettings.relay2 = parseObj[1].toInt16();
        saveRaley(2, deviceSettings.relay2);
        break;
      case 103:
        deviceSettings.relay3 = parseObj[1].toInt16();
        saveRaley(3, deviceSettings.relay3);
        break;
      case 104:
        deviceSettings.relay4 = parseObj[1].toInt16();
        saveRaley(4, deviceSettings.relay4);
        break;
      default:
        break;
      }
    }
  }
  end();
}

void setPhones(Text val)
{
  Text parseVal[8];
  val.split(parseVal, 8, ";");

  for (int i = 0; i < 8; i++)
  {
    if (parseVal[i].length() > 3)
    {
      Text parseObj[2];
      parseVal[i].split(parseObj, 2, "/");

      beginWrite();
      switch (parseObj[0].toInt())
      {
      case 101:
        deviceSettings.phone[0] = parseObj[1].toInt64();
        saveVariablesPhones(1, deviceSettings.phone[0]);
        break;
      case 102:
        deviceSettings.phone[1] = parseObj[1].toInt64();
        saveVariablesPhones(2, deviceSettings.phone[1]);
        break;
      case 103:
        deviceSettings.phone[2] = parseObj[1].toInt64();
        saveVariablesPhones(3, deviceSettings.phone[2]);
        break;
      case 104:
        deviceSettings.phone[3] = parseObj[1].toInt64();
        saveVariablesPhones(4, deviceSettings.phone[3]);
        break;
      case 105:
        deviceSettings.phone[4] = parseObj[1].toInt64();
        saveVariablesPhones(5, deviceSettings.phone[4]);
        break;
      case 106:
        deviceSettings.phone[5] = parseObj[1].toInt64();
        saveVariablesPhones(6, deviceSettings.phone[5]);
        break;
      case 107:
        deviceSettings.phone[6] = parseObj[1].toInt64();
        saveVariablesPhones(7, deviceSettings.phone[6]);
        break;
      case 108:
        deviceSettings.phone[7] = parseObj[1].toInt64();
        saveVariablesPhones(8, deviceSettings.phone[7]);
        break;
      default:
        break;
      }
      end();
    }
  }
}

void setWiFi(Text val)
{
  Text parseVal[15];
  val.split(parseVal, 15, ";");

  beginWrite();
  for (int i = 0; i < 15; i++)
  {

    if (parseVal[i].length() > 3)
    {
      Text parseObj[3];
      parseVal[i].split(parseObj, 3, "/");

      switch (parseObj[0].toInt())
      {
      case 101:
        deviceSettings.wifiSsid[0] = parseObj[1];
        saveVariablesWiFi(1, deviceSettings.wifiSsid[0], parseObj[2]);
        break;
      case 102:
        deviceSettings.wifiSsid[1] = parseObj[1];
        saveVariablesWiFi(2, deviceSettings.wifiSsid[1], parseObj[2]);
        break;
      case 103:
        deviceSettings.wifiSsid[2] = parseObj[1];
        saveVariablesWiFi(3, deviceSettings.wifiSsid[2], parseObj[2]);
        break;
      case 104:
        deviceSettings.wifiSsid[3] = parseObj[1];
        saveVariablesWiFi(4, deviceSettings.wifiSsid[3], parseObj[2]);
        break;
      case 105:
        deviceSettings.wifiSsid[4] = parseObj[1];
        saveVariablesWiFi(5, deviceSettings.wifiSsid[4], parseObj[2]);
        break;
      case 106:
        deviceSettings.wifiSsid[5] = parseObj[1];
        saveVariablesWiFi(6, deviceSettings.wifiSsid[5], parseObj[2]);
        break;
      case 107:
        deviceSettings.wifiSsid[6] = parseObj[1];
        saveVariablesWiFi(7, deviceSettings.wifiSsid[6], parseObj[2]);
        break;
      case 108:
        deviceSettings.wifiSsid[7] = parseObj[1];
        saveVariablesWiFi(8, deviceSettings.wifiSsid[7], parseObj[2]);
        break;
      case 109:
        deviceSettings.wifiSsid[8] = parseObj[1];
        saveVariablesWiFi(9, deviceSettings.wifiSsid[8], parseObj[2]);
        break;
      case 110:
        deviceSettings.wifiSsid[9] = parseObj[1];
        saveVariablesWiFi(10, deviceSettings.wifiSsid[9], parseObj[2]);
        break;
      case 111:
        deviceSettings.wifiSsid[10] = parseObj[1];
        saveVariablesWiFi(11, deviceSettings.wifiSsid[10], parseObj[2]);
        break;
      case 112:
        deviceSettings.wifiSsid[11] = parseObj[1];
        saveVariablesWiFi(12, deviceSettings.wifiSsid[11], parseObj[2]);
        break;
      case 113:
        deviceSettings.wifiSsid[12] = parseObj[1];
        saveVariablesWiFi(13, deviceSettings.wifiSsid[12], parseObj[2]);
        break;
      case 114:
        deviceSettings.wifiSsid[13] = parseObj[1];
        saveVariablesWiFi(14, deviceSettings.wifiSsid[13], parseObj[2]);
        break;
      case 115:
        deviceSettings.wifiSsid[14] = parseObj[1];
        saveVariablesWiFi(15, deviceSettings.wifiSsid[14], parseObj[2]);
        break;
      default:
        break;
      }
    }
  }
  end();
}

void setComand2Chenal(String pcString)
{
  Serial.println("setComand2Chenal 1");

  // Минимальная длина строки для commandStr и pId
  const uint8_t MIN_PCSTRING_LENGTH = 56;

  if (pcString.length() < MIN_PCSTRING_LENGTH)
  {
    Serial.println("Error: Input string too short for setComand2Chenal.");
    pChar2->setValue((uint8_t *)"", 0);
    return;
  }

  String commandStr = pcString.substring(0, 40);
  String pId = pcString.substring(41, 56);

  Serial.println("setComand2Chenal " + pcString);
  Serial.println("setComand2Chenal " + commandStr);

  if (commandStr.equals(blePass))
  {
    Serial.println("setComand2Chenal true");
    String key = addOrUpdateKey(pId);
    String out = deviceSettings.deviceId;
    out += ";";
    out += key;
    out += ";";
    out += aesIv;
    out += ";";
    out += deviceSettings.deviceType;
    out += ";";
    out += deviceSettings.deviceName;

    Serial.println("setComand2Chenal out " + out);
    pChar2->setValue((uint8_t *)out.c_str(), out.length());
    pChar2->notify();
  }
  else
  {
    Serial.println("setComand2Chenal false");
    pChar2->setValue((uint8_t *)"", 0);
  }
  Serial.println("setComand2Chenal end");
}

String getValueComand3(String decryptInput)
{
  String out = "";

  return out;
}

String getValueComand3Param(String decryptInput)
{
  String out = "";
  String command = decryptInput.substring(0, 10);
  String val = decryptInput.substring(10);

  return out;
}

void updatePC4()
{
  if (pChar4 == nullptr)
  {
    Serial.println("Error: pChar4 is NULL! Update skipped.");
    return;
  }

  String out = "";
  
  // 1. Используем переменную версии
  out += versionBleDataP4;

  // 2. Логика статуса сигнализации
  if (isProtect)
  {
    out += "200";
  }
  else if (protectFromSeconds >= 0)
  {
    int displayTime = 100 + (protectFromSeconds > 99 ? 99 : protectFromSeconds);
    out += (String)displayTime;
  }
  else
  {
    out += "100";
  }

  // 3. Добавляем ключ из keyArrNumberArr (вместо статической строки)
  // Преобразуем индексы/байты в символы 0-9, a-f
  for (int i = 0; i < 32; i++) {
    byte val = keyArrNumberArr[i];
    if (val < 10) {
      out += (char)(val + '0');      // 0-9
    } else {
      out += (char)(val - 10 + 'a'); // a-f (10=a, 11=b...)
    }
  }

  // 4. Остальные данные
  out += getBatteryVoltagePercentString();
  out += (String)notifyStore.getSizeSmall() + "0";

  out += (String)getCurrentTime();
  out+= testEsp;

  pChar4->setValue((uint8_t *)out.c_str(), out.length());
  pChar4->notify();
}

void setRandomKeyArrNumber()
{
  for (int i = 0; i < 32; i++)
  {
    keyArrNumberArr[i] = random(0, 32);
  }
}

void updatePC4AndVersion()
{
  setRandomKeyArrNumber();
  updateVersionBleDataP4();
  updatePC4();
}

void clearBle()
{
  pChar1->setValue((uint8_t *)"", 0);
  pChar2->setValue((uint8_t *)"", 0);
  pChar3->setValue((uint8_t *)"", 0);
  pChar5->setValue((uint8_t *)"", 0);
  updatePC4AndVersion();
}

// --- ИНИЦИАЛИЗАЦИЯ И ЗАПУСК ---

void createBLE()
{
  NimBLEDevice::init(deviceNameBle.c_str());
  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(&serverCallbacks);

  NimBLEService *pService = pServer->createService("4fafc201-1fb5-459e-8fcc-c5c9c331914b");

  // Создание характеристик (512 байт MTU поддерживается автоматически)
  Serial.println("Creating characteristics...");
  pChar1 = pService->createCharacteristic("beb5483e-36e1-4688-b7f5-ea07361b2108", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
  Serial.print("Char1 UUID: ");
  Serial.println(pChar1->getUUID().toString().c_str());

  pChar2 = pService->createCharacteristic("cba1d466-344c-4be3-ab3f-189f80dd7218", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
  Serial.print("Char2 UUID: ");
  Serial.println(pChar2->getUUID().toString().c_str());

  pChar3 = pService->createCharacteristic("cba1d466-344c-4be3-ab3f-189f80dd7319", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
  Serial.print("Char3 UUID: ");
  Serial.println(pChar3->getUUID().toString().c_str());

  pChar4 = pService->createCharacteristic("cba1d466-344c-4be3-ab3f-189f80dd7417", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
  Serial.print("Char4 UUID: ");
  Serial.println(pChar4->getUUID().toString().c_str());

  pChar5 = pService->createCharacteristic("cba1d466-344c-4be3-ab3f-189f80dd7517", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
  Serial.print("Char5 UUID: ");
  Serial.println(pChar5->getUUID().toString().c_str());

  // Вешаем коллбэки
  Serial.println("Setting callbacks...");
  pChar1->setCallbacks(&charCallbacks);
  pChar2->setCallbacks(&charCallbacks);
  pChar3->setCallbacks(&charCallbacks);
  pChar4->setCallbacks(&charCallbacks);
  pChar5->setCallbacks(&charCallbacks);
  Serial.println("Callbacks set successfully");

  pService->start();

  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->setName(deviceNameBle.c_str());
  pAdvertising->addServiceUUID(pService->getUUID());
  pAdvertising->start();
  Serial.print("BLE Started (NimBLE) with name: ");
  Serial.println(deviceNameBle);

  NimBLEAddress macAddress = NimBLEDevice::getAddress();
  deviceSettings.mac = String(macAddress.toString().c_str());
}

void stopBLE()
{
  Serial.println("ble stop");
  if (pServer != nullptr)
  {
    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    if (pAdvertising != nullptr)
    {
      pAdvertising->stop();
    }
  }
  NimBLEDevice::deinit(true);
  delay(200);
}

void startBLE()
{
  Serial.println("ble start");
  delay(200);

  NimBLEDevice::init(deviceNameBle.c_str());
  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  NimBLEService *pService = pServer->createService("4fafc201-1fb5-459e-8fcc-c5c9c331914b");

  // Создание характеристик
  pChar1 = pService->createCharacteristic("beb5483e-36e1-4688-b7f5-ea07361b2108", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
  pChar2 = pService->createCharacteristic("cba1d466-344c-4be3-ab3f-189f80dd7218", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
  pChar3 = pService->createCharacteristic("cba1d466-344c-4be3-ab3f-189f80dd7319", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
  pChar4 = pService->createCharacteristic("cba1d466-344c-4be3-ab3f-189f80dd7417", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
  pChar5 = pService->createCharacteristic("cba1d466-344c-4be3-ab3f-189f80dd7517", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);

  // Вешаем коллбэки
  pChar1->setCallbacks(&charCallbacks);
  pChar2->setCallbacks(&charCallbacks);
  pChar3->setCallbacks(&charCallbacks);
  pChar4->setCallbacks(&charCallbacks);
  pChar5->setCallbacks(&charCallbacks);

  pService->start();

  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->setName(deviceNameBle.c_str());
  pAdvertising->addServiceUUID(pService->getUUID());
  pAdvertising->start();
  Serial.print("BLE Restarted with name: ");
  Serial.println(deviceNameBle);
  delay(200);
}