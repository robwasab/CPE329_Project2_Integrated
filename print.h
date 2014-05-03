/*
 * print.h
 *
 *  Created on: Apr 17, 2014
 *      Author: robwasab
 */

#ifndef PRINT_H_
#define PRINT_H_
#include <stdint.h>

void print_nibble(uint8_t b);
void print_int(void * foo, uint8_t bytes);
#define PRINT_INT(val) print_int( (void *) &val, sizeof(val))
#endif /* PRINT_H_ */
