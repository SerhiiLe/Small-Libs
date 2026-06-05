/*
  Пример использования таймера на шаблонах
*/

#include <TimerMinimT.hpp>

// функция отсчёта времени. Должна быть объявлена до создания объекта, иначе компилятор не поймёт шаблон.
uint32_t my_time_func() {
  return millis()>>2; // для примера просто уменьшается в четыре раза millis()
}

TimerMinimT<micros> timer1;               // Инициализация без параметров, интервал устанавливается потом или по умолчанию 60000
TimerMinimT<millis> timer2 = 1000;        // Формат инициализации с одновременным заданием интервала через =
TimerMinimT<my_time_func> timer3(500);    // Формат инициализации с одновременным заданием интервала внутри ()

// Пример абстрактной структуры с какими-то данными и таймером
struct My_Data {
  int i = 0;
  TimerMinimT<millis> timer; // сам таймер
};

My_Data myData[5]; // массив структур

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  myData[3].timer.setInterval(2 * 1000); // инициализация интервала таймера внутри структуры, обращение по индексу
}

void loop() {
  static int i1 = 0, i2 = 0, i3 = 0, i4 = 0;
  if (timer1.isReady()) {           // Срабатываение timer1
    Serial.print("timer1: ");
    Serial.println(i1++);
    // digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }

  if (timer2.isReady()) {           // Срабатываение timer2
    Serial.print("timer2: ");
    Serial.println(i2++);
    // digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }

  if (timer3.isReady()) {           // Срабатываение timer3
    Serial.print("timer3: ");
    Serial.println(i3++);
    // digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }

  if (myData[3].timer.isReady()) {  // Срабатываение timer внутри timers[3]
    Serial.print("timers[3]: ");
    Serial.println(i4++);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
}

/*
Варианты функций отсчёта времени для других платформ:

С STM32 HAL:

uint32_t getTimeMs() {
    return HAL_GetTick();
}

timerMinimT<getTimeMs> timer;

Bare metal (SysTick):

volatile uint32_t g_ms = 0;

extern "C" void SysTick_Handler() {
    g_ms++;
}

uint32_t nowMS() {
  return g_ms;
}

timerMinimT<nowMS> timer;

Вариант для FreeRTOS:

#include "FreeRTOS.h"
#include "task.h"

struct RtosTime {
    static uint32_t now() {
        return xTaskGetTickCount() * portTICK_PERIOD_MS;
    }
};

timerMinimT<RtosTime> timer;

*/

