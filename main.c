#include <msp430.h>
#include "print.h"
#include "lcd.h"
#include "function_generator.h"
#include "delay.h"

#define FREQ_SEL_BUTTON BIT1
#define WAVE_SEL_BUTTON BIT2
#define DUTY_SEL_BUTTON BIT3
#define PERIOD (1 << 14)

int main(void)
{

  WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

  if (CALBC1_16MHZ==0xFF)					// If calibration constant erased
  {
    while(1);                               // do not load, trap CPU!!
  }

  DCOCTL = 0;                               // Select lowest DCOx and MODx settings
  BCSCTL1 = CALBC1_16MHZ;                   // Set range
  DCOCTL = CALDCO_16MHZ;                    // Set DCO step + modulation*/

  //configure ACLK to use clock crystal
  //DIVA is the ACLK divider.
  //When DIVA0 and DIVA1 Bits are zero, ACLK is divided
  //by 1
  BCSCTL1 &= ~(DIVA0 | DIVA1);

  //No touch, BCSTL2, only handles SMCLK and MCLK

  //clear the Low Freq XTal 1 sel bits 0 and 1 to select
  //32768Hz clk for LFXT1 oscillator
  //This also routes LFXT1 into ACLK. if LFXT1Sx = 10, ACLK uses VLOCLK instead
  BCSCTL3 &= ~(LFXT1S0 | LFXT1S1);

  //set XCAP to 11 to select a 12.5pF crystal capacitance
  BCSCTL3 |= XCAP1 | XCAP0;

BUTTONS_INIT:
  //configure P1.3 to switch on falling edge interrupts
  //force the pin to use the typical GPIO
  P1SEL  &= ~(FREQ_SEL_BUTTON | WAVE_SEL_BUTTON | DUTY_SEL_BUTTON);
  P1SEL2 &= ~(FREQ_SEL_BUTTON | WAVE_SEL_BUTTON | DUTY_SEL_BUTTON);

  //select HIGH to LOW transition
  P1IES |= FREQ_SEL_BUTTON | WAVE_SEL_BUTTON | DUTY_SEL_BUTTON;
  P1IE  |= FREQ_SEL_BUTTON | WAVE_SEL_BUTTON | DUTY_SEL_BUTTON;
  P1IFG &= ~(FREQ_SEL_BUTTON | WAVE_SEL_BUTTON | DUTY_SEL_BUTTON);
  P1DIR &= ~(FREQ_SEL_BUTTON | WAVE_SEL_BUTTON | DUTY_SEL_BUTTON);
  P1REN |= FREQ_SEL_BUTTON | WAVE_SEL_BUTTON | DUTY_SEL_BUTTON;

  lcd_init();
  display_reset();

//FUNCTION_GENERATOR_INIT:
  fg_init();

  //why not, just go back to home clear everything again

   P1IFG |= FREQ_SEL_BUTTON;

  _BIS_SR(LPM0_bits + GIE);                 // Enter LPM0 w/ interrupt
}

void print_frequency(u8 freq)
{
	char foo = freq | 0x30;
	lcd_put_char(foo);
	lcd_print("00Hz");
}

u8 debounce(void)
{
	u8 i;
	u8 count = 0;
	for (i = 0; i < 255; ++i)
	{
		if (P1IN & (FREQ_SEL_BUTTON | WAVE_SEL_BUTTON | DUTY_SEL_BUTTON)) ++count;
		delay_us(10);
		if (count == 255) return 1;
	}
	if (count > 200) return 1;
	return 0;
}


#pragma vector=PORT1_VECTOR
__interrupt void read_buttons(void)
{
	if(!debounce()) return;

    //wave state can be 0, 1, 2
	// 0 is SINE
	// 1 is SAW TOOTH
	// 2 is SQUARE

	static u8 wave_type = 1;

	//frequency can be 1, 2, 3, 4, 5
	//1 100Hz
	//2 200Hz
	//3 300Hz
	//4 400Hz
	//5 500Hz
	static u8 frequency = 1;
	//duty cycle can be 1, 2, 3, 4, 5, 6, 7, 8, 9
	//for 10, 20, 30, 40, 50.. % duty cycle
	static u8 duty_cycle = 10;

	u8 flags = P1IFG & (FREQ_SEL_BUTTON | WAVE_SEL_BUTTON | DUTY_SEL_BUTTON);

	display_reset();

	switch(flags)
	{
	case WAVE_SEL_BUTTON:
		LCD_FIRST_LINE();
		lcd_print("Set Wave Type");

		if (wave_type >= 3) wave_type = 1;
		else wave_type++;

		LCD_SECOND_LINE();

		switch(wave_type)
		{
		case 1:
			lcd_print("Square");
			break;
		case 2:
			lcd_print("Tri");
			break;
		case 3:
			lcd_print("Sine");
			break;
		}

        fg_change_waveform(wave_type);

		P1IFG &= ~WAVE_SEL_BUTTON;
		break;

	case FREQ_SEL_BUTTON:
		LCD_FIRST_LINE();

		if (frequency >= 5) { frequency = 1; }
		else {frequency ++; }

        lcd_print("Freq:");
        LCD_SECOND_LINE();
        print_frequency(frequency);

        fg_change_frequency(frequency);

		P1IFG &= ~FREQ_SEL_BUTTON;
		break;

	case DUTY_SEL_BUTTON:
		lcd_print("Duty:");
		P1IFG &= ~DUTY_SEL_BUTTON;
		break;
	default:
		P1IFG = 0;
	}

}
