// ------------------------------------------
//  Author: Prof. Taeweon Suh
//          Computer Science & Engineering
//          College of Informatics, Korea Univ.
//  Date:   March 29, 2020
// ------------------------------------------

// address of data register in GPIO linked to LED
#define csd_LED_ADDR 0x41200000
// address of data register in GPIO linked to switch (Not used)
#define csd_SW_ADDR 0x41210000

.extern csd_main

.global main
main:

 	ldr r4, =csd_LED_ADDR	// load address of GPIO(LED) to r4 register
	ldr r6, =0x00000001		// value used to turn on the LED, initially LED0 is on

loop:						// loop for turning on LEDs
	bl csd_main				// calling C code for checking switch input
	strb r6, [r4]			// storing output to GPIO(LED), turning led on

	mov r6, r6, ror #31		// rotate value in r6 register(input to led) to turn on the next switch (ex, LED0 -> LED1)
	cmp r6, #0x80			// if the value is bigger than 0x80
	movgt r6, #1			// set r6 to 0x01(because there are only 8 LEDS -> only last 8bits are used)

delay:						// loop for waiting time period for duration
	subs r0, r0, #1			// decrement r0(value got from C code, number of loops for duration) by 1
	bne delay				// if the loop for duration is not ended, keep decrement until 0

	b loop					// if the time delay for duration is over, go back to turn next LED on

forever:
	nop						// not used
 	b forever				// not used

.data
.align 4
