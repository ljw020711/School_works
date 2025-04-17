/*
 * GccApplication_lab_prep.c
 *
 * Created: 7/27/2023 10:26:32 AM
 * Author : suhtw
 */ 

#pragma GCC target ("thumb")

#include "sam.h"

int foo(int, int);

extern int lab_asm_port_test();
extern int lab_asm_func_test();

int main()
{
   int c;
   /* Initialize the SAM system */
   SystemInit();
   
   lab_asm_port_test();					// Call assembly code
   
   // Pin Configuration (Switch input)
   PORT->Group[0].PINCFG[8].reg = 0x2;	// PA08: INEN = 1, PMUXEN = 0
   PORT->Group[0].PINCFG[9].reg = 0x2;	// PA09: INEN = 1, PMUXEN = 0
   
   // Pin Configuration (LED output)
   PORT->Group[0].PINCFG[6].reg = 0x0;	// PA06: INEN = 0, PMUXEN = 0
   PORT->Group[0].PINCFG[7].reg = 0x0;	// PA07: INEN = 0, PMUXEN = 0
   
   // Direction for PA06 and PA07
   PORT->Group[0].DIR.reg = 0x3 << 6;	// Direction: PA06, PA07 (Output) PA08, PA09 (Input)
   
   while (1) {							// Executing forever
      PORT->Group[0].OUT.reg = (((PORT->Group[0].IN.reg >> 8) & 0x01) << 7) | (((PORT->Group[0].IN.reg >> 9) & 0x01) << 6); 
      // Get input data from switch 8, 9 then assign that value(0 or 1) to led7, 6 each
   }
   
   /*
   c = lab_asm_func_test();
   c = foo(c, 2);
   */
   
   return (c);
}

int foo(int a, int b) {
   return (a + b);
}