# Description

A set of small functions for implementing typical algorithms in Arduino.

Good practices are passed around from project to project. I simply copy them. But what if we collected them all in one library and simply connected them instead of copying them? Some functions are trivial, some are obvious, some are strange, but they all solve a problem. The library will gradually grow.

Набор небольших функций для реализации типичных  алгоритмов в Arduino.

Хорошие наработки гуляют из проекта в проект. Я их просто копирую. Но если их все собрать в одну библиотеку и просто подключать, а не копировать? Некоторые функции банальны, некоторые очевидны, некоторые странные, но все они решают какую-то задачу. Библиотека потихоньку будет наполняться.

## Table of contents ([ru](#оглавление))

- How to install
- How to use

- The simplest timer for organizing delays instead of delay()

## Оглавление

- Как установить
- Как пользоваться

- [простейший таймер для организации задержек вместо delay()](#timerminimh)

## Description of functions

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

### timerMinimT.h

Всё аналогично timerMinim.h, кроме того, что нужно при создании передавать функцию, котороая считает тики. Подробнее в examples

```cpp
// пример инициализации
timerMinimT<my_time_func> timer;
``` 
