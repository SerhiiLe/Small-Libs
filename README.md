# Description

A set of small functions for implementing typical algorithms in Arduino.

Good practices are passed around from project to project. I simply copy them. But what if we collected them all in one library and simply connected them instead of copying them? Some functions are trivial, some are obvious, some are strange, but they all solve a problem. The library will gradually grow.

Набор небольших функций для реализации типичных  алгоритмов в Arduino.

Хорошие наработки гуляют из проекта в проект. Я их просто копирую. Но если их все собрать в одну библиотеку и просто подключать, а не копировать? Некоторые функции банальны, некоторые очевидны, некоторые странные, но все они решают какую-то задачу. Библиотека потихоньку будет наполняться.

## Table of contents ([ru](#оглавление))

- How to install
- How to use

- [The simplest timer for organizing delays instead of delay()](#timerminimh)

## Оглавление

- [Как установить](#как-устанавливить)
- [Как пользоваться](#как-пользоваться)

- [Простейший таймер для организации задержек вместо delay()](#timerminimh)
- [Шаблонный класс простейшего таймера](#timerminimthpp)
- [Вычисления контрольных сумм](#fletcherchecksumh)
- [Кодирование и декодирование строк в форматы для URL и JSON](#stringconvertersh)
- [Набор фукции для упрощения организации http сервера](#webserverutilsh)

## Как устанавливить

Вариантов три:
- распаковать архив в libraries папки Arduino.
- добавить в platformio.ini в секции lib_deps = https://github.com/SerhiiLe/Small-Libs
- просто скопировать нужный файл в папку своего проекта

## Как пользоваться

Подключить нужные файлы через ```#include <>``` или всё сразу ```<Small-Libs.h>```

Несмотря на то, что не все функции Вам могут понадобить, лишнего кода в итоговую прошивку это не добавит. Линковщик вырежет все функции, которые Вы не использовали. Проверенно.

Можно создать глобальные объекты, но лучше создавать объекты по месту втри Ваших функий, только там, где это надо. Глобальные объекты позволят съекономить пару сотнен байт прошивки, но могут привести к путаннице. Только webServerUtils.h лучше делать глобальным объектом, так как там относительно громоздкая инициализация.

Кроме webServerUtils.h все функции не привязаны к платформам и могут работать где угодно, это просто алгоритмы. В том числе не на Arduino. Но может понадобится изменение типов переменных.

## Description of functions

Все функции оформлены как объекты, для минимизации пересечения с глобальными именами в проектах.

### timerMinim.h

Средство для пропуска нужного времени. Да, именно пропуска, это не полноценный таймер, который работает на прерываниях и расчитывает с точностью до микросекунд. Даже по сравнению с другими таймера на millis(), он не вычисляет точные интервалы. Как показала практика, всё это не нужно в реальной жизни. Нужно посмотреть прошело ли нужное время, чтобы выполнить какое-то действие. Идея была честно взята у проекта [GyverMatrixWiFi](https://github.com/vvip-68/GyverMatrixWiFi), где меня поразила простата и взял его к себе в проект часов. Со временем код раздувался, страдал гигантизмом и опять усыхал, выбрасывая всё, что оказалось не нужно. В результате получился маленький и компактный метод.

```cpp
// объявление таймера с указанием интервала
timerMinim(uint32_t interval=60000)

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

### timerMinimT.hpp

Всё аналогично timerMinim.h, кроме того, что нужно при создании передавать функцию, котороая считает тики. Подробнее в examples

```cpp
// пример инициализации
timerMinimT<my_time_func> timer;
```

### fletcherChecksum.h

Расчёт контрольной суммы. На входе всегда массив байт и его размер. На выходе контрольная сумма

```cpp
// Расчёт контрольно суммы по формуле Flatcher8
uint8_t fletcher8(uint8_t *data, uint16_t len)

// Расчёт контрольно суммы по формуле Fletcher16
uint16_t fletcher16(uint8_t *data, size_t len)
```

Пример:

```cpp
#include fletcherChecksum.h

struct some_data {
    int a = 1;
    char str[50] = "hello world!";
} data;

// какая-то функция, где надо подсчитать контрольную сумму
void some_func() {
    fletcherChecksum calc;
    int sum = calc.fletcher16((uint8_t*)&data, sizeof(some_data));
    Serial.print(sum);
}
```

### stringConverters.h

Кодирование и декодирование строк в форматы для URL и JSON

Все функции сделаны по одной схеме:
- Базовая функция, которая принимает готовый буфер для преобразования и его размер. Это полезно для больших строк или если строк много и используется один буфер для уменьшения фрагментации памяти
- Функция которая проше, она сама выделяет буфер в стеке примерно нужного размера. Если буфер оказался меньше, чем надо, то часть строки просто потеряется.
- Ещё более простая функция, которая просто ссылается на функцию выделения памяти.

Так как память выделяется статическим буфером в стеке, то может происходить его переполнение и перезагрузка микроконтроллера. Размер стека для esp8266 4kB, но можно относительно безопасно использовать только 2kB. Для esp32 всё в два раза больше. Если плата перезагружается, то можно вызвать конструктор с указанием максимального размера. Функции сами выделят память которая по их мнению им надо, но не больше разрешенного. Если нужны большие размеры строк, то надо самостоятельно выделить память через malloc/free и указать функции этот буфер. Но обычно размера в стеке достаточно.

```cpp
// конструктор с возможным указанием допустимого размера буфера
stringConverters(size_t max_size=2000)

// кодирование строки для GET запросов
const char* urlEncode(char* buf, const char* str, size_t max_length, bool params = false)
String urlEncode(const char* str, bool params = false)
String urlEncode(const String &str, bool params = false)

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

### webServerUtils.h

Набор фукции для упрощения организации http сервера.

Это именно набор отдельных типовых функций, а не готовое решение. Они решают две задачи:
- отдача статических файлов с кешированием в броузере
- перевод параметров в переменные для упрощения обработки форм.

Отдача статических файлов хорошо везде описана и любой ИИ вам изобразит готовую функцию. Моя функция просто немного оптимизирована и обкатана.

Обработка параметров тоже не срывает звёз с неба, но позволяет унифицировать обработку форм:
- подогнать полученные параметры под допустимый диапазон
- понять, что какой-то параметр изменился и надо выполнить како-то действие
- понять, что есть изменение в параметрах и надо записать изменения.

```cpp
// конструктор с указанием, какой именно сервер используется. Поддерживается WebServer для esp32 и ESP8266WebServer для esp8266
webServerUtils(Server& srv)

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
```