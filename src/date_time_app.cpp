#include "RTClib.h"
#include "date_time_app.h"

RTC_DS1307 rtc;

void setDateTime(DateTime dt){
     // dt = dt - TimeSpan(0, 3, 0, 0); // вычитаем 3 часа
      rtc.adjust(dt);
}

String getNowString(){
  DateTime now = rtc.now();
  String dateTimeNow = "Current DateTime: ";
  dateTimeNow += (String)now.year() + "." + (String)now.month() + "." + (String)now.day() + " " + now.hour() + ":" + (String)now.minute() + ":" + (String)now.second();

  return dateTimeNow;
};

uint32_t getCurrentTime()
{
  DateTime now = rtc.now();
  uint32_t unixTime = now.unixtime();
  return unixTime;
}

void printDateTime(){
  DateTime now = rtc.now();

  Serial.println();
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();

}