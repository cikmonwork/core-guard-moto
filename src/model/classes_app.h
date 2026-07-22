#ifndef CLASSES_APP_H
#define CLASSES_APP_H

#include <Arduino.h>

enum CommandEnum
{
  NOT_FOUND = 0, // комманда не найдена
  CLEAR = 100,   // очистка всех каналов
  ISP_ON = 101,  // вкл сигнализацию через секунд
  ISP_OFF = 102, // выкл сигнализацию

  SS_ON = 103,   // sensor Shock датчик удара
  SS_OFF = 104,  // sensor Shock датчик удара
  SRS_ON = 105,  // гиркон sensor Reed Switch
  SRS_OFF = 106, // гиркон sensor Reed Switch
  SI_ON = 107,   // датчик движения sensor Infrared
  SI_OFF = 108,  // датчик движения sensor Infrared

  ISS_ON = 109,  // isSleep on
  ISS_OFF = 110, // isSleep

  SSS = 111,     // чувствительность датчика удара sensor Shock Sense [0-100]
  PHONE = 112,   // телефон

  DN = 113,          // device name
  CT = 114,          // установка текущего времени 1970 в секундах
  WIFIPASS = 115,    // имя и пароль вай фай
  URLS = 116,        // юрл гугла ключи;дата

  FSD = 118,     // частота отправки данных секунды, и тип отправки, 0 запрет - 1 только вайфай, 2 только сим карта, 3 любой доступный способ приоритет вайфай
  
  SENSOR = 120,      // установка параметров сенсоров и общей работы
  DNBLE = 121,       // device name ble

  RELAY = 122,  // установка реле какие работают
  SatelliteVal = 170, // передача данных от сателитов, ид устройства, тип устройства, имя, данные
  GET_MSG = 301, // получить последнее сообщение
  SEND_MSG = 302, // отправить последнее сообщение
  SET_ENC = 303,  // установить шифрование BLE (1/true/on - включить, 0/false/off - выключить)
};

class Location
{
public:
  uint32_t timeFrom;
  uint32_t timeTo;
  float lat;
  float lon;

  // Конструктор по умолчанию
  Location()
  {
    timeFrom = 0;
    timeTo = 0;
    lat = 0;
    lon = 0;
  };

  // Конструктор с параметрами
  Location(uint32_t timeFrom1, uint32_t timeTo1, float lat1, float lon1)
  {
    timeFrom = timeFrom1;
    timeTo = timeTo1;
    lat = lat1;
    lon = lon1;
  };

  void setValues(uint32_t timeFrom1, uint32_t timeTo1, float lat1, float lon1)
  {
    timeFrom = timeFrom1;
    timeTo = timeTo1;
    lat = lat1;
    lon = lon1;
  };

  // Метод для преобразования объекта в строку
  String toString()
  {
    return "" + String(timeFrom) + "/" + String(timeTo) + "/" + String(lat) + "/" + String(lon) + ";";
  };
  String getLetLon()
  {
    return "" + String(lat, 7) + "," + String(lon, 7);
  };
};

class Notification
{
public:
  uint32_t time;
  // 10-сработала сигнализация без подробностей, 11-сработала сигнализация датчик дверей, 12-вибрации, 13-инфрокрастный, 
  // 21 - сработала защита ввода пароля, 22 - защита вода ключа, 31- установка защиты, 32-снятие с защиты
  // 1 32 0 111 99  1-чтоб небыло 0 в начале, 32-поседнее событие, 0-1-стоит ли сигнализация, 111-включены ли датчики двери, инфро, удара, 99-заряд акб
  uint32_t notyfication;
  uint16_t curLocation;

  Notification()
  {
    time = 0;
    curLocation = 0;
  };
  Notification(uint32_t timev, uint32_t notyficationv, uint16_t curLocationv)
  {
    time = timev;
    notyfication = notyficationv;
    curLocation = curLocationv;
  };
  String toString()
  {
    return "" + String(time) + "/" + String(notyfication) + "/" + String(curLocation);
  };
};

class NotificationAlarm
{
public:
  uint32_t time;
  // 10-сработала сигнализация без подробностей, 11-сработала сигнализация датчик дверей, 12-вибрации, 13-инфрокрастный, 21 - сработала защита ввода пароля, 22 - защита вода ключа, 31- установка защиты, 32-снятие с защиты
  //
  // 1 32 0 111 99  1-чтоб небыло 0 в начале, 32-поседнее событие, 0-1-стоит ли сигнализация, 111-включены ли датчики двери, инфро, удара, 99-заряд акб
  uint32_t notyfication;
  uint32_t lat;
  uint32_t lon;

  String getLetLon()
  {
    return "" + String(lat, 7) + "," + String(lon, 7);
  };

  NotificationAlarm()
  {
    time = 0;
    lat = 0;
    lon = 0;
  };
  NotificationAlarm(uint32_t timev, uint32_t notyficationv, uint32_t latv, uint32_t lonv)
  {
    time = timev;
    notyfication = notyficationv;
    lat = latv;
    lon = lonv;
  };
  String toString()
  {
    return "" + String(time) + "/" + String(notyfication) + "/" + String(lat) + "/" + String(lon);
  };
};

#endif
