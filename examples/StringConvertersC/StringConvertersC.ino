/*
  An example of using the library for converting JSON/UTF-8, JSON/UTF-16, URL strings.
  Option with a static buffer.

  Пример использования библиотеки ковертации строк JSON/UTF-8, JSON/UTF-16, URL.
  Вариант со статическим буфером.
*/

#include <StringConvertersC.h>

void setup() {
  Serial.begin(115200);
  
  // Строка для примера, в ней сиволы ascii, символы которые обязательно экранировать, символы utf8 16 bit и 32 bit.
  //An example string containing ascii characters, characters that must be escaped, utf8 16-bit and 32-bit characters.
  const char test[] = "mini=3&p2=\"проверка\" 🔥.local";
  Serial.println(F("original string:"));
  Serial.println(test);

  // Пример полного кодирования в JSON/UTF-16. Прямой вызов функции.
  // Example of full encoding in JSON/UTF-16. Direct function call.
  String res = StringConvertersC::jsonEncode(test);
  Serial.println(F("encoded to Json/utf16:"));
  Serial.println(res);

  // Создание "объекта", для более компактного вызова функций. Дополнительного места в памяти не занимает.
  // Creating an "object" for more compact function calls. Doesn't take up any additional memory.
  StringConvertersC conv;

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

  // Пример использование статического буфера. Надо заранее расчитать размер буфера, для полного кодирование надо не меньше, чем в три раза больше исходного размера.
  // An example of using a static buffer. The buffer size must be calculated in advance; for full encoding, it must be at least three times the original size.
  
  // Буфер для кодирование и само кодироване
  // Buffer for encoding and encoding itself
  char encoded[sizeof(test)*3+1];
  conv.jsonEncode(encoded, test, sizeof(encoded));

  // Декодирование в буфер
  // Decoding to buffer
  char buf[sizeof(test)];
  Serial.println(F("decoded from Json/utf16 (static buffer):"));
  Serial.println(conv.jsonDecode(buf, encoded, sizeof(buf)));

  res = conv.jsonEscape(test);
  Serial.println(F("decoded from Json/utf8 (static buffer):"));
  Serial.println(conv.jsonDecode(buf, encoded, sizeof(buf)));

  // Декодирование в буфер содержащий исходную строку (замена)
  // Decoding into a buffer containing the original string (replacement)
  res = conv.jsonEscape(test);
  Serial.println(F("decoded from Json/utf8 (replacement):"));
  Serial.println(conv.jsonDecode(encoded, encoded, sizeof(encoded)));

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

/*
### StringConvertersС.h

Encoding and decoding strings into URL and JSON formats, version with a static buffer.

This version of the functions requires you to manually allocate a buffer; for small strings, it's convenient to do this on the stack, which completely eliminates memory fragmentation. This version of the library also has a slightly smaller footprint after compilation, but the difference is not significant. This version should only be used if memory fragmentation issues occur with the dynamic version of these functions.

All functions follow a similar pattern:
- A basic function that accepts a buffer to be converted and its size. This is useful for large strings or if there are many strings and a single buffer is used to reduce memory fragmentation.
- A simpler function that automatically allocates a buffer on the stack of approximately the required size. If the buffer is smaller than necessary, part of the string will simply be lost.
- An even simpler function that simply calls the memory allocation function.

If you use only the second and third forms, you lose the advantage of a static buffer. In this case, it's better to use the dynamic version of this library.

Since memory is allocated using a static stack buffer, it can overflow and reset the microcontroller. The stack size for the esp8266 is 4 KB, but it's relatively safe to use only 2 KB. For the esp32, it's twice as large. If the board resets, you'll need to manually allocate memory using malloc/free and specify this buffer to the function. However, the stack buffer size is usually sufficient. The default buffer size is 2000 bytes, but you can change it by setting #define SC_SET_MAX_BUFFER_SIZE XXXX _before_ linking the library.

```cpp
#define SC_SET_MAX_BUFFER_SIZE 1500
#include <StringConvertersС.h>
```

```cpp
// string encoding for GET requests
const char* urlEncode(char* buf, const char* str, size_t max_length, bool params = false)
String urlEncode(const char* str, bool params = false)
String urlEncode(const String &str, bool params = false)

// decoding a string from the GET request format
const char* urlDecode(char* buf, const char* str, size_t max_length)
String urlDecode(const char* str)
String urlDecode(const String &str)

// Simple JSON escaping without encoding conversion
const char* jsonEscape(char *buf, const char *str, size_t max_length)
String jsonEscape(const char* str)
String jsonEscape(const String& str)

// Converting a JSON string from UTF-8 to UTF-16 like \uABCD
const char* jsonEncode(char* buf, const char *str, size_t max_length)
String jsonEncode(const char* str)
String jsonEncode(const String &str)

// JSON converter from utf16 \uABCD to utf8 text
const char* jsonDecode(char* buf, const char* str, size_t max_length)
String jsonDecode(const char* str)
String jsonDecode(const String &str)
```

### описание StringConvertersС.h

Кодирование и декодирование строк в форматы для URL и JSON, версия со статическим буфером.

Для этой версии функций нужно самостоятельно выделять буфер, для строк небольшого размера удобно это делать в стеке, что полностью исключает фрагментирование памяти. Так-же эта версия библиотеки немного меньше занимает после компиляции, но разница не существенна. Использовать эту версию нужно только при возникновении проблемы фрагментации памяти с динамической версией этих функций. 

Все функции сделаны по одной схеме:
- Базовая функция, которая принимает готовый буфер для преобразования и его размер. Это полезно для больших строк или если строк много и используется один буфер для уменьшения фрагментации памяти
- Функция которая проше, она сама выделяет буфер в стеке примерно нужного размера. Если буфер оказался меньше, чем надо, то часть строки просто потеряется.
- Ещё более простая функция, которая просто ссылается на функцию выделения памяти.

Если пользоваться только второй и третьей формой, то пропадает преимущество статического буфера. Тогда лучше пользоваться динамической версией этой библиотеки.

Так как память выделяется статическим буфером в стеке, то может происходить его переполнение и перезагрузка микроконтроллера. Размер стека для esp8266 4kB, но можно относительно безопасно использовать только 2kB. Для esp32 всё в два раза больше. Если плата перезагружается, то надо самостоятельно выделить память через malloc/free и указать функции этот буфер. Но обычно размера в стеке достаточно. По умолчанию допустимый размер буфера 2000 байт, но его можно измениить узавав _перед_ подключением библиотеки ```#define SC_SET_MAX_BUFFER_SIZE XXXX```

```cpp
#define SC_SET_MAX_BUFFER_SIZE 1500
#include <StringConvertersС.h>
```

Немного занудства.  
- Один символ в UTF-8 может занимать 1, 2, 3 или 4 байта. Один символ в UTF-16 занимает 2 или 4 байта.
- Для URL один символ за пределами букв и цифр кодируется не по символам, а по байтам. Это %XX на каждый байт символа. То есть в худшем варианте, когда все символы надо закодировать, на выходе мы получим x3 относительно исходного размера.
- Для Json есть два варианта кодирования, в UTF-8 и UTF-16. Оба должны восприниматься, но почему-то чаще Json кодируется в utf-16. В итоге, при декодировании, на выходе буфер будет всегда меньше или равен исходному размеру.
- При кодировании в Json/UTF-8 (функция jsonEscape), кодируются только управляющие символы, вроде \t\n или \u0002. На выходе буфер будет равен или немного больше, зависит от количества управляющих символов. Но если строка состоит только из "больших" управляющих символов, то выходной буфер будет x6.
- При кодировнии в Json/UTF-16 (функция jsonEncode), кодируются все многобайтные символы и управляющие. Для двухбайтного символа будет \uXXYY, то есть x3, для 3 или 4 байта будет \uXXYY\uZZCC, то есть x4 и x3. И не забываем при x6.
- В функциях автоматически выделяется +50 байт для jsonEscape, в надежде, что в реальной жизни никто страдать ерундой не будет.
- В функциях urlEncode и jsonEncode выделяется буфер x3, как среднее и самое вероятное.
- Если буфер окажется маленьким, чтобы поместить всё, то строка просто обрежется. Но в реальности у меня такого не происходило.

```cpp
// кодирование строки для GET запросов
const char* urlEncode(char* buf, const char* str, size_t max_length, bool params = false)
String urlEncode(const char* str, bool params = false)
String urlEncode(const String &str, bool params = false)

// декодирование строки для GET запросов
const char* urlDecode(char* buf, const char* str, size_t max_length)
String urlDecode(const char* str)
String urlDecode(const String &str)

// Простое экранирование для json без конвертации кодировки
const char* jsonEscape(char *buf, const char *str, size_t max_length)
String jsonEscape(const char* str)
String jsonEscape(const String& str)

// Ковертация строки для json, из utf8 в utf16 вида \uABCD
const char* jsonEncode(char* buf, const char *str, size_t max_length)
String jsonEncode(const char* str)
String jsonEncode(const String &str)

// Конвертер из json в utf16 вида \uABCD в текст utf8
const char* jsonDecode(char* buf, const char* str, size_t max_length)
String jsonDecode(const char* str)
String jsonDecode(const String &str)
```

*/