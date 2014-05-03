#include <msp430.h>
#include "delay.h"
#include "lcd.h"

#define u8 unsigned char
//unsigned 16 bit
#define u16 unsigned short

void send_nibble(u8 nibble, u8 regsel)
{
	if (regsel) LCD_OUT |= RS;

	else        LCD_OUT &= ~RS;

	LCD_OUT |= E;

    LCD_OUT |= 0x0f & nibble;

    //satisfy E pulse width time of 220nS
    _delay_cycles(4);

    LCD_OUT &= ~E;

    _delay_cycles(2);

    //clear the lower four bits
    LCD_OUT &= ~0x0f;

    //finish waiting a TOTAL enable cycle
    //of 500nS.
    //we've waited 8 * 62.5nS and instruction
    //cycle times, more than enough
    _delay_cycles(2);
}

void send_byte(u8 data, u8 regsel)
{
    //send the high byte first
	send_nibble(0x0f & (data >> 4), regsel);

	//send the low byte second
	send_nibble(0x0f & data, regsel);

	//ps, I know it's redundant to mask out the lower
	//four bits of our data each time I call send_nibble
	//...it's redundant because I mask out the lower four bits
	//anyways inside send_nibble...
	//but, for the sake of readability, that's why I mask
	//them out here!
}

void function_set()
{
	const u8 TWO_LINE = BIT3;
	const u8 DISP_ON = BIT2;

	//send the 0x2 twice
	send_nibble(0x02, CMD);
	send_nibble(0x02, CMD);
	send_nibble(TWO_LINE | DISP_ON, CMD);
	//wait 39_us
	delay_us(40);
}

void display_control()
{
	const u8 DISP_ON = BIT2;
	const u8 CURSOR_ON = BIT1;
	const u8 BLINK_ON = BIT0;
	//you know, hopefully compiler is smart enough to crunch
	//these into a single constant, this is for the sake of
	//readability!
	//I could have said data = 0x0f, but that wouldn't mean much...
	const u8 data = BIT3 | DISP_ON | CURSOR_ON | BLINK_ON;
	send_byte(data, CMD);

	//wait for 39uS
	delay_us(40);
}

void display_clear()
{
	send_byte(BIT0, CMD);
	//wait for 1.53mS!
	delay_us(1600);
}

void display_home()
{
	send_byte(BIT1, CMD);
	//wait 1.53mS!
	delay_us(1600);
}

void display_reset()
{
	display_clear();
	display_home();
}

void cursor_cordinate(u8 row, u8 col)
{
	//the first line goes from 0x00 to 0x27
	//the second line goes from 0x40 to 0x67
	u8 addr = 0;

	if (col >= 16) col = 0;

    if (row >= 2) row = 0;

    if (row == 1) addr += 0x40;

    addr += col;

    addr |= BIT7;

    send_byte(addr, CMD);
    delay_us(40);
}

void entry_mode_set()
{
	const u8 INC = BIT1;
	const u8 SHIFT = BIT0;
	const u8 data = BIT2 | INC;
	send_byte(data, CMD);
	//wait for 39 us
	delay_us(40);
}

void lcd_put_char(char c)
{
	send_byte(c, DATA);
	//wait 43uS
	delay_us(44);
}

void lcd_print(char * str)
{
	u8 i = 0;
	while(str[i] != '\0')
	{
		lcd_put_char(str[i]);
		++i;
	}
}

void write_custom_character(const u8 * array, u8 addr)
{
	//clear lower bytes
	addr &= 0x3f;
	u8 data = BIT6;
	send_byte(data, CMD);

	u8 i;

	for(i = 0; i < 8; ++i)
	{
		send_byte(array[i], DATA);
	}

	//reset the address to DDRAM
	data = BIT7;
	send_byte(data, CMD);
}

void lcd_reset()
{
	P1OUT |= BIT6;
	delay_ms(100);
	P1OUT &= ~BIT6;
}

void lcd_init()
{
	P2OUT = 0x00;
	P1DIR |= BIT6;
	P1OUT &= ~BIT6;
	P2DIR = 0xff;

	lcd_reset();
	delay_ms(20);

	function_set();
	display_control();
	display_clear();
	entry_mode_set();
	display_reset();
	//write_custom_character(smile, 0);
}
