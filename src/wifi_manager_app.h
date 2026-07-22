#ifndef WIFI_MANAGER_APP_H
#define WIFI_MANAGER_APP_H

#include <Arduino.h>


void sendDataWifi();
void sendDataWifiPost();
bool sendLocations();
bool sendNotifications();
bool sendLatencyStatus();
void sendKeys();
void performHttpsRequest();
byte sendSmartPatch(String fullUrl, String jsonBody);
String sendSmartGet(String fullUrl);
bool syncSettingsFromApi();
bool isUpdateSettingsApi();
bool syncGeneralSettings();
bool syncPhones();
bool syncWifi();
bool pushSettingsToApi();
bool pushGeneralSettingsToApi();
bool pushPhonesToApi();
bool pushWifiToApi();


#endif
