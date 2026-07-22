#include <Arduino.h>

#ifndef CRYPT_APP_H
#define CRYPT_APP_H

extern size_t MAX_PAYLOAD_LENGTH; // Максимальная длина дешифрованной/шифрованной полезной нагрузки (в байтах)
extern size_t MAX_LORA_PAYLOAD_LENGTH; // Максимальная длина полезной нагрузки для LoRa (в байтах, типичное значение)


 String decryptBleString(String encryptText, String pId);
 String encriptBleString(String text, String pId);
 String generateKey(String pId);
 void getKeysFromMemory();
 String addOrUpdateKey(String pId);


int decodeBase64(const char* input, unsigned char* output);
String encryptAES(const char* plainText, const char* keyStr, const char* ivStr);
String decryptAES(const char* base64Input, const char* keyStr, const char* ivStr);

#endif