#include <Arduino.h>
#include <model/classes_app.h> 
#include <map>
#include <string>
#include <HardwareSerial.h>

#ifndef SIM_MANAGER_APP_H
#define SIM_MANAGER_APP_H

extern HardwareSerial sim800;

void alarmPhones();
void initSim();
void callPhone();

#endif