#ifndef StringConverters_h
#define StringConverters_h

#include <Arduino.h>

/*
Вариант с динамическим выделением памяти в куче.
*/

class StringConverters {

public:

	// кодирование строки для GET запросов
	static String urlEncode(const char* str, bool params = false) {
		String out;
		size_t length = strlen(str);
		size_t chunk = length /2;
		if (chunk < 6) chunk = 6; // запас на два символа
		size_t reservedSize = chunk * 3; // Начальный размер в 1.5 раза больше. Реальный размер 1x ... 3x
		if (!reserveMore(out, reservedSize)) return ""; // нет свободной памяти

		for (size_t i = 0; i < length; i++) {
			if (reservedSize - out.length() < 3) { // Если к этому моменту буфер заканчивается, то лучше сделать запас, даже если он не пригодиться
				if (!reserveMore(out, reservedSize, chunk)) break; // память закончилась. Вернуть ту строку, которая сформировалась к этому моменту
			}
			char c = str[i];
			if (params && (c == '&' || c == '=')) { // не экранировать эти спец-символы, для передачи готовой строки параметров
				out += c;
			} else if (isalnum((byte)c) || c == '-' || c == '_' || c == '.' || c == '~') {
				out += c;
			} else {
				out += '%';
				appendHex(out, c);
			}
		}
		return out;
	}
	// кодирование строки для GET запросов
	static String urlEncode(const String &str, bool params = false) {
		return urlEncode(str.c_str(), params);
	}

	// Простое экранирование для json без конвертации кодировки
	static String jsonEscape(const char* str) {
		String out;
		size_t length = strlen(str);
		size_t chunk = length /4;
		if (chunk < 6) chunk = 6; // запас на один символ
		size_t reservedSize = chunk * 5 + 10; // данное кодирование не так сильно раздувает буфер, по этому 1.25 + запас 
		if (!reserveMore(out, reservedSize)) return ""; // нет свободной памяти

		// поиск символов, которые надо экранировать. Если не надо, то просто копирование
		for (size_t i = 0; i < length; i++) {
			if (reservedSize - out.length() < 6) { // Если к этому моменту буфер заканчивается, то лучше сделать запас, даже если он не пригодиться
				if (!reserveMore(out, reservedSize, chunk)) break; // память закончилась. Вернуть ту строку, которая сформировалась к этому моменту
			}
			char c = str[i];
			// стандартные управляющие символы с буквенным аналогом
			char t = is_letter_esc(c);
			if ((byte)c < 32 || t) {
				// Управляющие символы ASCII (0x00 - 0x1F), у которых нет буквенного аналога и с буквенным индексом
				out += '\\'; // общий эскейп символ для все последовательностей
				if (t) out += t; // буквенный сивол
				else { // числовая последовательность
					out += 'u';
					appendHex(out, 0);
					appendHex(out, c);
				}
			} else {
				out += c;
			}
		}
		return out;
	}
	// Простое экранирование для json без конвертации кодировки
	static String jsonEscape(const String& str) {
		return jsonEscape(str.c_str());
	}

	// Ковертация строки для json, из utf8 в utf16 вида \uABCD
	static String jsonEncode(const char* str) {
		String out;
		size_t length = strlen(str);
		size_t chunk = length /2;
		if (chunk < 24) chunk = 24; // запас на два символа
		size_t reservedSize = chunk * 4; // Начальный размер в 2 раза больше. Реальный размер 1x ... 6x
		if (!reserveMore(out, reservedSize)) return ""; // нет свободной памяти

		for (size_t i = 0; i < length; i++) {
			if (reservedSize - out.length() < 12) { // Если к этому моменту буфер заканчивается, то лучше сделать запас, даже если он не пригодиться
				if (!reserveMore(out, reservedSize, chunk)) break; // память закончилась. Вернуть ту строку, которая сформировалась к этому моменту
			}
			byte c = (byte)str[i];
			char t = is_letter_esc(c);
			if ( c > 127 || t || c < 32 ) {
				out += '\\';
				if (t) { // это стандартная бувенная последовательность
					out += t;
					continue;
				}
				out += 'u';
				if (c < 32) { // это управляющий символ без буквенного эквивалента
					appendHex(out, 0);
					appendHex(out, c);
					continue;
				}
				// utf8 -> utf16
				if ((c >> 5) == 0x06) {
					if (i+1 > length) break;
					uint16_t cc = ((uint16_t)(c & 0x1F) << 6) | ((uint16_t)(str[++i] & 0x3F));
					appendHex(out, cc>>8);
					appendHex(out, cc&0xff);
				} else if ((c >> 4) == 0x0E) {
					if (i+2 > length) break;
					uint16_t cc = (uint16_t)(c & 0x0F) << 12;
					cc |= (uint16_t)(str[++i] & 0x3F) << 6;
					cc |= ((uint16_t)(str[++i] & 0x3F));
					appendHex(out, cc>>8);
					appendHex(out, cc&0xff);
				} else if ((c >> 3) == 0x1E) {
					if (i+3 > length) break;
					uint32_t CP = (uint32_t)(c & 0x07) << 18;
					CP |= (uint32_t)(str[++i] & 0x3F) << 12;
					CP |= (uint32_t)(str[++i] & 0x3F) << 6;
					CP |= (uint32_t)(str[++i] & 0x3F);
					CP -= 0x10000;
					uint16_t cc = 0xD800 + (uint16_t)((CP >> 10) & 0x3FF);
					appendHex(out, cc>>8);
					appendHex(out, cc&0xff);
					cc = 0xDC00 + (uint16_t)(CP & 0x3FF);
					out += "\\u";
					appendHex(out, cc>>8);
					appendHex(out, cc&0xff);
				}
			} else {
				out += (char)c;
			}
		}
		return out;
	}
	// Ковертация строки для json, из utf8 в utf16 вида \uABCD
	static String jsonEncode(const String &str) {
		return jsonEncode(str.c_str());
	}

	// Конвертер json из utf16 вида \uABCD в текст utf8
	static String jsonDecode(const char* str) {
		String out;
		size_t length = strlen(str); // выходная строка всегда меньше либо равна исходной
		if (!out.reserve(length)) return ""; // нет свободной памяти
		char* error; // указатель на символ который не является шестнадцатеричным числом.
		char unicode[6] = "0x"; // буфер в котором будем создавать число по формату функции strtol 0xABCD

		for (size_t i = 0; str[i] != '\0'; ++i) {
			char c = str[i];
			if (c != '\\') {
				out += c;
				continue;
			}
			c = str[++i];
			if (c == 'u') { // о, да это же похоже на utf16
				// выборка из 4х последовательных символов, чтобы получить формат 0xABCD (16 бит)
				if (i+4 > length) break; // входная строка внезапно оборвалась :(
				for (int j = 2; j < 6; j++) {
					c = str[++i];
					unicode[j] = c;
				}
				long uFirst = strtol(unicode, &error, 16); // первый промежуточный вариант
				if (uFirst < 32) {
					// это управляющий сивол формата \u00XX
					out += (char)uFirst;
					continue;
				}
				uint32_t codepoint = 0; // выделенный код символа utf16
				// utf16 может быть 16 бит и 32 бита (utf8 может иметь 8, 16, 24, 32 бита)
				if (uFirst <= 0xD7FF) { // это похоже на 16 битный вариант utf16
					codepoint = uFirst;
				} else if (uFirst <= 0xDBFF) { // это похоже на 32 битный вариант utf16
					// надо повторить предыдущий шаг, чтобы получить ещё 16 бит.
					if (i+6 > length) break; // входная строка внезапно оборвалась :(
					i += 2; // пропуск пары \u
					for (int j = 2; j < 6; j++) {
						c = str[++i];
						unicode[j] = c;
					}
					long uSecond = strtol(unicode, &error, 16); // второй промежуточный вариант
					codepoint = (((uFirst - 0xD800) << 10) | (uSecond - 0xDC00)) + 0x10000;
				}
				//-------(2) Codepoint to UTF-8 -------
				if ( codepoint <= 0x007F && codepoint != 0 ) {
					out += (char)codepoint;
				} else if ( codepoint <= 0x07FF ) {
					out += (char)(((codepoint >> 6) & 0x1F) | 0xC0);
					out += (char)((codepoint & 0x3F) | 0x80);
				} else if ( codepoint <= 0xFFFF ) {
					out += (char)(((codepoint >> 12) & 0x0F) | 0xE0);
					out += (char)(((codepoint >> 6) & 0x3F) | 0x80);
					out += (char)(((codepoint) & 0x3F) | 0x80);
				} else { //if (codepoint <= 0x10FFFF) {
					out += (char)(((codepoint >> 18) & 0x07) | 0xF0);
					out += (char)(((codepoint >> 12) & 0x3F) | 0x80);
					out += (char)(((codepoint >> 6) & 0x3F) | 0x80);
					out += (char)(((codepoint) & 0x3F) | 0x80);
				}
			// Кроме непосредственно utf16 могут быть другие символы, которые должны быть экранированы в json
			} else if (c == 0) break;
			else if (c == 'n') out += '\n';
			else if (c == 'r') out += '\r';
			else if (c == 't') out += '\t';
			else if (c == 'b') out += '\b';
			else if (c == 'f') out += '\f';
			else out += c;
		}
		return out;
	}
	// Конвертер json из utf16 вида \uABCD в текст utf8
	static String jsonDecode(const String &str) {
		return jsonDecode(str.c_str());
	}

private:
	// запрос на рсширение буфера строки
	static bool reserveMore(String &out, size_t &reserved, size_t additional = 0) {
		reserved = (out.length() > reserved ? out.length(): reserved) + additional;
		return out.reserve(reserved);
	}

	// печать байта в виде шестнадцатеричного числа
	static void appendHex(String &out, byte c) {
		byte t = c >> 4;
		out += (char)(t > 9 ? t - 10 + 'A': t + '0'); 
		t = c & 0xf;
		out += (char)(t > 9 ? t - 10 + 'A': t + '0'); 
	}

	// стандартные управляющие символы с буквенным аналогом
	static char is_letter_esc(char c) {
		char t = 0;
		if (c == '"') t = '"';
		else if (c == '\\') t = '\\';
		else if (c == '\n') t = 'n';
		else if (c == '\r') t = 'r';
		else if (c == '\t') t = 't';
		else if (c == '\b') t = 'b';
		else if (c == '\f') t = 'f';
		return t;
	}
};

#endif
