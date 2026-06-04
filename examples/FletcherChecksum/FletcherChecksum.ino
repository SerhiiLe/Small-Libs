/*
  An example of using the checksum calculation library

  Пример использования библиотеки расчёта контрольной суммы
*/

#include <FletcherChecksum.h>

// пример данных, контрольную сумму которых надо подсчитать
// example of data whose checksum needs to be calculated
struct some_data {
    int a = 1;
    char str[50] = "hello world!";
} data;


void setup() {
  Serial.begin(115200);

  // Пример для 16 бит версии. Достаточно для обычного использования
  // Example for the 16-bit version. Sufficient for normal use.

  uint16_t sum16 = FletcherChecksum::fletcher16((uint8_t*)&data, sizeof(some_data));

  Serial.print(F("fletcher16: "));
  Serial.println(sum16);

  // Пример для 8 бит версии. Точность ниже, но достаточна для к примеру верификации EEPROM
  // Example for the 8-bit version. The accuracy is lower, but sufficient for, for example, EEPROM verification.
  
  uint8_t sum8 = FletcherChecksum::fletcher8((uint8_t*)&data, sizeof(some_data));
  
  Serial.print(F("fletcher8: "));
  Serial.println(sum8);

}

void loop() {
  // put your main code here, to run repeatedly:

}
