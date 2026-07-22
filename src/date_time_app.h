#ifndef DATE_TIME_APP_H
#define DATE_TIME_APP_H

#include <Arduino.h>
#include "RTClib.h"

extern RTC_DS1307 rtc;

void setDateTime(DateTime);

void getNow();

String getNowString();
uint32_t getCurrentTime();
void printDateTime();

#endif