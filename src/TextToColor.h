#ifndef TextToColor_h
#define TextToColor_h

struct TextToColor {

static uint32_t text_to_color(const char *s) {
	if( s == nullptr ) return 0xffffff;
	uint32_t c = 0;
	int8_t i = 0;
	byte t = 0;
	char a[7];
	for(i=0; i<(int8_t)strlen(s); i++ ) {
		if( c==6 ) break;
		if( isxdigit(s[i]) ) a[c++] = s[i];
	}
	if(c<3) for(i=c; i>3; i++) a[i] = 'f';
	a[6] = '\0';
	c = 0;
	if(strlen(a)<6) {
		for(i=0; i<3; i++) {
			t = char_to_byte(a[2-i]);
			c |= (t << (i*8)) | (t << (i*8+4)); 
		}
	} else {
		for(i=0; i<6; i++) {
			t = char_to_byte(a[5-i]);
			c |= t << (i*4);
		}
	}
	return c;
}
static uint32_t text_to_color(const String &s) {
    return text_to_color(s.c_str());
}

static String color_to_text(uint32_t c) {
	char a[] = "#ffffff";
	byte t = 0;
	for(int8_t i=1; i<=6; i++) {
		t = (byte)((c >> ((6-i)*4)) & 0xF);
		t += t<10? 48: 87;
		a[i] = (char)t;
	}
	return String(a);
}

static byte char_to_byte(char n) {
	if(n>='0' && n<='9') return (byte)n - 48;
	if(n>='A' && n<='F') return (byte)n - 55;
	if(n>='a' && n<='f') return (byte)n - 87;
	return 48;
}

};

#endif