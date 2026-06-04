#ifndef StringConvertersC_h
#define StringConvertersC_h

#include <Arduino.h>

/*
Оригинальный вариант с выделением буфера в стеке

Эти переходы между различными кодировками немного задалбливают. На языках более высокого уровня обычно этого даже не замечаешь, а здесь функции длинной в километр.
*/

#ifndef SC_SET_MAX_BUFFER_SIZE
#define SC_SET_MAX_BUFFER_SIZE 2000
#endif

class StringConvertersC {

public:

// string encoding for GET requests
static const char* urlEncode(char* buf, const char* str, size_t max_length, bool params = false) {
	size_t i = 0, length = strlen(str);
	char c, t, *p = buf, *last = buf + max_length -1; // -1 На завершающий ноль

	while( i < length && p < last) {
		c = str[i++];
		if (params && (c == '&' || c == '=')) { // не экранировать эти спец-символы, для передачи готовой строки параметров
			*p++ = c;
		} else if (isalnum((byte)c) || c == '-' || c == '_' || c == '.' || c == '~') {
			*p++ = c;
		} else {
			if (p+2 > last) break;
			*p++ = '%';
			print_byte(p, c);
		}
	}
	*p = 0;
	return buf;
}
// string encoding for GET requests
static String urlEncode(const char* str, bool params = false) {
	size_t len = strlen(str) * 3 + 10;
	if (len > DEFAULT_MAX_SIZE) len = DEFAULT_MAX_SIZE; // обрезание буфера, не вся строка может поместиться, она будет обрезана
	char buf[len];
	// Для больших строк надо играть с malloc/free, но это потенциальная проблема фрагментирования памяти и для маленьких строк избыточно
	// Если сильно надо большие строки, то лучше заранее выделить большой буфер в вызывающей функции и передавать этот буфер в базовую функцию
	// char* buf = (char*) malloc(len * sizeof(char));
	// free(buf);
	return String(urlEncode(buf, str, len, params));
}
// string encoding for GET requests
static String urlEncode(const String &str, bool params = false) {
	return urlEncode(str.c_str(), params);
}

// Simple JSON escaping without encoding conversion
static const char* urlDecode(char* buf, const char* str, size_t max_length) {
	size_t i = 0, length = strlen(str);
	char c, t, *p = buf, *last = buf + max_length -1; // -1 На завершающий ноль
	char* error; // указатель на символ который не является шестнадцатеричным числом.
	char code[] = "0x00"; // буфер в котором будем создавать число по формату функции strtol 0xAB

	while( i < length && p < last) {
		c = str[i++];
		if (c == '+') {
			*p++ = ' ';
		} else if (c == '%') {
			if (i+2 > length) break;
			code[2] = str[i++];
			code[3] = str[i++];
			*p++ = (char)strtol(code, &error, 16);
		} else {
			*p++ = c;
		}
	}
	*p = 0;
	return buf;
}
// Simple JSON escaping without encoding conversion
static String urlDecode(const char* str) {
	size_t len = strlen(str) +1;
	if (len > DEFAULT_MAX_SIZE) len = DEFAULT_MAX_SIZE; // временный массив в стеке
	char buf[len];
	return String(urlDecode(buf,str,len));
}
// Simple JSON escaping without encoding conversion
static String urlDecode(const String &str) {
	return String(urlDecode(str.c_str()));
}

// Simple JSON escaping without encoding conversion
static const char* jsonEscape(char *buf, const char *str, size_t max_length) {
	size_t i = 0, length = strlen(str);
	byte c;
	char t, *p = buf, *last = buf + max_length -1; // -1 На завершающий ноль

	while( i < length && p < last) {
		// поиск символов, которые надо экранировать. Если не надо, то просто копирование
    	c = (byte)str[i++]; // получение очередного символа и сразу увеличение для следующего цикла

		// стандартные управляющие символы с буквенным аналогом
		t = is_letter_esc(c);
		if (c < 32 || t) {
			if (p+1 > last) break;
			// Управляющие символы ASCII (0x00 - 0x1F), у которых нет буквенного аналога и с буквенным индексом
			*p++ = '\\'; // общий эскейп символ для все последовательностей
			if (t) *p++ = t; // буквенный сивол
			else { // числовая последовательность
				if (p+5 > last) break;
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
// Simple JSON escaping without encoding conversion
static String jsonEscape(const char* str) {
	size_t len = (strlen(str) * 5) /4 + 10; // думаю буфера +25% достаточно
	if (len > DEFAULT_MAX_SIZE) len = DEFAULT_MAX_SIZE; // обрезание буфера, не вся строка может поместиться, она будет обрезана
	char buf[len];
	return String(jsonEscape(buf, str, len));
}
// Simple JSON escaping without encoding conversion
static String jsonEscape(const String& str) {
	return jsonEscape(str.c_str());
}

// Converting a JSON string from UTF-8 to UTF-16 like \uABCD
static const char* jsonEncode(char* buf, const char *str, size_t max_length) {
	size_t i = 0, length = strlen(str);
	byte c; 
	char t, *p = buf, *last = buf + max_length -1;

	while( i < length && p < last) {
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
				if (i+1 > length) break;
		        uint16_t cc = (uint16_t)(str[i-1] & 0x1F) << 6;
				cc |= (uint16_t)(str[i++] & 0x3F);
				print_byte(p, cc>>8);
				print_byte(p, cc&0xff);
			} else if( c >> 4 == 14 ) {
				if (i+2 > length) break;
				uint16_t cc = (uint16_t)(str[i-1] & 0x0F) << 12;
				cc |= (uint16_t)(str[i++] & 0x3F) << 6;
				cc |= (uint16_t)(str[i++] & 0x3F);
				print_byte(p, cc>>8);
				print_byte(p, cc&0xff);
			} else if( c >> 3 == 30 ) {
				if (i+3 > length) break;
				uint32_t CP = ((uint32_t)(str[i-1] & 0x07) << 18);
				CP |= ((uint32_t)(str[i++] & 0x3F) << 12);
				CP |= ((uint32_t)(str[i++] & 0x3F) << 6);
				CP |= (uint32_t)(str[i++] & 0x3F);
				CP -= 0x10000;
				uint16_t cc = 0xD800 + (uint16_t)((CP >> 10) & 0x3FF);
				print_byte(p, cc>>8);
				print_byte(p, cc&0xff);
				cc = 0xDC00 + (uint16_t)(CP & 0x3FF);
				*p++ = '\\';
				*p++ = 'u';
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
// Converting a JSON string from UTF-8 to UTF-16 like \uABCD
static String jsonEncode(const char* str) {
	size_t len = strlen(str)*3 + 24;
	if (len > DEFAULT_MAX_SIZE) len = DEFAULT_MAX_SIZE; // обрезание буфера, не вся строка может поместиться, она будет обрезана
	char buf[len];
	return String(jsonEncode(buf, str, len));
}
// Converting a JSON string from UTF-8 to UTF-16 like \uABCD
static String jsonEncode(const String &str) {
	return jsonEncode(str.c_str());
}

// JSON converter from utf16 \uABCD to utf8 text
static const char* jsonDecode(char* buf, const char* str, size_t max_length) {
	size_t i = 0, length = strlen(str);
	char c, t, *p = buf, *last = buf + max_length -1;
	char* error; // указатель на символ который не является шестнадцатеричным числом.
	char unicode[] = "0x0000"; // буфер в котором будем создавать число по формату функции strtol 0xABCD

	while( i < length && p < last) {
		c = str[i++];
		if (c == '\\') { // если найден esc символ, то приступаем
			c = str[i++];
			if (c == 'u') { // о, да это же похоже на utf16
				// выборка из 4х последовательных символов, чтобы получить формат 0xABCD (16 бит)
				if (i+3 > length) break; // входная строка внезапно оборвалась :(
				for (int j = 2; j < 6; j++){
					unicode[j] = str[i++];
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
					if (i+5 > length) break; // входная строка внезапно оборвалась :(
					i += 2; // пропуск пары \u
					for (int j = 2; j < 6; j++){
						unicode[j] = str[i++];
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
				} else { //if (codepoint <= 0x10FFFF) {
					*p++ = ((codepoint >> 18) & 0x07) | 0xF0;
					*p++ = ((codepoint >> 12) & 0x3F) | 0x80;
					*p++ = ((codepoint >> 6) & 0x3F) | 0x80;
					*p++ = ((codepoint) & 0x3F) | 0x80;
				}
			// Кроме непосредственно utf16 могут быть другие символы, которые должны быть экранированы в json
			} else if (c == 0) break;
			else if (c == 'n') *p++ = '\n';
			else if (c == 'r') *p++ = '\r';
			else if (c == 't') *p++ = '\t';
			else if (c == 'b') *p++ = '\b';
			else if (c == 'f') *p++ = '\f';
			else *p++ = c;
		} else {
			// обычные символы, в том числе из json в utf8
			*p++ = c;
		}
	}
	*p = 0;
	return buf;
}
// JSON converter from utf16 \uABCD to utf8 text
static String jsonDecode(const char* str) {
	size_t len = strlen(str) +1;
	if (len > DEFAULT_MAX_SIZE) len = DEFAULT_MAX_SIZE; // временный массив в стеке
	char buf[len];
	return String(jsonDecode(buf, str, len));
}
// JSON converter from utf16 \uABCD to utf8 text
static String jsonDecode(const String &str) {
	return jsonDecode(str.c_str());
}

private:

// печать байта в виде шестнадцатеричного числа
static void print_byte(char* &buf, byte c) {
	byte t = c >> 4;
	*buf++ = t > 9 ? t - 10 + 'A': t + '0';
	t = c & 0xf;
	*buf++ = t > 9 ? t - 10 + 'A': t + '0'; 
}

// стандартные управляющие символы с буквенным аналогом
static char is_letter_esc(char c) {
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

// статический дефолтный размер буфера. Буфер на стеке, лучше не превышать 2к. Для ESP32 можно 4000.
static const size_t DEFAULT_MAX_SIZE = SC_SET_MAX_BUFFER_SIZE;

}; // конец class StringConverters

#endif