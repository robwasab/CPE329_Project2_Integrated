#include "print.h"
#include"lcd.h"

#define PRINT_STRING(x) lcd_print(x)
#define PUTC(x) lcd_put_char(x)

void print_nibble(uint8_t b)
{
	b &= 0x0f;
	if (b < 0x0a)
	{
		PUTC(0x30 | b);
	}
	else
	{
		b -= 9;
		PUTC(0x40 | b);
	}
}

void print_int(void * foo, uint8_t bytes)
{
	uint8_t * b = (uint8_t *) foo;
    bytes -= 1;
	PRINT_STRING("0x");
	do {
	   print_nibble(b[bytes] >> 4);
	   print_nibble(b[bytes]);
	}
	while (bytes--);
}
