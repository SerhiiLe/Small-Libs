/*
  An example of using the library for converting JSON/UTF-8, JSON/UTF-16, URL strings.
  Option with dynamic buffer.

  Пример использования библиотеки ковертации строк JSON/UTF-8, JSON/UTF-16, URL.
  Вариант с динамическим буфером.
*/

#include <StringConverters.h>

void setup() {
  Serial.begin(115200);
  
  // Строка для примера, в ней сиволы ascii, символы которые обязательно экранировать, символы utf8 16 bit и 32 bit.
  //An example string containing ascii characters, characters that must be escaped, utf8 16-bit and 32-bit characters.
  const char test[] = "mini=3&p2=\"проверка\" 🔥.local";
  Serial.println(F("original string:"));
  Serial.println(test);

  // Пример полного кодирования в JSON/UTF-16. Прямой вызов функции.
  // Example of full encoding in JSON/UTF-16. Direct function call.
  String res = StringConverters::jsonEncode(test);
  Serial.println(F("encoded to Json/utf16:"));
  Serial.println(res);

  // Создание "объекта", для более компактного вызова функций. Дополнительного места в памяти не занимает.
  // Creating an "object" for more compact function calls. Doesn't take up any additional memory.
  StringConverters conv;

  // Декодирование обратно из JSON/UTF-16
  // Decoding back from JSON/UTF-16
  res = conv.jsonDecode(res);
  Serial.println(F("decoded from Json/utf16:"));
  Serial.println(res);

  // Облегчённое кодирование в JSON/UTF-8. Полученная строка занимает меньше места и обычно работает так-же как полное кодирование в JSON/UTF-16
  // Lightweight JSON/UTF-8 encoding. The resulting string takes up less space and generally performs just as well as full JSON/UTF-16 encoding.
  res = conv.jsonEscape(test);
  Serial.println(F("encoded to Json/utf8:"));
  Serial.println(res);

  // Декодирование обратно из JSON/UTF-8
  // Decoding back from JSON/UTF-8
  res = conv.jsonDecode(res);
  Serial.println(F("decoded from Json/utf8:"));
  Serial.println(res);


  // Кодирование в формат для URL
  // URL encoding
  res = conv.urlEncode(test);
  Serial.println(F("encoded to URL:"));
  Serial.println(res);

  // Кодирование в формат для URL с сохраннением формата параметров (символы & и =)
  // URL encoding with parameter formatting (& and = symbols) preserved
  res = conv.urlEncode(test, true);
  Serial.println(F("encoded to URL parameters:"));
  Serial.println(res);

  // Декодирование из формата для URL
  // URL decoding
  Serial.println(F("decoded from URL:"));
  Serial.println(conv.urlDecode(res));

}

void loop() {
  // put your main code here, to run repeatedly:

}
