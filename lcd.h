/*
 * lcd.h
 *
 *  Created on: Apr 17, 2014
 *      Author: robwasab
 */

#ifndef LCD_H_
#define LCD_H_

#define RS BIT5
#define E BIT4
#define LCD_OUT P2OUT
#define LCD_DIR P2DIR

//used with write_byte
//do I write a byte to the command register?
//or the data register?
#define CMD 0
#define DATA 1
#define u8 unsigned char
//unsigned 16 bit
#define u16 unsigned short

//used as a building block
void send_nibble(u8 nibble, u8 regsel);
//uses send_nibble
void send_byte(u8 data, u8 regsel);

//see samsung data sheet 4 bit init routine
void function_set(void);
void display_control(void);
void display_clear(void);
void display_home(void);

//display_reset uses display_clear to clear the screen
//then display_home to return the cursor back to home
void display_reset(void);

void entry_mode_set(void);

//cursor cordinate puts the cursor at a specific row 0 or 1
//or column 0 thru 15
//it uses instruction: "Set DDRAM Address"
void cursor_cordinate(u8 row, u8 col);

void lcd_put_char(char c);

//takes a char pointer (an array variable declared: "char str[] = "hello";"
//str is a pointer to a char memory block
void lcd_print(char * str);

//init combines everything
void lcd_init();

//strobes a PMOS
void lcd_reset();

void write_custom_character(const u8 * array, u8 addr);

#define LCD_SECOND_LINE() cursor_cordinate(1, 0)
#define LCD_FIRST_LINE() cursor_cordinate(0,0)

#endif /* LCD_H_ */
