#include <Arduino.h>
#include <model/classes_app.h>
#include <functions_app.h>
#include <map>
#include <string>
#include <variables_app.h>
#include <HardwareSerial.h>
#include <TinyGPS++.h>
#include "date_time_app.h"
#include "ble_manager_app.h"

HardwareSerial SerialGPS(1);
TinyGPSPlus gps;

// Глобальная переменная для хранения времени предыдущей точки
uint32_t lastPointTime = 0;

void saveGPS() {
    // --- БЛОК ДЛЯ ТЕСТА (Генерируем 2 случайные точки) ---
    uint32_t now = getCurrentTime();
    
    // Создаем тестовую локацию:
   //  timeFrom: now - 10 сек, timeTo: now, координаты: случайные
    Location testLoc(
      now - 10, now, 
      (float)random(48000000, 51000000) / 1000000.0, // Широта в пределах Украины для реалистичности
      (float)random(24000000, 35000000) / 1000000.0  // Долгота
    );

    locStore.enqueue(testLoc); 
    // Serial.print("Test Location Enqueued: ");
    // Serial.println(testLoc.getLetLon());
  



  // 1. Обработка данных с GPS модуля
  while (SerialGPS.available()) {
    gps.encode(SerialGPS.read());
    
    if (gps.location.isUpdated()) {
      uint32_t currentTime = getCurrentTime(); // RTC (Unix Timestamp)
      
      // Если это самая первая точка после включения
      if (lastPointTime == 0) lastPointTime = currentTime - 1; 

      // Создаем объект локации
      // Формат: timeFrom, timeTo, lat, lon
      Location newLoc(
        lastPointTime, 
        currentTime, 
        (float)gps.location.lat(), 
        (float)gps.location.lng()
      );

      // Кладем в наше новое потокобезопасное хранилище
      locStore.enqueue(newLoc);

      // Обновляем время для следующего интервала
      lastPointTime = currentTime;

      Serial.print("GPS Saved: ");
      Serial.println(newLoc.getLetLon());
    }
  }
}