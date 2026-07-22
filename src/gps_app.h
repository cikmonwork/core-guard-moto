#include <Arduino.h>
#include <model/classes_app.h> 
#include <functions_app.h> 
#include <map>
#include <string>
#include <HardwareSerial.h>
#include <TinyGPS++.h>

#ifndef GPS_APP_H
#define GPS_APP_H

extern HardwareSerial SerialGPS;
extern TinyGPSPlus gps;

extern uint32_t lastPointTime;

void saveGPS();

#endif