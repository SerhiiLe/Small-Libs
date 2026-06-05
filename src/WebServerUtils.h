#ifndef WebServerUtils_h
#define WebServerUtils_h

#include <Arduino.h>

#include "StringConverters.h"

template <typename Server>
class WebServerUtils {
public:

// A constructor specifying which server is used. WebServer for ESP32 and ESP8266 is supported.
WebServerUtils(Server& srv) : SRV(srv) {}

// change flag, changes to true when the new value does not match the old one
bool need_save = false;

// Sending a file. The first parameter is the file path. The second parameter is a function that enables caching for specific files.
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

// determining whether a checkbox is selected or not
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

// definition of integers
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

// definition of floating-point numbers
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

// string definition (for String)
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

// string definition (for char[])
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

// time definition, HH:MM to uint16_t
template <typename F>
bool time(F *name, uint16_t &var) {
	if( SRV.hasArg(name) ) {
		if( SmallConverters::decode_time(SRV.arg(name)) != var ) {
			var = SmallConverters::decode_time(SRV.arg(name));
			need_save = true;
			return true;
		}
	}
	return false;
}

// storing the color from the string #RRGGBB in a uint32_t variable
template <typename F>
bool color(F *name, uint32_t &var) {
	if( SRV.hasArg(name) ) {
		if( SmallConverters::text_to_color(SRV.arg(name).c_str()) != var ) {
			var = SmallConverters::text_to_color(SRV.arg(name).c_str());
			need_save = true;
			return true;
		}
	}
	return false;
}

// Sets the browser cache lifetime in seconds, defaulting to one hour = 3600 seconds
void setCacheLive(uint16_t cache_life_time) {
	cache_live = constrain(cache_life_time, 0, 86400);
}

private:

Server& SRV; // ссылка на сервер
uint32_t cache_live = 3600;  // время жизни кэша

}; // конец class webServerUtils

#endif