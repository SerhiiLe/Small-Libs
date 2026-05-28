#ifndef stringConverters_h
#define stringConverters_h

#include <Arduino.h>

/*
Эти переходы между различными кодировками немного задалбливают. На языках более высокого уровня обычно этого даже не замечаешь, а здесь функции длинной в километр.
*/

class stringConverters {

public:

// конструктор с возможным указанием допустимого размера буфера
stringConverters(size_t max_size=2000) : _maxSize(max_size) {}

// кодирование строки для GET запросов
const char* urlEncode(char* buf, const char* str, size_t max_length, bool params = false) {
	size_t i = 0;
	byte c; 
	char t, *p = buf, *last = buf + max_length - 3; // 10 это максимальное значение которое может быть добавлено %XX

	while( str[i] != '\0' && p < last) {
		c = (byte)str[i++];
		if(params && (c == '&' || c == '=')) { // не экранировать эти спец-символы, для передачи готовой строки параметров
			*p++ = c;
		} else
		if(isalnum(c)) {
			*p++ = c;
		} else {
			*p++ = '%';
			print_byte(p, c);
		}
	}
	*p = 0;
	return buf;
}
// кодирование строки для GET запросов
String urlEncode(const char* str, bool params = false) {
	size_t len = strlen(str) * 3 + 10;
	if (len > _maxSize) len = _maxSize; // буфер на стеке, лучше не превышать 2к. Для ESP32 можно 4000
	char buf[len+1];
	// Для больших строк надо играть с malloc/free, но это потенциальная проблема фрагментирования памяти и для маленьких строк избыточно
	// Если сильно надо большие строки, то лучше заранее выделить большой буфер в вызывающей функции и передавать этот буфер в базовую функцию
	// char* buf = (char*) malloc(len * sizeof(char));
	// free(buf);
	return String(urlEncode(buf, str, len));
}
// кодирование строки для GET запросов
String urlEncode(const String &str, bool params = false) {
	return urlEncode(str.c_str(), params);
}

// Простое экранирование для json без конвертации кодировки
const char* jsonEscape(char *buf, const char *str, size_t max_length) {
	size_t i = 0;
	byte c;
	char t, *p = buf, *last = buf + max_length - 6; // 6 это максимальное значение которое может быть добавлено \u0011

	while( str[i] != '\0' && p < last) {
		// поиск символов, которые надо экранировать. Если не надо, то просто копирование
    	c = (byte)str[i++]; // получение очередного символа и сразу увеличение для следующего цикла

		// стандартные управляющие символы с буквенным аналогом
		t = is_letter_esc(c);
		if (c < 32 || t) {
			// Управляющие символы ASCII (0x00 - 0x1F), у которых нет буквенного аналога и с буквенным индексом
			*p++ = '\\'; // общий эскейп символ для все последовательностей
			if (t) *p++ = t; // буквенный сивол
			else { // числовая последовательность
				*p++ = 'u';
				print_byte(p, 0);
				print_byte(p, c);
			}
		} else {
			// Все остальные символы, включая любые байты UTF-8 (c < 0 или c >= 32),
			// просто копируются как есть.
			*p++ = c;
		}
	}

	*p = 0;
	return buf;
}
// Простое экранирование для json без конвертации кодировки
String jsonEscape(const char* str) {
	size_t len = (strlen(str) * 5) /4 + 10; // думаю буфера +25% достаточно, если кочечно строка не состоит из одних только кавычек, тогда проблема.
	if (len > _maxSize) len = _maxSize; // буфер на стеке, лучше не превышать 2к. Для ESP32 можно 4000
	char buf[len+1];
	return String(jsonEscape(buf, str, len));
}
// Простое экранирование для json без конвертации кодировки
String jsonEscape(const String& str) {
	return jsonEscape(str.c_str());
}

// Ковертация строки для json, из utf8 в utf16 вида \uABCD
const char* jsonEncode(char* buf, const char *str, size_t max_length) {
	size_t i = 0;
	byte c; 
	char t, *p = buf, *last = buf + max_length - 10; // 10 это максимальное значение которое может быть добавлено \u00112233

	while( str[i] != '\0' && p < last) {
		// Выделение символа UTF-8 и перевод его в UTF-16 для вывода в JSON
		// 0xxxxxxx - 7 бит 1 байт, 110xxxxx - 10 бит 2 байта, 1110xxxx - 16 бит 3 байта, 11110xxx - 21 бит 4 байта
		c = (byte)str[i++];
		t = is_letter_esc(c);
		if ( c > 127 || t || c < 32 ) {
			*p++ = '\\';
			if (t) { // это стандартная бувенная последовательность
				*p++ = t;
				continue;
			}
			*p++ = 'u';
			if (c < 32) { // это управляющий символ без буквенного эквивалента
				print_byte(p, 0);
				print_byte(p, c);
				continue;
			}
			// utf8 -> utf16
			if( c >> 5 == 6 ) {
		        uint16_t cc = ((uint16_t)(str[i-1] & 0x1F) << 6);
				cc |= (uint16_t)(str[i++] & 0x3F);
				print_byte(p, cc>>8);
				print_byte(p, cc&0xff);
			} else if( c >> 4 == 14 ) {
				uint16_t cc = ((uint16_t)(str[i-1] & 0x0F) << 12);
				cc |= ((uint16_t)(str[i++] & 0x3F) << 6);
				cc |= (uint16_t)(str[i++] & 0x3F);
				print_byte(p, cc>>8);
				print_byte(p, cc&0xff);
			} else if( c >> 3 == 30 ) {
				uint32_t CP = ((uint32_t)(str[i-1] & 0x07) << 18);
				CP |= ((uint32_t)(str[i++] & 0x3F) << 12);
				CP |= ((uint32_t)(str[i++] & 0x3F) << 6);
				CP |= (uint32_t)(str[i++] & 0x3F);
				CP -= 0x10000;
				uint16_t cc = 0xD800 + (uint16_t)((CP >> 10) & 0x3FF);
				print_byte(p, cc>>8);
				print_byte(p, cc&0xff);
				cc = 0xDC00 + (uint16_t)(CP & 0x3FF);
				print_byte(p, cc>>8);
				print_byte(p, cc&0xff);
			}
		} else {
			*p++ = c;
		}
	}

	*p = '\0';
	return buf;
}
// Ковертация строки для json, из utf8 в utf16 вида \uABCD
String jsonEncode(const char* str) {
	size_t len = strlen(str)*3 + 10;
	if (len > _maxSize) len = _maxSize; // буфер на стеке, лучше не превышать 2к. Для ESP32 можно 4000
	char buf[len+1];
	return String(jsonEncode(buf, str, len));
}
// Ковертация строки для json, из utf8 в utf16 вида \uABCD
String jsonEncode(const String &str) {
	return jsonEncode(str.c_str());
}

// Конвертер из json в utf16 вида \uABCD в текст utf8
const char* jsonDecode(char* buf, const char* str, size_t max_length) {
	size_t i = 0;
	char iChar; 
	char t, *p = buf, *last = buf + max_length; // в данном случае длина может быть только меньше
	char* error; // указатель на символ который не является шестнадцатеричным числом.
	char unicode[6] = "0x"; // буфер в котором будем создавать число по формату функции strtol 0xABCD

	while( str[i] != '\0' && p < last) {
		iChar = str[i++];
		if (iChar == '\\') { // если найден esc символ, то приступаем
			iChar = str[++i];
			if (iChar == 'u') { // о, да это же похоже на utf16
				// выборка из 4х последовательных символов, чтобы получить формат 0xABCD (16 бит)
				for (int j = 2; j < 6; j++){
					iChar = str[++i];
					unicode[j] = iChar;
				}
				long uFirst = strtol(unicode, &error, 16); // первый промежуточный вариант
				if (uFirst < 32) {
					// это управляющий сивол формата \u00XX
					*p++ = (byte)uFirst;
					continue;
				}
				uint32_t codepoint = 0; // выделенный код символа utf16
				// utf16 может быть 16 бит и 32 бита (utf8 может иметь 8, 16, 24, 32 бита)
				if (uFirst <= 0xD7FF) { // это похоже на 16 битный вариант utf16
					codepoint = uFirst;
				} else if (uFirst <= 0xDBFF) { // это похоже на 32 битный вариант utf16
					// надо повторить предыдущий шаг, чтобы получить ещё 16 бит.
					for (int j = 2; j < 6; j++){
						iChar = str[++i];
						unicode[j] = iChar;
					}
					long uSecond = strtol(unicode, &error, 16); // второй промежуточный вариант
					codepoint = (((uFirst - 0xD800) << 10) | (uSecond - 0xDC00)) + 0x10000;
				}
				//-------(2) Codepoint to UTF-8 -------
				if ( codepoint <= 0x007F && codepoint != 0 ) {
					*p++ = (char)codepoint;
				} else if ( codepoint <= 0x07FF ) {
					*p++ = ((codepoint >> 6) & 0x1F) | 0xC0;
					*p++ = (codepoint & 0x3F) | 0x80;
				} else if ( codepoint <= 0xFFFF ) {
					*p++ = ((codepoint >> 12) & 0x0F) | 0xE0;
					*p++ = ((codepoint >> 6) & 0x3F) | 0x80;
					*p++ = ((codepoint) & 0x3F) | 0x80;
				} else if (codepoint <= 0x10FFFF) {
					*p++ = ((codepoint >> 18) & 0x07) | 0xF0;
					*p++ = ((codepoint >> 12) & 0x3F) | 0x80;
					*p++ = ((codepoint >> 6) & 0x3F) | 0x80;
					*p++ = ((codepoint) & 0x3F) | 0x80;
				}
			// Кроме непосредственно utf16 могут быть другие символы, которые должны быть экранированы в json
			} else if (iChar == 'n') *p++ = '\n';
			else if (iChar == 'r') *p++ = '\r';
			else if (iChar == 't') *p++ = '\t';
			else if (iChar == 'b') *p++ = '\b';
			else if (iChar == 'f') *p++ = '\f';
			else *p++ = iChar;
		} else {
			// обычные символы, в том числе из json в utf8
			*p++ = iChar;
		}
	}
	*p = 0;
	return buf;
}
// Конвертер из json, из utf16 вида \uABCD в текст utf8
String jsonDecode(const char* str) {
	int len = strlen(str);
	if (len > _maxSize) len = _maxSize; // временный массив в стеке (4096), по этому с ограничением, для esp8266 лучше не превышать 2к
	char buf[len+1];
	return String(jsonDecode(buf, str, len));
}
// Конвертер из json, из utf16 вида \uABCD в текст utf8
String jsonDecode(const String &str) {
	return jsonDecode(str.c_str());
}

private:

// печать байта в виде шестнадцатеричного числа
void print_byte(char* &buf, byte c) {
	if((c & 0xf) > 9)
		*(buf+1) = (c & 0xf) - 10 + 'A';
	else
		*(buf+1) = (c & 0xf) + '0';
	c = (c>>4) & 0xf;
	if(c > 9)
		*buf = c - 10 + 'A';
	else
		*buf = c+'0';
	buf += 2;
}

// стандартные управляющие символы с буквенным аналогом
char is_letter_esc(char c) {
	char t = 0;

	if (c == '"') 		t = '"';
	else if (c == '\\') t = '\\';
	else if (c == '\n') t = 'n';
	else if (c == '\r') t = 'r';
	else if (c == '\t') t = 't';
	else if (c == '\b') t = 'b';
	else if (c == '\f') t = 'f';
	
	return t;
}

const size_t _maxSize;

}; // конец class stringConverters

#endif