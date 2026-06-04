#ifndef TimerMinimT_hpp
#define TimerMinimT_hpp

/*
	Таймер для переодически повторяющихся операций.
	Создаётся с именем функции которая должна возвращать тики. Это позволяет использовать на не Arduino платформах

	Отсчёт каждого цикла начинается с момента опроса готовности таймера, если он к этому времени сработал.
	Это приводит к тому, что периоды не равномерны, время каждого цикла не может быть меньше заданного,
	но может быть намного больше заданного. Таким образом таймер не отсчитывает равные промежутки,
	а скорее гарантирует пропуск заданного времени для приписанной к таймеру операции.

	В фоне ничего не считается, ресурсов не потребляет, все расчёты только в момент вызова isReady()

	Для разовых ожиданий проще использовать millis() или его аналог напрямую.
*/

template <unsigned long (*TimeFunc)()>
class TimerMinimT
{
	public:
		// timer declaration with interval specification
		TimerMinimT(unsigned long interval=60000) {
			setInterval(interval);
		}
		// setting the timer interval and resetting the timer
		void setInterval(unsigned long interval) {
			// хотя-бы одна миллисекунда, для приличия
			_interval = interval ? interval: 1;
			reset();
		}
		// returns true when the time has come
		bool isReady() {
			if ((long)(TimeFunc() - _next) > 0) {
				reset();
				return true;
			}
			return false;
		}
		// manual reset of the timer, the countdown will start from the beginning
		void reset() {
			_next = TimeFunc() + _interval;
		}
		// set a delay until the next triggering, or duplicates reset() if the delay is 0
		void setNext(unsigned long next = 0) {
			_next = TimeFunc() + (next ? next: _interval);
		}

	private:
		unsigned long _interval = 0;
		unsigned long _next = 0;
};

#endif
