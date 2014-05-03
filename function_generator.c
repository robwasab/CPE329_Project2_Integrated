#include <msp430.h> 
#include "function_generator.h"
//
//	Assignment 4 - SPI DAC
//	Raymond Huang, Robby Tong
//	CPE329 - 05/06
//  J. Gerfen
//

void Drive_DAC(unsigned int level);
volatile unsigned int TempDAC_Value = 0;
volatile unsigned int increment_TempDAC = 1;
volatile unsigned int interrupt_count = 0;
volatile unsigned int sinewave_sel = 0;
volatile unsigned short skip_count = 0;
volatile unsigned short duty_toggle = 0;

const unsigned int SineWave[] = {	0x3e8,0x406,0x424,0x442,0x461,0x47e,0x49c,0x4ba,0x4d7,0x4f5,0x512,0x52e,0x54b,0x567,0x582,0x59e,0x5b9,0x5d3,0x5ed,0x607,0x620,0x639,0x651,0x668,0x67f,0x695,0x6ab,0x6c0,0x6d5,0x6e8,0x6fb,0x70d,0x71f,0x730,0x740,0x74f,0x75d,0x76b,0x778,0x784,
									0x78f,0x799,0x7a3,0x7ab,0x7b3,0x7ba,0x7c0,0x7c5,0x7c9,0x7cc,0x7ce,0x7d0,0x7d0,0x7d0,0x7ce,0x7cc,0x7c9,0x7c5,0x7c0,0x7ba,0x7b3,0x7ab,0x7a3,0x799,0x78f,0x784,0x778,0x76b,0x75d,0x74f,0x740,0x730,0x71f,0x70d,0x6fb,0x6e8,0x6d5,0x6c0,0x6ab,0x695,
									0x67f,0x668,0x651,0x639,0x620,0x607,0x5ed,0x5d3,0x5b9,0x59e,0x582,0x567,0x54b,0x52e,0x512,0x4f5,0x4d7,0x4ba,0x49c,0x47e,0x461,0x442,0x424,0x406,0x3e8,0x3ca,0x3ac,0x38e,0x36f,0x352,0x334,0x316,0x2f9,0x2db,0x2be,0x2a2,0x285,0x269,0x24e,0x232,
									0x217,0x1fd,0x1e3,0x1c9,0x1b0,0x197,0x17f,0x168,0x151,0x13b,0x125,0x110,0xfb,0xe8,0xd5,0xc3,0xb1,0xa0,0x90,0x81,0x73,0x65,0x58,0x4c,0x41,0x37,0x2d,0x25,0x1d,0x16,0x10,0xb,0x7,0x4,0x2,0x0,0x0,0x0,0x2,0x4,
									0x7,0xb,0x10,0x16,0x1d,0x25,0x2d,0x37,0x41,0x4c,0x58,0x65,0x73,0x81,0x90,0xa0,0xb1,0xc3,0xd5,0xe8,0xfb,0x110,0x125,0x13b,0x151,0x168,0x17f,0x197,0x1b0,0x1c9,0x1e3,0x1fd,0x217,0x232,0x24e,0x269,0x285,0x2a2,0x2be,0x2db,
									0x2f9,0x316,0x334,0x352,0x36f,0x38e,0x3ac,0x3ca,0x3e8};


//tests
volatile unsigned int frequency = 1;
volatile unsigned int waveform = 1;
volatile unsigned int duty = 5;
volatile unsigned short ccr0_add_value = 0;

void fg_change(unsigned int wf, unsigned int freq, unsigned int dty);

int test_main(void)
{
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	

    //____CPU_CLOCK____//
	if (CALBC1_16MHZ==0xFF)					// If calibration constant erased
    {
		while(1);							// do not load, trap CPU!!
    }
    DCOCTL = 0;								// Select lowest DCOx and MODx settings
    BCSCTL1 = CALBC1_16MHZ;					// Set range
    DCOCTL = CALDCO_16MHZ;					// Set DCO step + modulation*/

    //____PORTS____//
    P1DIR |= BIT4;
    P1SEL |= BIT7 + BIT5;				// Sets Pins 5/7 to USBCLK and UCBSIMO
    P1SEL2 |= BIT7 + BIT5;				// Sets Pins 5/7 to USBCLK and UCBSIMO

    //___SPI___//
    UCB0CTL0 |=	UCCKPL 						// inactive state high, so active state low
    			+ UCMSB						// MSB first
    			+ UCMST						// Master Mode
    			+ UCSYNC;					// Synchronous mode
    UCB0CTL1 |= UCSSEL_2; 					// Sets UCB clock basis to SMCLK; 16MHz
    UCB0BR0 |= 0x01;						// Counts from 0x00 to 0x10; 16 cycles on a 16 MHz CLK
    UCB0BR1 |= 0x00;						// ERGO: SPI CLK = 1MHz
    UCB0CTL1 &= ~UCSWRST;					// Disables reset state; initialized

    //___TIMER___//
    CCTL0 = CCIE;                             // CCR0 interrupt enabled
    CCR0 = 0;
    TACTL = TASSEL_2 + MC_2;                  // SMCLK, contmode
    _BIS_SR(LPM0_bits + GIE);                 // Enter LPM0 w/ interrupt
    return 0;
}

void fg_init()
{
    //____PORTS____//
    P1DIR |= BIT4;
    P1SEL |= BIT7 + BIT5;				// Sets Pins 5/7 to USBCLK and UCBSIMO
    P1SEL2 |= BIT7 + BIT5;				// Sets Pins 5/7 to USBCLK and UCBSIMO

    //___SPI___//
    UCB0CTL0 |=	UCCKPL 						// inactive state high, so active state low
    			+ UCMSB						// MSB first
    			+ UCMST						// Master Mode
    			+ UCSYNC;					// Synchronous mode
    UCB0CTL1 |= UCSSEL_2; 					// Sets UCB clock basis to SMCLK; 16MHz
    UCB0BR0 |= 0x01;						// Counts from 0x00 to 0x10; 16 cycles on a 16 MHz CLK
    UCB0BR1 |= 0x00;						// ERGO: SPI CLK = 1MHz
    UCB0CTL1 &= ~UCSWRST;					// Disables reset state; initialized

    //___TIMER___//
    CCTL0 = CCIE;                             // CCR0 interrupt enabled
    CCR0 = 0;
    TACTL = TASSEL_2 + MC_2;                  // SMCLK, contmode


}

void Drive_DAC(unsigned int level)
{
	unsigned int DAC_Word = 0;

	DAC_Word = (0x1000) | (level & 0x0FFF);	// 0x1000
											// 0x1 = 0001
											// 0: Write to DAC register
											// 0: bleh
											// 0: 2x Output Gain
											// 1: Active Mode, Vout available

	P1OUT &= ~BIT4; 						// Clear pin 4 for /CS active low

	UCB0TXBUF = (DAC_Word >> 8); 			//UCB0TXBUF: Output Data; Shifts 8bits to right

	while(!(IFG2 & UCB0TXIFG));
	UCB0TXBUF = (unsigned char)
			(DAC_Word & 0x00FF);

	while(!(IFG2 & UCB0TXIFG));
		__delay_cycles(100);

	P1OUT |= BIT4; 							// Sets pin 4 to turn off /CS
	return;
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
	Drive_DAC(TempDAC_Value);
	fg_change(waveform, frequency, duty);
	CCR0 += ccr0_add_value;
}



void fg_change(unsigned int wf, unsigned int freq, unsigned int dty)
{
	if(sinewave_sel == 1)
	{
		if(freq == 1)
		{
			ccr0_add_value = CCR0_100HZ_SINE;
			skip_count = 1;
		}
		else if(freq == 2)
		{
			ccr0_add_value = CCR0_200HZ_SINE;
			skip_count = 2;
		}
		else if(freq == 3)
		{
			ccr0_add_value = CCR0_300HZ_SINE;
			skip_count = 2;
		}
		else if(freq == 4)
		{
			ccr0_add_value = CCR0_400HZ_SINE;
			skip_count = 3;
		}
		else if(freq == 5)
		{
			ccr0_add_value = CCR0_500HZ_SINE;
			skip_count = 4;
		}
	}

	else
	{
		if(freq == 1)
		{
			ccr0_add_value = CCR0_100HZ;
		}
		else if(freq == 2)
		{
			ccr0_add_value = CCR0_200HZ;
		}
		else if(freq == 3)
		{
			ccr0_add_value = CCR0_300HZ;
		}
		else if(freq == 4)
		{
			ccr0_add_value = CCR0_400HZ;
		}
		else if(freq == 5)
		{
			ccr0_add_value = CCR0_500HZ;
		}
	}


	if(wf == 1)//SQUARE WAVE 5Vpp; 100Hz; 2.5V DC offset
	{
		if(sinewave_sel == 1)
		{
			interrupt_count = 0;
			sinewave_sel == 0;
			TempDAC_Value = 0;
		}


		unsigned int square_count = 128;

		if(freq == 1)
		{
			square_count = 128;

			//if(duty_toggle == 1)
			//{
			//	if(dty == 1)
			//	{
			//		square_count = 128;
			//	}
			//	else if(dty == 1)
			//	{}
			//}

			//else
			//{
			//	if(dty == 0)
			//		square_count = 128;
			//}

		}
		else if (freq == 2)
			square_count  = 76;
		else if (freq == 3)
			square_count  = 50;
		else if (freq == 4)
			square_count  = 38;
		else if (freq == 5)
			square_count  = 30;

		if(interrupt_count < square_count)
		{
			interrupt_count++;
		}


		else
		{
			if(TempDAC_Value == 2000)
			{
				TempDAC_Value -= 2000;
				duty_toggle = 0;
			}

			else if(TempDAC_Value == 0)
			{
				TempDAC_Value += 2000;
				duty_toggle = 1;
			}
			interrupt_count = 0;
		}
	}

	else if (wf == 2) // TRIANGLE WAVE 5Vpp; 100Hz; 2.5V DC offset
	{
		if(sinewave_sel == 1)
		{
			interrupt_count = 0;
			sinewave_sel = 0;
			TempDAC_Value = 0;
		}

		unsigned int triangle_increase = 16;

		if(freq == 1)
			triangle_increase = 16;
		else if (freq == 2)
			triangle_increase = 26;
		else if (freq == 3)
			triangle_increase = 40;
		else if (freq == 4)
			triangle_increase = 53;
		else if (freq == 5)
			triangle_increase = 67;

		if(increment_TempDAC == 1)
		{
			TempDAC_Value += triangle_increase;
			if(TempDAC_Value > 1980)
			{
				increment_TempDAC = 0;
				TempDAC_Value = 1980;
			}
		}

		else
		{
			TempDAC_Value -= triangle_increase;
			if(TempDAC_Value > 3000)
			{
				increment_TempDAC = 1;
				TempDAC_Value = 0;
			}
		}
	}


	else if(wf == 3)
	{
		if(sinewave_sel == 0)
		{
			interrupt_count = 0;
			sinewave_sel = 1;
			TempDAC_Value = 0;
		}

		if(interrupt_count < 208)
		{
			TempDAC_Value = SineWave[interrupt_count];
			interrupt_count += skip_count;
		}

		else
		{
			interrupt_count -= 208;
			TempDAC_Value = SineWave[interrupt_count];
		}
	}
}


void fg_change_waveform(unsigned int wf)
{
	waveform = wf;
}

void fq_change_duty(unsigned int dty)
{
	duty = dty;
}

void fg_change_frequency(unsigned int freq)
{
	frequency = freq;
}
