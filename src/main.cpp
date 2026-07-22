#include <Arduino.h>
#include <HardwareSerial.h>
#include <Wire.h>
#include "RTClib.h"
#include <Adafruit_I2CDevice.h>
#include <SPI.h>

#include "ble_manager_app.h"
#include "wifi_manager_app.h"
#include <model/classes_app.h>
#include <variables_app.h>
#include "functions_app.h"
#include "date_time_app.h"
#include "gps_app.h"
#include "sim_manager_app.h"
#include "crypt_app.h"
#include "memory_app.h"
#include "memory_module_eer_app.h"
#include <Wire.h>
#include <WiFi.h>

#define LED 2

hw_timer_t *Hourly_timer = NULL;
volatile int8_t countLoop = 0;
unsigned long lastSecondTick = 0;   // Для отсчета секунд
unsigned long lastBleCheck = 0;     // Для периодической проверки BLE
unsigned long lastSettingsSync = 0; // Для синхронизации настроек с Firebase

void IRAM_ATTR onHourlyTimer()
{
  errorInputPass = 0;
  errorInputKey = 0;
  clearBle();
  setRandomKeyArrNumber();
}

void onMinutesTask(void *parameter)
{
  int gpsCnt = 0;
  unsigned long lastWifiSend = 0;

  for (;;)
  {
    unsigned long now = millis() / 1000; // текущее время в секундах

    // 1. Обратный отсчёт постановки на охрану (каждую 10 сек)
    if (protectFromSeconds > 0)
    {
      protectFromSeconds == protectFromSeconds - 10;
    }
    else if (protectFromSeconds == 0)
    {
      startProtect();
      protectFromSeconds = -1;
    }

    // 2. Экстренный обзвон (приоритет №1)
    if (alarmState == 1)
    {
      alarmPhones();
      alarmState = 2;
    }

    // 3. Сохранение GPS (каждые gpsFrequency минут)
    if (++gpsCnt >= (deviceSettings.gpsFrequency * 6))
    { // gpsFrequency в минутах, 6*10 сек интервал
      saveGPS();
      gpsCnt = 0;
    }

    // 4. Плановая отправка (по frequencySending секунд)
    // frequencySending по умолчанию 60 сек, интервал задачи 10 сек
    if (deviceSettings.isWifiFrequencySending > 0)
    {
      if (now - lastWifiSend >= (unsigned long)deviceSettings.wifiFrequency)
      {
        sendDataWifi();
        lastWifiSend = now;
      }
    }

    // Синхронизация настроек из API каждые 20 секунд
    if (deviceSettings.isWifiFrequencySending > 0)
    {
      if (now - lastSettingsSync >= deviceSettings.wifiFrequency)
      {
        isUpdateSettingsApi();
        lastSettingsSync = now;
      }
    }

    vTaskDelay(pdMS_TO_TICKS(10000)); // 10 секунд
  }
}

void notificationAlarmStart()
{
}

void IRAM_ATTR detectMotion()
{
  if (!motionDetected && isProtect)
  {
    motionDetected = true;
  }
}

void IRAM_ATTR detectGerkon()
{
  if (!gerkonDetected && isProtect)
  {
    gerkonDetected = true;
  }
}

void setup()
{
  Serial.println("Start Setup");
  testEsp = generateRandomString(10);

  pinMode(LED, OUTPUT);
  pinMode(RELE_PIN1, OUTPUT);
  pinMode(RELE_PIN2, OUTPUT);
  pinMode(RELE_PIN3, OUTPUT);

  Serial.begin(115200);
  SerialGPS.begin(9600, SERIAL_8N1, 16, 17); // Настройка для GPS
  delay(100);

  initSim();
  delay(100);

  Wire.begin();
  delay(20);

  getVariables();
  getFirstEEProm();

  delay(20);
  createBLE();
  delay(50);

  setRandomKeyArrNumber();

  // Create a task on Core 2
  xTaskCreatePinnedToCore(onMinutesTask, "MinutesTask", 20000, NULL, 2, NULL, 0);

  // SETUP RTC MODULE
  if (!rtc.begin())
  {
    Serial.println("RTC module is NOT found");
    Serial.flush();
    while (1)
      ;
  }

  pinMode(SENSOR_SHOCK_PIN, INPUT);                                               // Установка пина датчика как вход
  attachInterrupt(digitalPinToInterrupt(SENSOR_SHOCK_PIN), detectMotion, RISING); // Настройка прерывания

  pinMode(GERKON_PIN, INPUT_PULLUP); // Установка пина датчика как вход
  attachInterrupt(digitalPinToInterrupt(GERKON_PIN), detectGerkon, FALLING);

  getKeysFromMemory();
}

// преверяет датчики, ставит на сигнализацию, запускает тревогу
// Мониторинг BLE подключения - перезапуск рекламы при необходимости
void checkBleConnection()
{
  Serial.println("[BLE] checkBleConnection() ");
  // Только если нет активного подключения - периодическая проверка
  if (!deviceConnected)
  {
    unsigned long currentMillis = millis();

    // Проверка каждые 10 секунд если не подключено
    if (currentMillis - lastBleCheck >= 10000)
    {
      lastBleCheck = currentMillis;

      Serial.println("[BLE] Not connected, checking/advertising...");

      NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
      if (pAdvertising)
      {
        // Останавливаем и перезапускаем рекламу для надёжности
        pAdvertising->stop();
        delay(50);
        bool started = pAdvertising->start();
        Serial.printf("[BLE] Advertising restart: %s\n", started ? "OK" : "FAILED");
      }
    }
  }

  // Если ранее было подключение, а сейчас нет - сбрасываем флаг
  if (oldDeviceConnected && !deviceConnected)
  {
    Serial.println("[BLE] Connection was lost");
    oldDeviceConnected = false;
  }
}

void checkAndRunLoopAll()
{
  // 1. МГНОВЕННАЯ РЕАКЦИЯ НА ДАТЧИКИ (приоритет - сразу при срабатывании)
  if (isProtect && (motionDetected || gerkonDetected))
  {
    if (alarmState == 0)
    {
      Serial.println(F("!!! SECURITY BREACH !!!"));
      alarmState = 1; // Запуск звонилки в onMinutesTask

      // Определяем код события: 11-геркон (датчик дверей), 12-вибрация (датчик удара)
      uint32_t eventCode = gerkonDetected ? 11 : 12;

      // Формируем упакованный статус (полный формат)
      // Формат: 1XXXXX0XXX99 где:
      // 1 - префикс (чтобы не было 0 в начале)
      // XXXXX - код события (10=тревога, 11=геркон, 12=вибрация, 31=постановка, 32=снятие)
      // 0/1 - isProtect (стоит ли на охране)
      // XXX - датчики (геркон, инфракрасный, удар) в двоичном виде, напр. 111 = все включены
      // 99 - заряд батареи (0-99)
      uint32_t fullStatus = 100000000;                       // Префикс 1 и нули
      fullStatus += eventCode * 1000000;                     // Код события (11 или 12)
      fullStatus += isProtect * 100000;                      // Охрана (0 или 1)
      fullStatus += deviceSettings.sensorReedSwitch * 10000; // Геркон (0 или 1) * 10000
      fullStatus += deviceSettings.sensorInfrared * 1000;    // Инфракрасный (0 или 1) * 1000
      fullStatus += deviceSettings.sensorShock * 100;        // Удар (0 или 1) * 100
      fullStatus += getBatteryVoltagePercent();              // Заряд батареи (0-99)

      // Добавляем в очередь на отправку
      notifyStore.enqueue(Notification(getCurrentTime(), fullStatus, locStore.getSize()));

      isSendWifi = 1; // Поднимаем флаг для внеплановой отправки по Wi-Fi
    }

    // Сбрасываем флаги прерываний
    motionDetected = false;
    gerkonDetected = false;
    pushSettingsToApi();
  }
}

void loop()
{
  // Мониторинг BLE подключения
  checkBleConnection();
  delay(10);
  checkAndRunLoopAll();
  delay(10);
}
