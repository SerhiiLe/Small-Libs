/*
  Пример использования простого таймера
*/

#include <timerMinim.hpp>

timerMinim timer1;         // Инициализация без параметров, интервал устанавливается потом или по умолчанию 60000
timerMinim timer2 = 1000;  // Формат инициализации с одновременным заданием интервала через =
timerMinim timer3(500);    // Формат инициализации с одновременным заданием интервала внутри ()

// Пример абстрактной структуры с какими-то данными и таймером
struct My_Data {
  int i = 0;
  timerMinim timer; // сам таймер
};

My_Data myData[5]; // массив структур

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  myData[3].timer.setInterval(2 * 1000); // инициализация интервала таймера внутри структуры, обращение по индексу
}

void loop() {
  static int i1 = 0, i2 = 0, i3 = 0, i4 = 0;

  if (timer1.isReady()) {           // Срабатываение timer1
    Serial.print("timer1: ");
    Serial.println(i1++);
    timer1.setNext(1500);           // После первого срабатываения через минуту, следующие через 1.5 секунды
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
