/*
 * GccApplication_lab_prep.c
 *
 * Created: 7/27/2023 10:26:32 AM
 * Author : suhtw
 */ 

#pragma GCC target ("thumb")

#include "sam.h"

void GCLK_setup();
void PORT_setup();
void RTC_setup();

int main()
{
	int led;
	
	/* Initialize the SAM system */
	SystemInit();

	GCLK_setup();							// Setup Generic Clock
				
	PORT_setup();							// Setup PORT(PIN)
	
	RTC_setup();							// Setup RTC
	
	led = 1;
	
	while (1) {
		if (RTC->MODE0.INTFLAG.bit.CMP0) {
			PORT->Group[0].OUT.reg = led << 17; // Turn on Built-in LED: Output register
			led = led ^ 1;						// toggle
			RTC->MODE0.INTFLAG.bit.CMP0 = 1;	// clear overflow interrupt flag
		}
	}
	
	return (1);
}



void GCLK_setup() {
	SYSCTRL->OSC8M.bit.PRESC = 0;				// Prescaler = 1
	SYSCTRL->OSC8M.bit.ONDEMAND = 0;			// Oscillator is always on 
	
	GCLK->GENCTRL.bit.ID = 0;					// Select Generic Clock Generator 0
	GCLK->GENCTRL.bit.SRC = 6;					// Select 8Mhz Clock(0x06)
	GCLK->GENCTRL.bit.OE = 1;					// Output Enable: GCLK_IO[0], for comparing purpose
	GCLK->GENCTRL.bit.GENEN = 1;				// Generator Enable
	
	GCLK->CLKCTRL.bit.ID = 4;					// Select RTC
	GCLK->CLKCTRL.bit.GEN = 0;					// Generator 0 is selected for RTC
	GCLK->CLKCTRL.bit.CLKEN = 1;				// Enable the clock to RTC
	
}

void PORT_setup() {
	PORT->Group[0].PINCFG[14].reg = 0x41;		// DRVSTR = 1, PMUXEN = 1
	PORT->Group[0].PMUX[7].bit.PMUXE = 0x07;	// 14 / 2 = 7, 14(Even) <- function H selected
	
	PORT->Group[0].PINCFG[17].reg = 0x0;		// peripheral Mux enable = 0
	PORT->Group[0].DIR.reg = 0x1 << 17;			// Direction: Output
	PORT->Group[0].OUT.reg = 0 << 17 ;			// turn off the led(initial)
	
}

void RTC_setup() {
	RTC->MODE0.CTRL.bit.ENABLE = 0;				// first, disable the RTC
	RTC->MODE0.CTRL.bit.MODE = 0;				// set mode 0
	RTC->MODE0.CTRL.bit.MATCHCLR = 1;			// clear the counter value if the value equals compare register
	
	RTC->MODE0.COMP->reg = 4000;				// count until 4000 in decimal
	RTC->MODE0.COUNT.reg = 0x0;					// start counting from 0
	RTC->MODE0.CTRL.bit.ENABLE = 1;				// enable the RTC(counting starts)
	
}