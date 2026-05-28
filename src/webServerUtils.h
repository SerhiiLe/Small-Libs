#ifndef httpArgToVar_h
#define httpArgToVar_h

#include <Arduino.h>

template <typename Server>
class webServerUtils {
public:

// конструктор с указанием, какой именно сервер используется. Поддерживается WebServer для esp32 и ESP8266WebServer для esp8266
webServerUtils(Server& srv) : SRV(srv) {}

// флаг изменения, меняется в true когда новое значение не совпадает со старым
bool need_save = false;

// Отправка файла. Первый параметр - путь к файлу. Второй параметр - функция которая разрешает кеширование для определённых файлов
bool fileSend(const String &path, bool (*cc)(const String &p)=nullptr) {
	bool cache_enable = true;
	// определение типа файла
	const char *ct = nullptr;
	if(path.endsWith(F(".html"))) ct = PSTR("text/html");
	else if(path.endsWith(F(".css"))) ct = PSTR("text/css");
	else if(path.endsWith(F(".js"))) ct = PSTR("application/javascript");
	else if(path.endsWith(F(".json"))) { ct = PSTR("application/json"); cache_enable = cc ? cc(path): false; } 
	else if(path.endsWith(F(".png"))) ct = PSTR("image/png");
	else if(path.endsWith(F(".jpg"))) ct = PSTR("image/jpeg");
	else if(path.endsWith(F(".gif"))) ct = PSTR("image/gif");
	else if(path.endsWith(F(".ico"))) ct = PSTR("image/x-icon");
	else { ct = PSTR("text/plain"); cache_enable = cc ? cc(path): false; }
	// открытие файла на чтение
	if(!fs_isStarted) {
		// файловая система не загружена, переход на страничку обновления
		SRV.client().printf_P(PSTR("HTTP/1.1 200\r\nContent-Type: %s\r\nContent-Length: 80\r\nConnection: close\r\n\r\n<html><body><h1><a href='/update'>File system not exist!</a></h1></body></html>"),ct);
		return true;
	}
	if(LittleFS.exists(path)) {
		File file = LittleFS.open(path, "r");
#ifdef ESP32
		// Для ESP32 это работает немного быстрее, но занимает на 804 байта больше.
		if (cache_enable)
			SRV.sendHeader("Cache-Control", "public, max-age=3600, immutable");
		SRV.streamFile(file, ct);
#else
		// файл существует и открыт, выделение буфера передачи и отсылка заголовка
		char buf[1476];
		size_t sent = 0;
		int siz = file.size();
		SRV.client().printf_P(PSTR("HTTP/1.1 200\r\nContent-Type: %s\r\n"), ct);
		if (cache_enable)
			SRV.client().printf_P(PSTR("Cache-Control: public, max-age=%u, immutable\r\n"), cache_live);
		SRV.client().printf_P(PSTR("Content-Length: %d\r\nConnection: close\r\n\r\n"), siz);
		// отсылка файла порциями, по размеру буфера или остаток
		while(siz > 0) {
			size_t len = std::min((int)(sizeof(buf) - 1), siz);
			file.read((uint8_t *)buf, len);
			SRV.client().write((const char*)buf, len);
			siz -= len;
			sent+=len;
		}
#endif
		file.close();
	} else return false; // файла нет, ошибка
	return true;
}

/****** шаблоны простых операций для выделения переменных из web ******/

// определение выбран checkbox или нет
template <typename F>
bool checkbox(F *name, uint8_t &var) {
	if( SRV.hasArg(name) ) {
		if( var == 0 ) {
			var = 1;
			need_save = true;
			return true;
		}
	} else {
		if( var > 0 ) {
			var = 0;
			need_save = true;
			return true;
		}
	}
	return false;
}

// определение простых целых чисел
template <typename F, typename T>
bool to_int(F *name, T &var, long from, long to) {
	if( SRV.hasArg(name) ) {
		if( SRV.arg(name).toInt() != (long)var ) {
			var = constrain(SRV.arg(name).toInt(), from, to);
			need_save = true;
			return true;
		}
	}
	return false;
}

// определение дробных чисел
template <typename F>
bool to_float(F *name, float &var, float from, float to, float prec=8.0f) {
	if( SRV.hasArg(name) ) {
		if( round(SRV.arg(name).toFloat()*pow(10.0f,prec)) != round(var*pow(10.0f,prec)) ) {
			var = constrain(SRV.arg(name).toFloat(), from, to);
			need_save = true;
			return true;
		}
	}
	return false;
}

// определение строк (для String)
template <typename F>
bool to_string(F *name, String &var) {
	if( SRV.hasArg(name) ) {
		if( SRV.arg(name) != var ) {
			var = SRV.arg(name);
			need_save = true;
			return true;
		}
	}
	return false;
}

// определение простых строк (для char[])
template <typename F>
bool to_string(F *name, char * var, size_t len) {
	if( SRV.hasArg(name) ) {
		if( strcmp(SRV.arg(name).c_str(), var) != 0 ) {
			strncpy_P(var, SRV.arg(name).c_str(), len);
			var[len] = 0;
			need_save = true;
			return true;
		}
	}
	return false;
}

// определение времени
template <typename F>
bool time(F *name, uint16_t &var) {
	if( SRV.hasArg(name) ) {
		if( decode_time(SRV.arg(name)) != var ) {
			var = decode_time(SRV.arg(name));
			need_save = true;
			return true;
		}
	}
	return false;
}

// установка другого времени жизни кэша в броузере, в секундах
void setCacheLive(uint16_t cache_life_time) {
	cache_live = cache_life_time;
}

private:

// декодирование времени, заданного в поле input->time (HH:MM)
uint16_t decode_time(String s) {
	// выделение часов и минут из строки вида 00:00
	size_t pos = s.indexOf(":");
	uint8_t h = constrain(s.toInt(), 0, 23);
	uint8_t m = constrain(s.substring(pos+1).toInt(), 0, 59);
	return h*60 + m;
}

Server& SRV; // ссылка на сервер
uint16_t cache_live = 3600;  // время жизни кэша

}; // конец class webServerUtils

#endif