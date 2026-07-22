#ifndef MEMORY_MODULE_EER_APP_H
#define MEMORY_MODULE_EER_APP_H

#include <Arduino.h>
#include "uEEPROMLib.h"
#include <variables_app.h>

extern uEEPROMLib eeprom;
extern const int eeAddress_IsProtect; 
extern const int eeAddress_LastAlarm; 
extern const int eeAddress_LastAlarmDT; 

void writeIsProtect(bool val);
bool readIsProtect();
void writeLastAlarm(unsigned long val);
unsigned long readLastAlarm();
void writeLastAlarmDT(unsigned long val);
unsigned long readLastAlarmDT();
void getFirstEEProm();
void setFirstInitEEProm();

void writeLastAlarmLocationLat(float val);
float readLastAlarmLocationLat();
void writeLastAlarmLocationLon(float val);
float readLastAlarmLocationLon();


#endif