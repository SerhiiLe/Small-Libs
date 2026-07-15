# Description

A set of small functions for implementing typical algorithms in Arduino.

Good practices are passed around from project to project. I simply copy them. But what if we collected them all in one library and simply connected them instead of copying them? Some functions are trivial, some are obvious, some are strange, but they all solve a problem. The library will gradually grow.

Набор небольших функций для реализации типовых алгоритмов в Arduino.

Хорошие наработки гуляют из проекта в проект. Я их просто копирую. Но если их все собрать в одну библиотеку и просто подключать, а не копировать? Некоторые функции банальны, некоторые очевидны, некоторые странные, но все они решают какую-то задачу. Библиотека потихоньку будет наполняться.

## Table of contents ([ru](#оглавление))

- [How to install](#how-to-install)
- [How to use](#how-to-use)

- [The simplest timer for organizing delays instead of delay()](#timerminimh)
- [A template class for a simple timer](#timerminimthpp)
- [Checksum calculations](#fletcherchecksumh)
- [Encoding and decoding strings into URL and JSON formats, #RRGGBB and time](#stringconvertersh)
- [A set of functions to simplify the organization of an HTTP server](#webserverutilsh)
- [Repeating actions (bink)](#blinkminimh)

## How to install

There are three options:  
- Unzip the archive into the libraries folder of the Arduino.
- Add lib_deps = https://github.com/SerhiiLe/Small-Libs to the platformio.ini section.
- Simply copy the required file into your project folder.

## How to use

Include the necessary files using ```#include <>``` or all at once using ```#include <Small-Libs.h>```

Even though you may not need all the functions, this won't add any extra code to the final firmware. The linker will strip out all the functions you don't use. It's proven.

You can create global objects or create objects locally within your functions, only where needed. However, it's better to make webServerUtils.h a global object, as it requires relatively cumbersome initialization.

Aside from webServerUtils.h, all functions are platform-independent and can run anywhere; they're just algorithms. This includes non-Arduino functions. However, you may need to change the variable types.

## Description of functions

All functions are implemented as objects to minimize collisions with global names in projects.

In functions that require passing a buffer size, this is done intentionally, as creating template functions instead of directly specifying the size significantly bloats the resulting binary code. Shortening the source code and making the syntax more elegant is counterproductive to saving firmware size.

### TimerMinim.h

A tool for skipping a certain amount of time. Yes, skipping is correct; it's not a full-fledged timer that runs on interrupts and calculates with microsecond accuracy. Even compared to other millis() timers, it doesn't calculate precise intervals. As practice has shown, all this is unnecessary in real life. You need to check whether the required time has passed to perform some action. The idea was honestly borrowed from the [GyverMatrixWiFi](https://github.com/vvip-68/GyverMatrixWiFi) project, where I was struck by its simplicity and incorporated it into my clock project. Over time, the code bloated, suffered from gigantism, and then shrank again, throwing out everything that turned out to be unnecessary. The result is a small and compact object.

```cpp
// timer declaration with interval specification
TimerMinim(uint32_t interval=60000)

// setting the timer interval and resetting the timer
void setInterval(uint32_t interval)

// returns true when the time has come
bool isReady()

// manual reset of the timer, the countdown will start from the beginning
void reset()

// set a delay until the next triggering, or duplicates reset() if the delay is 0
void setNext(uint32_t next = 0)
```

### TimerMinimT.hpp

Everything is similar to timerMinim.h, except that you need to pass a function that counts ticks when creating it. See examples for more details.

```cpp
// initialization example
timerMinimT<my_time_func> timer;
```

### FletcherChecksum.h

Checksum calculation. The input is always a byte array and its size. The output is the checksum.

```cpp
// Calculating the checksum using the Flatcher8 formula
uint8_t fletcher8(uint8_t *data, uint16_t len)

// Calculating the checksum using the Flatcher16 formula
uint16_t fletcher16(uint8_t *data, size_t len)

// Calculating the CRC32 checksum
uint32_t calculateCRC32(const uint8_t *data, size_t length)
```
Example:

```cpp
#include FletcherChecksum.h

// example of data whose checksum needs to be calculated
struct some_data {
    int a = 1;
    char str[50] = "hello world!";
} data;

// some function where it is necessary to calculate a checksum
void some_func() {
    int sum = FletcherChecksum::fletcher16((uint8_t*)&data, sizeof(some_data));
    Serial.print(sum);
}
```

### StringConverters.h

Encoding and decoding strings into URL and JSON formats

Buffers for conversion are automatically allocated on the heap (String). If there is insufficient memory for full encoding, the string will be truncated.

```cpp
// string encoding for GET requests
String urlEncode(const char* str, bool params = false)
String urlEncode(const String &str, bool params = false)

// decoding a string from the GET request format
String urlDecode(const char* str, bool params = false)
String urlDecode(const String &str, bool params = false)

// Simple JSON escaping without encoding conversion
String jsonEscape(const char* str)
String jsonEscape(const String& str)

// Converting a JSON string from UTF-8 to UTF-16 like \uABCD
String jsonEncode(const char* str)
String jsonEncode(const String &str)

// JSON converter from utf16 \uABCD to utf8 text
String jsonDecode(const char* str)
String jsonDecode(const String &str)

// #RRGGBB to uint32_t
uint32_t text_to_color(const char *s)

// uint32_t to #RRGGBB
String color_to_text(uint32_t c)

// decoding the time specified in the input->time field (HH:MM)
uint16_t text_to_time(String s)

// encoding the time specified in HH:MM
static String time_to_text(uint16_t time)
```

It can be used in the form of:

```cpp
#include <StringConverters.h>

void some_func() {
    StringConverters conv;
    String result = conv.jsonEncode("просто что-то");
}
```

And in the form:

```cpp
#include <StringConverters.h>

void some_func() {
    String result = StringConverters::jsonEncode("просто что-то");
}
```

This does not affect the size of the code after compilation, only the convenience and readability of the code.

### WebServerUtils.h

A set of functions to simplify the organization of an HTTP server.

This is a set of individual standard functions, not a ready-made solution. They solve several problems:
- serving static files with browser caching
- converting parameters to variables to simplify form processing.
- adjusting received parameters to an acceptable range.
- recognizing that there has been a change in parameters and that the changes need to be recorded.

See the example for more details.

```cpp
// A constructor specifying which server is used. WebServer for ESP32 and ESP8266 is supported.
WebServerUtils(Server& srv)

// change flag, changes to true when the new value does not match the old one
bool need_save = false;

// Sending a file. The first parameter is the file path. The second parameter is a function that enables caching for specific files.
bool fileSend(const String &path, bool (*cc)(const String &p)=nullptr)

// Sets the browser cache lifetime in seconds, defaulting to one hour = 3600 seconds
void setCacheLive(uint16_t cache_life_time)

// determining whether a checkbox is selected or not
template <typename F>
bool checkbox(F *name, uint8_t &var)

// definition of integers
template <typename F, typename T>
bool to_int(F *name, T &var, long from, long to)

// definition of floating-point numbers
template <typename F>
bool to_float(F *name, float &var, float from, float to, float prec=8.0f)

// string definition (for String)
template <typename F>
bool to_string(F *name, String &var)

// string definition (for char[])
template <typename F>
bool to_string(F *name, char * var, size_t len)

// time definition, HH:MM to uint16_t
template <typename F>
bool time(F *name, uint16_t &var)

// storing the color from the string #RRGGBB in a uint32_t variable
bool color(F *name, uint32_t &var)

// decoding the time specified in the input->time field (HH:MM)
static uint16_t decode_time(String s)
```

### BlinkMinim.h

Repeating actions. In the simplest case, it controls a pin (LED). But it can also act as a timer and perform more complex repeating actions.

```cpp
// pin - PIO pin number, mode - LED switching on at LOW or HIGH level
BlinkMinim(uint8_t pin=255, uint8_t level=HIGH, void (*writeBit)(uint8_t, uint8_t)=*digitalWrite, int (*readBit)(uint8_t)=*digitalRead)

// duplicates the constructor for cases where it was called without arguments. Can be called multiple times
void begin(uint8_t pin, uint8_t level=HIGH, void (*writeBit)(uint8_t, uint8_t)=*digitalWrite, int (*readBit)(uint8_t)=*digitalRead)

// stop blink
void clean()

// Must be called in loop()
void tick()

// start or stop blink
void blink(uint8_t mode=1, uint16_t interval=500, uint16_t cnt=0, uint16_t duration=0, void (*userFunc)(void)=nullptr)

// set the pin state
void set(uint8_t mode=1)

// invert the state of the pin
void invert()

// active or not
uint8_t state()
```

For more details, see the example.


## Оглавление

- [Как установить](#как-устанавливить)
- [Как пользоваться](#как-пользоваться)

- [Простейший таймер для организации задержек вместо delay()](#описание-timerminimh)
- [Шаблонный класс простейшего таймера](#описание-timerminimthpp)
- [Вычисления контрольных сумм](#описание-fletcherchecksumh)
- [Кодирование и декодирование строк в форматы для URL и JSON, #RRGGBB и время](#описание-stringconvertersh)
- [Набор фукции для упрощения организации http сервера](#описание-webserverutilsh)
- [Повторяющиеся действия (blink)](#описание-blinkminimh)

## Как устанавливить

Вариантов три:
- распаковать архив в libraries папки Arduino.
- добавить в platformio.ini в секции lib_deps = https://github.com/SerhiiLe/Small-Libs
- просто скопировать нужный файл в папку своего проекта

## Как пользоваться

Подключить нужные файлы через ```#include <>``` или всё сразу ```#include <Small-Libs.h>```

Несмотря на то, что не все функции Вам могут понадобиться, лишнего кода в итоговую прошивку это не добавит. Линковщик вырежет все функции, которые Вы не использовали. Проверенно.

Можно создать глобальные объекты, или создавать объекты по месту вутри Ваших функий, только там, где это надо. Только webServerUtils.h лучше делать глобальным объектом, так как там относительно громоздкая инициализация.

Кроме webServerUtils.h все функции не привязаны к платформам и могут работать где угодно, это просто алгоритмы. В том числе не на Arduino. Но может понадобится изменение типов переменных.

## Описание функций

Все функции оформлены как объекты, для минимизации пересечения с глобальными именами в проектах.

В функциях, где надо передавать размер буфера, это сделано специально, так как создание шаблонных функций вместо прямого указания размера сильно раздувает итоговый бинарный код. Сокращение исходного кода и более красивый синтаксис против экономии размера прошивки.

### описание TimerMinim.h

Средство для пропуска нужного времени. Да, именно пропуска, это не полноценный таймер, который работает на прерываниях и расчитывает с точностью до микросекунд. Даже по сравнению с другими таймерами на millis(), он не вычисляет точные интервалы. Как показала практика, всё это не нужно в реальной жизни. Нужно посмотреть прошло ли нужное время, чтобы выполнить какое-то действие. Идея была честно взята у проекта [GyverMatrixWiFi](https://github.com/vvip-68/GyverMatrixWiFi), где меня поразила простата и взял его к себе в проект часов. Со временем код раздувался, страдал гигантизмом и опять усыхал, выбрасывая всё, что оказалось не нужно. В результате получился маленький и компактный объект.

```cpp
// объявление таймера с указанием интервала
TimerMinim(uint32_t interval=60000)

// установка интервала работы таймера и сброс таймера
void setInterval(uint32_t interval)

// возвращает true, когда пришло время.
bool isReady()

// ручной сброс таймера, отсчёт начнётся с начала
void reset()

// выставить задержку до следующего срабатывания, или дублирует reset(), если задержка 0.
void setNext(uint32_t next = 0)
```

Более детальные примеры в examples.

### описание TimerMinimT.hpp

Всё аналогично timerMinim.h, кроме того, что нужно при создании передавать функцию, котороая считает тики. Подробнее в examples

```cpp
// пример инициализации
timerMinimT<my_time_func> timer;
```

### описание FletcherChecksum.h

Расчёт контрольной суммы. На входе всегда массив байт и его размер. На выходе контрольная сумма

```cpp
// Расчёт контрольно суммы по формуле Flatcher8
uint8_t fletcher8(uint8_t *data, uint16_t len)

// Расчёт контрольно суммы по формуле Fletcher16
uint16_t fletcher16(uint8_t *data, size_t len)

// Расчёт контрольной суммы по формуле CRC32
uint32_t calculateCRC32(const uint8_t *data, size_t length) {
```

Пример:

```cpp
#include FletcherChecksum.h

// пример данных, контрольную сумму которых надо подсчитать
struct some_data {
    int a = 1;
    char str[50] = "hello world!";
} data;

// какая-то функция, где надо подсчитать контрольную сумму
void some_func() {
    int sum = FletcherChecksum::fletcher16((uint8_t*)&data, sizeof(some_data));
    Serial.print(sum);
}
```

### описание StringConverters.h

Кодирование и декодирование строк в форматы для URL и JSON

Буфера для конвертации выделяются автоматически в куче (String). Если для полного кодирования памяти мало, то строка будет обрезана.

```cpp
// кодирование строки для GET запросов
String urlEncode(const char* str, bool params = false)
String urlEncode(const String &str, bool params = false)

// декодирование строки для GET запросов
String urlDecode(const char* str, bool params = false)
static String urlDecode(const String &str, bool params = false)

// Простое экранирование для json без конвертации кодировки
String jsonEscape(const char* str)
String jsonEscape(const String& str)

// Ковертация строки для json, из utf8 в utf16 вида \uABCD
String jsonEncode(const char* str)
String jsonEncode(const String &str)

// Конвертер json из utf16 вида \uABCD в текст utf8
String jsonDecode(const char* str)
String jsonDecode(const String &str)

// #RRGGBB в uint32_t
uint32_t text_to_color(const char *s)

// uint32_t в #RRGGBB
String color_to_text(uint32_t c)

// перевод строки вида HH:MM в число, количество минут с полуночи
uint16_t text_to_time(String s)

// перевод количество минут в строку вида HH:MM
static String time_to_text(uint16_t time)
```

Использовать можно как в виде:

```cpp
#include <StringConverters.h>

void some_func() {
    StringConverters conv;
    String result = conv.jsonEncode("просто что-то");
}
```

Так и в виде:

```cpp
#include <StringConverters.h>

void some_func() {
    String result = StringConverters::jsonEncode("просто что-то");
}
```

На размер кода после компиляции это не влияет, только на удобство и читаемость кода.

### описание WebServerUtils.h

Набор фукции для упрощения организации http сервера.

Это именно набор отдельных типовых функций, а не готовое решение. Они решают несколько задач:
- отдача статических файлов с кешированием в броузере
- перевод параметров в переменные для упрощения обработки форм.
- подогнать полученные параметры под допустимый диапазон.
- понять, что есть изменение в параметрах и надо записать изменения.

Более детально смотрите в примере.

```cpp
// конструктор с указанием, какой именно сервер используется. Поддерживается WebServer для esp32 и ESP8266WebServer для esp8266
WebServerUtils(Server& srv)

// флаг изменения, меняется в true когда новое значение не совпадает со старым
bool need_save = false;

// Отправка файла. Первый параметр - путь к файлу. Второй параметр - функция которая разрешает кеширование для определённых файлов
bool fileSend(const String &path, bool (*cc)(const String &p)=nullptr)

// установка времени жизни кэша в броузере, в секундах, по умолчанию один час = 3600 секунд
void setCacheLive(uint16_t cache_life_time)

// определение выбран checkbox или нет
template <typename F>
bool checkbox(F *name, uint8_t &var)

// определение простых целых чисел
template <typename F, typename T>
bool to_int(F *name, T &var, long from, long to)

// определение дробных чисел
template <typename F>
bool to_float(F *name, float &var, float from, float to, float prec=8.0f)

// определение простых строк (для String)
template <typename F>
bool to_string(F *name, String &var)

// определение простых строк (для char[])
template <typename F>
bool to_string(F *name, char * var, size_t len)

// определение времени
template <typename F>
bool time(F *name, uint16_t &var)

// сохранение цвета из строки #RRGGBB в переменной uint32_t
bool color(F *name, uint32_t &var)

// выделение времени из текста (HH:MM)
static uint16_t decode_time(String s)
```

### описание BlinkMinim.h

Повторяющиеся действия. В простейшем случае управление pin (светодиодом). Но может, в том числе играть роль таймера и выполнять более сложные повторяющиеся действия.

```cpp
// pin - номер ножки PIO, mode - включение светодиода по низкому LOW или высокому HIGH уровню
BlinkMinim(uint8_t pin=255, uint8_t level=HIGH, void (*writeBit)(uint8_t, uint8_t)=*digitalWrite, int (*readBit)(uint8_t)=*digitalRead)

// дублирует конструктор, для случаев, если он был вызван без аргументов
void begin(uint8_t pin, uint8_t level=HIGH, void (*writeBit)(uint8_t, uint8_t)=*digitalWrite, int (*readBit)(uint8_t)=*digitalRead)

// stop blink
void clean()

// Надо вызывать в loop()
void tick()

// запуск или остановка blink
void blink(uint8_t mode=1, uint16_t interval=500, uint16_t cnt=0, uint16_t duration=0, void (*userFunc)(void)=nullptr)

// принудительно установить состояние пина
void set(uint8_t mode=1)

// инвертировать состояние пина
void invert()

// Узнать, активен ли блинк
uint8_t state()
```

Детальнее в примере
