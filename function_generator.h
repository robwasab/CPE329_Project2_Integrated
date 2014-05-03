/*
 * function_generator.h
 *
 *  Created on: May 2, 2014
 *      Author: ryhuang
 */

#ifndef FUNCTION_GENERATOR_H_
#define FUNCTION_GENERATOR_H_

#define CCR0_100HZ_SINE 769
#define CCR0_200HZ_SINE 769
#define CCR0_300HZ_SINE 513
#define CCR0_400HZ_SINE 571
#define CCR0_500HZ_SINE 615

#define CCR0_100HZ 625
#define CCR0_200HZ 516
#define CCR0_300HZ 533
#define CCR0_400HZ 533
#define CCR0_500HZ 533

void fg_change_frequency(unsigned int freq);
void fg_change_waveform(unsigned int wf);
void fq_change_duty(unsigned int dty);

void fg_init(void);
int test_main(void);

#endif /* FUNCTION_GENERATOR_H_ */
