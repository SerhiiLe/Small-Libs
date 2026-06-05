/*
  Blink
  The example uses the built-in LED and the LED on pin 3 (PWM).
  The built-in LED will blink like a normal blink.
  The PWM LED will change brightness slowly at first, then more quickly. The serial line will indicate that the callback has been called.

  Мигалка.
  В примере задействован встроенный светодиод и светодиод на ножке 3 (PWM).
  Встроенный светодиод будет моргать как обычный blink.
  Светодиод на PWM будет сначала менять яркость редко, а потом чаще. В serial будет писаться о вызове callback.
*/

#include <BlinkMinim.h>

BlinkMinim blink_simple(LED_BUILTIN);
BlinkMinim blink_complex;

#define PIN_PWM 3

// callback. Всегда void и без параметров
// callback. Always void and without parameters
void some_function() {
  Serial.println(F("callback run"));
  // и опять запустить отсчёт
  blink_complex.blink(ON, 1000, 5, 200, some_function);
}

// переменная хранящяя состояние виртуального пина
// a variable storing the state of a virtual pin
int virtual_pin_state = 0;

// альтернативный digitalWrite
// alternative digitalWrite
void my_write(uint8_t pin, uint8_t val) {
  virtual_pin_state = val;
  if (virtual_pin_state) analogWrite(pin, 200);
  else analogWrite(pin, 50);
}

// альтернативный digitalRead
// alternative digitalRead
int my_read(uint8_t pin) {
  return virtual_pin_state;
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("Starting..."));

  // запуск моргалки с интервалом в одну секунду. Классический блинк.
  // Blinking light every second. Classic blink.
  blink_simple.blink(ON, 1000);

  // можно просто выключить пин, независимо от текущего состояния. По сути тот-же digitalWrite, но не надо указывать пин
  // You can simply turn off the pin, regardless of its current state. It's essentially the same as digitalWrite, but you don't need to specify the pin.
  blink_simple.set(OFF);

  // и включить. ON и OFF могут быть как низким, так и высоким уровнем, задаётся при настройке объекта
  // and turn on. ON and OFF can be either low or high levels, and are set when configuring the object.
  blink_simple.set(ON);

  // disable blink
  // blink_simple.blink(OFF); 

  // инициализация blink с виртуальными функциями вместо стандартных. Могут содержать сложную логику, например управление каким-то мультиплексором.
  // метод begin() можно вызывать много раз, меняя логику работы объекта.
  // Initializing blink with virtual functions instead of standard ones. They can contain complex logic, such as controlling a multiplexer.
  // The begin() method can be called multiple times, changing the object's logic.
  blink_complex.begin(PIN_PWM, HIGH, my_write, my_read);

  // запуск блинка на моргание каждую две секунды, но "зажигать" только на пол секунды, после 5 морганий вызвать функцию some_function
  // Set the blink to blink every two seconds, but only for half a second. After 5 blinks, call the some_function function.
  blink_complex.blink(ON, 2000, 5, 500, some_function);
}

void loop() {
  // функция не асинхронная, надо вызывать tick() в loop()
  // The function is not asynchronous, you need to call tick() in loop()
  blink_simple.tick();

  // для каждого объекта blink по отдельности, так как каждый объект полностью самостоятелен
  // for each blink object separately, since each object is completely independent
  blink_complex.tick();
}
