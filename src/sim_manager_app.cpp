#include <Arduino.h>
#include <model/classes_app.h>
#include <map>
#include <string>
#include <variables_app.h>
#include <HardwareSerial.h>

HardwareSerial sim800(2);

void initSim()
{
    sim800.begin(9600, SERIAL_8N1, 33, 32);
    delay(250);
    sim800.print("AT+CMGF=1\r");
}

void callPhone(u_int64_t phone)
{
    if (phone != 0){
        String command = "ATD+";
        command += phone;
        command +=";";
        sim800.println(command);
        delay(15000);
        sim800.println("ATH0;");
        delay(1000);
    }
}

void alarmPhones()
{
    for (int i = 0; i < 8; i++) {
        callPhone(deviceSettings.phone[i]);
    }
}
