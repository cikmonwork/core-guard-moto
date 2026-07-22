#ifndef MEMORY_MODULE_EER_APP_H
#define MEMORY_MODULE_EER_APP_H

#include <Wire.h>
#include "uEEPROMLib.h"
#include <variables_app.h>

uEEPROMLib eeprom(0x50);

const int eeAddress_IsProtect = 0; 
const int eeAddress_LastAlarm = 1; 
const int eeAddress_LastAlarmDT = 5; 
const int eeAddress_LastAlarmLocationLat = 9; 
const int eeAddress_LastAlarmLocationLon = 13; 

void writeIsProtect(bool val){
  eeprom.eeprom_write(eeAddress_IsProtect, val);
}

bool readIsProtect(){
  bool out = 0;
  eeprom.eeprom_read(eeAddress_IsProtect, &out);
  return out;
}

void writeLastAlarm(unsigned long val){
  eeprom.eeprom_write(eeAddress_LastAlarm, val);
}

unsigned long readLastAlarm(){
  unsigned long out = 0;
  eeprom.eeprom_read(eeAddress_LastAlarm, &out);
  return out;
}

void writeLastAlarmDT(unsigned long val){
  eeprom.eeprom_write(eeAddress_LastAlarmDT, val);
}

unsigned long readLastAlarmDT(){
  unsigned long out = 0;
  eeprom.eeprom_read(eeAddress_LastAlarmDT, &out);
  return out;
}


void writeLastAlarmLocationLat(float val){
  eeprom.eeprom_write(eeAddress_LastAlarmLocationLat, val);
}

float readLastAlarmLocationLat(){
  float out = 0;
  eeprom.eeprom_read(eeAddress_LastAlarmLocationLat, &out);
  return out;
}

void writeLastAlarmLocationLon(float val){
  eeprom.eeprom_write(eeAddress_LastAlarmLocationLon, val);
}

float readLastAlarmLocationLon(){
  float out = 0;
  eeprom.eeprom_read(eeAddress_LastAlarmLocationLon, &out);
  return out;
}

void setFirstInitEEProm(){
    writeIsProtect(false);
    writeLastAlarm(0);
    writeLastAlarmDT(0);
    writeLastAlarmLocationLat(0.0);
    writeLastAlarmLocationLon(0.0);
}

void getFirstEEProm(){
   if(readIsProtect()){
    isProtect = true;
   } 

  unsigned long  alarmstatus = readLastAlarm();
  if(alarmstatus != 0){
    unsigned long  lastAlarmDT = readLastAlarmDT();
    unsigned long  lastAlarmLat = readLastAlarmLocationLat();
    unsigned long  lastAlarmLon = readLastAlarmLocationLon();
    Notification notification = Notification(lastAlarmDT, alarmstatus, 0);
    notificationsAlarmLast = notification;
  }

}


#endif



// void setup() {
//   Serial.begin(115200);

//   Wire.begin();

//   int inttmp = 32123;
//   float floattmp = 3.1416;
//   char chartmp = 'A';
//   char c_string[] = "lastminuteengineers.com";  // 23 Characters
//   int string_length = strlen(c_string);

//   Serial.println("Writing into memory...");

//   // Write an int
//   if (!eeprom.eeprom_write(0, inttmp)) {
//     Serial.println("Failed to store int.");
//   } else {
//     Serial.println("int was stored correctly.");
//   }

//   // write a float
//   if (!eeprom.eeprom_write(4, floattmp)) {
//     Serial.println("Failed to store float.");
//   } else {
//     Serial.println("float was stored correctly.");
//   }

//   // Write single char at address
//   if (!eeprom.eeprom_write(8, chartmp)) {
//     Serial.println("Failed to store char.");
//   } else {
//     Serial.println("char was stored correctly.");
//   }

//   // Write a long string of chars FROM position 33 which isn't aligned to the 32 byte pages of the EEPROM
//   if (!eeprom.eeprom_write(33, (byte *)c_string, strlen(c_string))) {
//     Serial.println("Failed to store string.");
//   } else {
//     Serial.println("string was stored correctly.");
//   }

//   Serial.println("");
//   Serial.println("Reading memory...");

//   Serial.print("int: ");
//   eeprom.eeprom_read(0, &inttmp);
//   Serial.println(inttmp);


//   Serial.print("int2: ");
//   int inttmp2 = 0;
//   eeprom.eeprom_read(0, &inttmp2);
//   Serial.println(inttmp2);

//   Serial.print("float: ");
//   eeprom.eeprom_read(4, &floattmp);
//   Serial.println((float)floattmp);

//   Serial.print("char: ");
//   eeprom.eeprom_read(8, &chartmp);
//   Serial.println(chartmp);

//   Serial.print("string: ");
//   eeprom.eeprom_read(33, (byte *)c_string, string_length);
//   Serial.println(c_string);

//   Serial.println();
// }