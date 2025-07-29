/*
 * GccApplication2.c
 *
 * Created: 7/3/2023 7:19:17 PM
 * Author : suhtw
 */ 

#pragma GCC target ("thumb")

#include "sam.h"
#include "uart_print.h"

void GCLK_setup();
void USART_setup();
void PORT_setup();
void EIC_setup();
void RTC_setup();
void TC3_setup();
void TC4_setup();
void TC5_setup();

int main()
{
    int i;
   
   /* Initialize the SAM system */
    SystemInit();
   
   GCLK_setup();
   
   USART_setup();

   PORT_setup();
   
   EIC_setup();
   
   TC3_setup();
   
   TC4_setup();
   
   TC5_setup();
   
//
// NVIC setup for EIC (ID #4)
//

   NVIC->ISER[0] = (1 << 4) | (1 << 20);;  // Interrupt Set Enable for EIC and TC5
   NVIC->IP[1] = 0x40 << 0 ;            // priority for EIC: IP1[7:0] = 0x40 (=b0100_0000, 2-bit MSBs)   
   NVIC->IP[5] = 0xC0 << 0;            // priority for TC5: IP5[7:0] = 0xC0 (=b1100_0000, 2-bit MSBs)

   // doing nothing meaningful
   int a = 0;

   while (1) {
      
      a += 0;
   
   };
      
   return (0);
}


void GCLK_setup() {
   
   // OSC8M
   SYSCTRL->OSC8M.bit.PRESC = 0;  // prescalar to 1
   SYSCTRL->OSC8M.bit.ONDEMAND = 0;	// oscillator always on

   //
   // Generic Clock Controller setup for RTC
   // * RTC ID: #4 
   // * Generator #0 is feeding RTC
   // * Generator #0 is taking the clock source #6 (OSC8M: 8MHz clock input) as an input
   //
   // * EIC ID: #5
   //
   GCLK->GENCTRL.bit.ID = 0; // Generator #0
   GCLK->GENCTRL.bit.SRC = 6; // OSC8M
   GCLK->GENCTRL.bit.GENEN = 1; // Generator Enable
   
   GCLK->CLKCTRL.bit.ID = 4; // ID #4 (RTC)
   GCLK->CLKCTRL.bit.GEN = 0; // Generator #0 selected for RTC
   GCLK->CLKCTRL.bit.CLKEN = 1; // Now, clock is supplied to RTC!   
   
   GCLK->CLKCTRL.bit.ID = 5; // ID #5 (EIC)
   GCLK->CLKCTRL.bit.GEN = 0; // Generator #0 selected for RTC
   GCLK->CLKCTRL.bit.CLKEN = 1; // Now, clock is supplied to RTC!   

}

void PORT_setup() {
   
   // PORT setup for PA17: Built-in LED output & Trigger in Ultrasonic Sensor
   PORT->Group[0].PINCFG[17].reg = 0x0;      // peripheral mux enable = 0
   PORT->Group[0].DIR.reg = 0x1 << 17;         // Direction: Output
   PORT->Group[0].OUT.reg = 0 << 17 ;          // Set the Trigger to 0

   // PORT setup for PA16 to take the echo input from Ultrasonic sensor
   PORT->Group[0].PINCFG[16].reg = 0x41;      // peripheral mux: DRVSTR=1, PMUXEN = 1
   PORT->Group[0].PMUX[8].bit.PMUXE = 0x0;      // peripheral function A (EIC) selected: EXTINT[0]
   
   // PORT setup for PA18, PA19, PA22, PA23 for TC3, TC4 (motor driver)
   PORT->Group[0].PINCFG[18].reg = 0x41;      // peripheral mux: DRVSTR=1, PMUXEN = 1
   PORT->Group[0].PINCFG[19].reg = 0x41;      // peripheral mux: DRVSTR=1, PMUXEN = 1
   PORT->Group[0].PMUX[9].bit.PMUXE = 0x4;      // peripheral function E
   PORT->Group[0].PMUX[9].bit.PMUXO = 0x4;      // peripheral function E
   
   PORT->Group[0].PINCFG[22].reg = 0x41;      // peripheral mux: DRVSTR=1, PMUXEN = 1
   PORT->Group[0].PINCFG[23].reg = 0x41;      // peripheral mux: DRVSTR=1, PMUXEN = 1
   PORT->Group[0].PMUX[11].bit.PMUXE = 0x4;   // peripheral function E
   PORT->Group[0].PMUX[11].bit.PMUXO = 0x4;   // peripheral function E
   
   // PORT setup for PA06, PA07 for the direction of the motor
   PORT->Group[0].PINCFG[6].reg = 0x0;         // PMUXEN = 0
   PORT->Group[0].PINCFG[7].reg = 0x0;         // PMUXEN = 0
   
   PORT->Group[0].DIRSET.reg = 0X3 << 6;      // use PA06, PA07 as output pin
   PORT->Group[0].OUTSET.reg = 0X1 << 7;      // PA07 : 1, PA06 : 0
}

void EIC_setup() {
   // Interrupt configuration for EXTINT[0] via PA16
   EIC->CONFIG[0].bit.FILTEN0 = 1 ;      // filter is enabled
   EIC->CONFIG[0].bit.SENSE0 = 0x3 ;      // Both-edges detection
   EIC->INTENSET.bit.EXTINT0 = 1 ;         // External Interrupt 0 is enabled
   EIC->CTRL.bit.ENABLE = 1 ;            // EIC is enabled   
}

void RTC_setup() {
   //
   // RTC setup: MODE0 (32-bit counter) with COMPARE 0
   //

   RTC->MODE0.CTRL.bit.ENABLE = 0; // Disable first
   RTC->MODE0.CTRL.bit.MODE = 0; // Mode 0
   RTC->MODE0.CTRL.bit.MATCHCLR = 1; // match clear
   
   // 8MHz RTC clock  --> 10 usec when 80 is counted
   RTC->MODE0.COMP->reg = 80; // compare register to set up 10usec interval 
   RTC->MODE0.COUNT.reg = 0x0; // initialize the counter to 0
   RTC->MODE0.CTRL.bit.ENABLE = 1; // Enable
}


//
// EIC Interrupt Handler
//

unsigned int RTC_count, count_start, count_end;		// count to measure the distance
unsigned int Num_EIC_interrupts = 0;				// number of interrupts(to identify start or end of measuring)

void EIC_Handler(void)
{
   unsigned int echo_time_interval, distance;		// RTT, distance to the wall
   
   EIC->INTFLAG.bit.EXTINT0 = 1 ; // Clear the EXTINT0 interrupt flag
   Num_EIC_interrupts++;			// increment number of interrupt (new interrupt appear)
   
   if (Num_EIC_interrupts == 1) {			// first interrupt(start of measuring)
      count_start = RTC->MODE0.COUNT.reg;	// counter value of starting
   }
   else if (Num_EIC_interrupts == 2) {		// second interrupt(end of measuring)
      
      count_end = RTC->MODE0.COUNT.reg;		// counter value of ending
      RTC_count = count_end - count_start;	// get how many clocks used for measuring distance
      echo_time_interval = RTC_count / 8 ;         // echo interval in usec (8MHz clock input)
      distance = (echo_time_interval / 2) * 0.034 ;   // distance in cm
      
      
      if (distance <= 10) {                  // Stop
         TC3->COUNT16.CC[1].reg = 0;            // stop by generating no pulse
         TC4->COUNT16.CC[1].reg = 0;            // stop by generating no pulse
         
      }
      else if (distance <= 20) {               // low speed
         TC3->COUNT16.CC[1].reg = 200;         // low speed: on duration 200 / 1000
         TC4->COUNT16.CC[1].reg = 200;         // low speed: on duration 200 / 1000
         
      }
      else if (distance <= 30) {               // mid speed
         TC3->COUNT16.CC[1].reg = 600;         // mid speed: on duration 600 / 1000
         TC4->COUNT16.CC[1].reg = 600;         // mid speed: on duration 600 / 1000
         
      }
      else if (distance > 30) {               // hight speed
         TC3->COUNT16.CC[1].reg = 1000;         // high speed: on duration 1000 / 1000 (full speed)
         TC4->COUNT16.CC[1].reg = 1000;         // high speed: on duration 1000 / 1000 (full speed)
         
      }
      
      
      
      print_decimal(distance / 100);            // print 100s
      distance = distance % 100;               // get 10s and 1s
      print_decimal(distance / 10);				// print 100s
      print_decimal(distance % 10);				// print 100s
      print_enter();							// print 100s
      Num_EIC_interrupts = 0 ;					// clear num_interrupt
   }
}

void TC5_Handler(void) {
   
   TC5->COUNT16.INTFLAG.bit.OVF = 1 ;			// Clear the interrupt flag
   
   // Supply 10 usec pulse to Trigger input in Ultrasonic sensor
   PORT->Group[0].OUTCLR.reg = 1 << 17 ;		// first clear out register
   
   RTC_setup();									// enable RTC
   
   while (RTC->MODE0.INTFLAG.bit.CMP0 != 1) ;	// wait until RTC is ready to count new 10usec

   RTC->MODE0.INTFLAG.bit.CMP0 = 1;				// clear overflow interrupt flag
   PORT->Group[0].OUTSET.reg = 1 << 17;			// out register to 1

   while (RTC->MODE0.INTFLAG.bit.CMP0 != 1) ;	// 10 usec over

   RTC->MODE0.INTFLAG.bit.CMP0 = 1;				// clear overflow interrupt flag
   PORT->Group[0].OUTCLR.reg = 1 << 17 ;		// out register to 0

   // Now use RTC to measure the pulse width of echo input.
   RTC->MODE0.CTRL.bit.ENABLE = 0;				// Disable first
   RTC->MODE0.CTRL.bit.MATCHCLR = 0;			// Now just count...
   RTC->MODE0.COUNT.reg = 0x0;					// initialize the counter to 0
   RTC->MODE0.CTRL.bit.ENABLE = 1;				// Enable
}

void USART_setup() {
   
   //
   // PORT setup for PB22 and PB23 (USART)
   //
   PORT->Group[1].PINCFG[22].reg = 0x41; // peripheral mux: DRVSTR=1, PMUXEN = 1
   PORT->Group[1].PINCFG[23].reg = 0x41; // peripheral mux: DRVSTR=1, PMUXEN = 1

   PORT->Group[1].PMUX[11].bit.PMUXE = 0x03; // peripheral function D selected
   PORT->Group[1].PMUX[11].bit.PMUXO = 0x03; // peripheral function D selected

   // Power Manager
   PM->APBCMASK.bit.SERCOM5_ = 1 ; // Clock Enable (APBC clock) for USART
   
   //
   // * SERCOM5: USART
   // * Generator #0 is feeding USART as well
   //
   GCLK->CLKCTRL.bit.ID = 0x19; // ID #0x19 (SERCOM5: USART): GCLK_SERCOM3_CORE
   GCLK->CLKCTRL.bit.GEN = 0; // Generator #0 selected for USART
   GCLK->CLKCTRL.bit.CLKEN = 1; // Now, clock is supplied to USART!

   GCLK->CLKCTRL.bit.ID = 0x13; // ID #0x13 (SERCOM5: USART): GCLK_SERCOM_SLOW
   GCLK->CLKCTRL.bit.GEN = 0; // Generator #0 selected for USART
   GCLK->CLKCTRL.bit.CLKEN = 1; // Now, clock is supplied to USART!
   
   //
   // USART setup
   //
   SERCOM5->USART.CTRLA.bit.MODE = 1 ; // Internal Clock
   SERCOM5->USART.CTRLA.bit.CMODE = 0 ; // Asynchronous UART
   SERCOM5->USART.CTRLA.bit.RXPO = 3 ; // PAD3
   SERCOM5->USART.CTRLA.bit.TXPO = 1 ; // PAD2
   SERCOM5->USART.CTRLB.bit.CHSIZE = 0 ; // 8-bit data
   SERCOM5->USART.CTRLA.bit.DORD = 1 ; // LSB first

   SERCOM5->USART.BAUD.reg = 0Xc504 ; // 115,200 bps (baud rate) with 8MHz input clock
   
   SERCOM5->USART.CTRLB.bit.RXEN = 1 ;	//Enable RX
   SERCOM5->USART.CTRLB.bit.TXEN = 1 ;	//Enable TX

   SERCOM5->USART.CTRLA.bit.ENABLE = 1;   //Enable UART
}

void TC3_setup()
{
   GCLK->CLKCTRL.bit.ID = 0x1B;				// select TC3
   GCLK->CLKCTRL.bit.GEN = 0;				// select generator 0
   GCLK->CLKCTRL.bit.CLKEN = 1;				// enable clock (8MHz)
   
   PM->APBCMASK.bit.TC3_ = 1;				// clock enable (APBC clock) for TC3
   
   TC3->COUNT16.CTRLA.bit.MODE = 0;			// 16bit counter mode
   TC3->COUNT16.CTRLA.bit.WAVEGEN = 3;		// match pwm
   TC3->COUNT16.CTRLA.bit.PRESCALER = 6;	// prescaler 256
   
   TC3->COUNT16.COUNT.reg = 0;				// count start from 0
   TC3->COUNT16.CC[0].reg = 1000;			// total period = 1000
   TC3->COUNT16.CC[1].reg = 0;				// on duration = 0
   TC3->COUNT16.CTRLA.bit.ENABLE = 1;		// enable TC3
}

void TC4_setup()
{
   GCLK->CLKCTRL.bit.ID = 0x1C;				// select TC4, TC5
   GCLK->CLKCTRL.bit.GEN = 0;				// select generator 0
   GCLK->CLKCTRL.bit.CLKEN = 1;				// enable clock (8MHz)
   
   PM->APBCMASK.bit.TC4_ = 1;				// clock enable (APBC clock) for TC4
   
   TC4->COUNT16.CTRLA.bit.MODE = 0;			// 16bit counter mode
   TC4->COUNT16.CTRLA.bit.WAVEGEN = 3;		// match pwm
   TC4->COUNT16.CTRLA.bit.PRESCALER = 6;	// prescaler 256
   
   TC4->COUNT16.COUNT.reg = 0;				// count start from 0
   TC4->COUNT16.CC[0].reg = 1000;			// total period = 1000
   TC4->COUNT16.CC[1].reg = 0;				// on duration = 0
   TC4->COUNT16.CTRLA.bit.ENABLE = 1;		// enable tc4
}

void TC5_setup()
{
   PM->APBCMASK.bit.TC5_ = 1;               // clock enable (APBC clock) for TC5
   
   TC5->COUNT16.CTRLA.bit.MODE = 0;         // 16bit counter mode
   TC5->COUNT16.CTRLA.bit.WAVEGEN = 1;      // MFRQ
   TC5->COUNT16.CTRLA.bit.PRESCALER = 6;    // prescaler = 256, 8MHz / 256 = 31250
   
   TC5->COUNT16.INTENSET.bit.OVF = 1;       // Enable interrupt
   
   TC5->COUNT16.COUNT.reg = 0;				// count from 0
   TC5->COUNT16.CC[0].reg = 31250;          // count until cc0 and overflow(interrupt)
   TC5->COUNT16.CTRLA.bit.ENABLE = 1;       // start counter
}