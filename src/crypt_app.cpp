#include <Arduino.h>
#include <model/classes_app.h>
#include <variables_app.h>
#include "functions_app.h"
#include "memory_app.h"


#include "mbedtls/aes.h"
#include <libb64/cdecode.h>
#include <libb64/cencode.h>


size_t MAX_PAYLOAD_LENGTH = 400; // Максимальная длина дешифрованной/шифрованной полезной нагрузки (в байтах)
size_t MAX_LORA_PAYLOAD_LENGTH = 243; 

 void getKeysFromMemory();

String generateKey(String pId)
{
  String out = deviceSettings.deviceId + ";";
  String key = generateRandomString(32);
  out += key;
  out += aesIv;

  aesRegistry.put(pId, key);

  return out;
}

void getKeysFromMemory()
{
  beginRead(); // Открываем Preferences один раз ПЕРЕД циклом
  
  for (int i = 0; i < 50; i++)
  { 
    char kKey[12];
    char pKey[12];
    snprintf(kKey, sizeof(kKey), "aesKey%d", i);
    snprintf(pKey, sizeof(pKey), "aesPId%d", i);

    if (preferences.isKey(kKey) && preferences.isKey(pKey)) 
    {
        String keyVal = preferences.getString(kKey, "");
        String pidVal = preferences.getString(pKey, "");
        
        if (keyVal != "" && pidVal != "") {
            aesRegistry.put(pidVal, keyVal);
        }
    }
  }
  
  end(); // Закрываем один раз ПОСЛЕ цикла
}


String addOrUpdateKey(String pId)
{
  String key = generateRandomString(32);

  int keyIndex = aesRegistry.getIndex(pId);
  if (keyIndex == -1)
  {
    keyIndex = aesRegistry.put(pId, key);
    savePidAndKey(pId, key, keyIndex);
  }
  else
  {
    aesRegistry.put(pId, key);
    saveKeyMemory(pId, key, keyIndex);
  }
  return key;
};

String encodeBase64(const unsigned char* input, int length) {
    base64_encodestate s;
    base64_init_encodestate(&s);
    
    // Резервируем память: base64 занимает примерно на 33% больше места + запас
    char* encoded = (char*)malloc(length * 2); 
    
    int cnt = base64_encode_block((const char*)input, length, encoded, &s);
    cnt += base64_encode_blockend(encoded + cnt, &s);
    
    // Важно: libb64 может добавлять символ переноса строки \n в конце
    // Убираем его для корректной передачи по BLE
    String result = String(encoded);
    result.trim(); 
    
    free(encoded);
    return result;
}

int decodeBase64(const char* input, unsigned char* output) {
    base64_decodestate s;
    base64_init_decodestate(&s);
    int cnt = base64_decode_block(input, strlen(input), (char*)output, &s);
    return cnt;
}

String encryptAES(const char* plainText, const char* keyStr, const char* ivStr) {
    // 1. Подготовка данных и Padding (PKCS7)
    int plainLen = strlen(plainText);
    int paddingLen = 16 - (plainLen % 16);
    int encryptedLen = plainLen + paddingLen;

    unsigned char* inputBuffer = (unsigned char*)malloc(encryptedLen);
    memcpy(inputBuffer, plainText, plainLen);
    
    // Добавляем байты отступа (PKCS7: если не хватает 3 байта, пишем три раза 0x03)
    for (int i = 0; i < paddingLen; i++) {
        inputBuffer[plainLen + i] = (unsigned char)paddingLen;
    }

    // 2. Настройка AES
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    
    unsigned char key[32];
    unsigned char iv[16];
    memcpy(key, keyStr, 32);
    memcpy(iv, ivStr, 16); // Внимание: CBC меняет IV в процессе, используйте копию

    mbedtls_aes_setkey_enc(&aes, key, 256); // 256 бит для ключа в 32 символа

    unsigned char* outputBuffer = (unsigned char*)malloc(encryptedLen);

    // 3. Шифрование CBC
    int ret = mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, encryptedLen, iv, inputBuffer, outputBuffer);

    mbedtls_aes_free(&aes);
    free(inputBuffer);

    if (ret != 0) {
        free(outputBuffer);
        return "Error: Encryption fail";
    }

    // 4. Кодирование в Base64
    String base64Result = encodeBase64(outputBuffer, encryptedLen);
    
    free(outputBuffer);
    return base64Result;
}


String decryptAES(const char* base64Input, const char* keyStr, const char* ivStr) {
    // 1. Декодируем Base64
    int maxLen = strlen(base64Input);
    unsigned char* decodedBytes = (unsigned char*)malloc(maxLen);
    int decodedLen = decodeBase64(base64Input, decodedBytes);

    if (decodedLen <= 0) {
        free(decodedBytes);
        return "Error: Base64 fail";
    }

    // 2. Настройка AES
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    
    unsigned char key[32];
    unsigned char iv[16];
    memcpy(key, keyStr, 32);
    memcpy(iv, ivStr, 16);

    // Устанавливаем ключ 256 бит (для вашего ключа SrA9...32 символа)
    mbedtls_aes_setkey_dec(&aes, key, 256);

    unsigned char* output = (unsigned char*)malloc(decodedLen + 1);
    
    // 3. Дешифровка CBC
    int ret = mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, decodedLen, iv, decodedBytes, output);

    mbedtls_aes_free(&aes);
    free(decodedBytes);

    if (ret != 0) {
        free(output);
        return "Error: AES fail";
    }

    // 4. Удаление PKCS7 Padding
    int paddingValue = output[decodedLen - 1];
    int plainLen = decodedLen;
    if (paddingValue >= 1 && paddingValue <= 16) {
        plainLen -= paddingValue;
    }

    output[plainLen] = '\0';
    String result = String((char*)output);
    free(output);

    return result;
}


String decryptBleString(String encryptText, String pId)
{
  Serial.println("decryptBleString");
  Serial.println(encryptText);
  Serial.println(pId);
  
  String out = "";
  if (encryptText.length() > 24)
  {
    if (pId.length() > 4)
    {
      String valueStr = encryptText.substring(16, encryptText.length());
      String key = aesRegistry.get(pId);

      Serial.println(valueStr);
      Serial.println(key);
      if (key != "")
      {
        out = decryptAES(valueStr.c_str(), key.c_str(), "2123456789123456");
        int lengthStr = out.substring(0, 4).toInt() - 1000;

        out = out.substring(0, lengthStr);
      }
    }
  }

  return out;
}

String encriptBleString(String text, String pId)
{
   String key = aesRegistry.get(pId);
   key.trim();

   if(key.length() < 32) return "";

    unsigned char local_key[32];
    memcpy(local_key, key.c_str(), 32);
   
  //  unsigned char local_key[32] = {
  //       'S','r','A','9','n','I','x','b','8','P','X','M','a','3','M','q',
  //       'N','U','7','Y','w','X','2','Z','Z','B','i','3','U','y','7','N'
  //   };
   unsigned char local_iv[16] = {'2','1','2','3','4','5','6','7','8','9','1','2','3','4','5','6'};

   String out = encryptAES(text.c_str(), (const char*)local_key, (const char*)local_iv);

   return out;
}
