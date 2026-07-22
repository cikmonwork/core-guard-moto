#ifndef FUNCTIONS_APP_H
#define FUNCTIONS_APP_H

#include <Arduino.h>
#include <model/classes_app.h>

void printSystemConfig();
void printStatus();

String generateRandomString(int8_t count);
String generateRandomCharKey();
bool isErrorProt();
void addNotifications(int8_t event);
void addNotificationsAlarm(int8_t event);
String getCurStatus();
void stopProtect();
void startProtect();

Location getLastLocation();

float haversine(float lat1, float lon1, float lat2, float lon2);
float toRadians(float degrees);

byte getBatteryVoltagePercent();

void checkSensors();
void alarmPhones();


void motorWork(bool isWork);
void handleRelayEvent(byte eventType);
void releON(byte eventType);
void releOff();

String generateRandomString(int length);
void updateVersionBleDataP4();
String getBatteryVoltagePercentString();


#endif
