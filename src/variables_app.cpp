#include <Arduino.h>
#include "variables_app.h"
#include <model/classes_app.h> 
#include <functions_app.h> 
#include <map>
#include <string>
#include <model/LocationsStore.h>
#include <model/FlatRegistry.h>
#include <model/DataStore.h>


String testEsp = "";

// системные переменные
char versionBleDataP4 = '0';// [0-9, a-z, A-Z] версии данных чтоб телефон мог сравнить свою с текущей
bool isNotGenKeyArr = true; // чтоб не генерировать каждый раз после выхода из сна
bool isProtect = 0; // стоит ли на сигнализации
volatile bool isSleep = 0; // режим сниженного потребления
int protectFromSeconds = -1; // поставить на сигнализацию через определенное вреемя секунды
volatile byte alarmState = 0; // сработала ли сигнализация 0-не сработала, 1-сработала, 2-совершены звонки
byte keysMinuteVisibleTakt = 3; // сколько тактов минутного таймера будут видны ключи
volatile byte workAntenna = 0; // 0 - ничего не работает - 1-блютуз. 2 - wifi 
volatile bool motionDetected = false; // флага при срабатывании датчика удара
volatile bool gerkonDetected = false; // флага при срабатывании датчика геркона
volatile bool isSendAlarms = false; // флаг чтоб отправить уведомления один раз при сработке сигнализации
volatile bool isUpdateKeys = false; // флаг проверки добавиления/удаления ключей

// Relays and GPS frequency are now managed via deviceSettings struct

//ble
String deviceNameBle = "svss"; // как будет отображатся устройство при подкл блютуз
bool isClearBLE = false; // нужно ли очистить и обновить все каналы блютуз

DeviceSettingModel deviceSettings;
volatile bool isSettingsChangedBle = false;
volatile bool isSettingsPhoneChangedBle = false;
volatile bool isSettingsWifiChangedBle = false;


String blePass = "ZBJmBEnc6KoP926a01yuYq33rZN92h2d5T7TALBA"; // пароль блютуз для получения массивы ключей
int countPassArr = 3; // число попыток неправильного ввода
byte errorInputPass = 0; // попытки неправильного ввода пароля
byte errorInputKey = 0; // попытки неправильного ввода ключей(команд)


byte keyArrNumberArr[32] = {}; // крипто ключ шифрования перемешанный номера элементов ключа


// wifi 
// String urlKeys = "https://script.google.com/macros/s/AKfycbzje06bs0Fn7OamFNMGmBDge3OBElKckgpewowSvBYgxFChyY4zBoP-CDyFfR3rJXGeBw/exec";
// String urlData = "https://script.google.com/macros/s/AKfycbz9aJnIq7TdQ4DTJNq04CWBfWT0Yiim9RRuUBp68USDxNbaT2uz259rqPubj5FgYBsoQQ/exec";
String urlKeys = "";
String urlData = "";
volatile byte isSendWifi = 0; // 0 - уже отправлено 1 - ждет отпраки 2-100 - ожидание


// WiFi SSIDs are now managed via deviceSettings struct

// Values and sensors are now managed via deviceSettings struct



Notification notificationsAlarmLast = Notification(); // последнее уведомление тревоги

LocationsStore locStore(500); // массив координат
String aesIv = "2123456789123456";
FlatRegistry<String> aesRegistry(50); // Реестр для AES ключей
DataStore<Notification> notifyStore(100); // массив уведомлений
