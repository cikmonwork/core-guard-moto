#ifndef BLE_MANAGER_APP_H
#define BLE_MANAGER_APP_H

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <StringUtils.h>

// Глобальные переменные
extern bool deviceConnected;
extern bool oldDeviceConnected;

// BLE сервер и характеристики (указатели)
extern NimBLEServer* pServer;
extern NimBLECharacteristic* pChar1;
extern NimBLECharacteristic* pChar2;
extern NimBLECharacteristic* pChar3;
extern NimBLECharacteristic* pChar4;
extern NimBLECharacteristic* pChar5;

// Функции для команд
void setComand1Chenal(String val);
void setComand2Chenal(String val);
void setComand3Chenal(String val);
void setComand4Chenal(String val);
void setComand5Chenal(String val);

void setRandomKeyArrNumber();

// Вспомогательные функции (если нужны)
String getValueComand3(String decryptInput);
String getValueComand3Param(String decryptInput);

// Функции для управления BLE
void createBLE();
void stopBLE();
void startBLE();
void updatePC4();
void updatePC4AndVersion();
void clearBle();

void setSensors(Text val);
void setReleys(Text val);

#endif