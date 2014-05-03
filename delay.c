#include "delay.h"

void delay_us(unsigned short i)
{
	while(i--)
	{
		_delay_cycles(16);
	}
}

void delay_ms(unsigned short i)
{
    while(i--)
    {
    	delay_us(1000);
    }
	return;
}
