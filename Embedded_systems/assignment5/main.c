/*
 * GccApplication2.c
 *
 * Created: 7/3/2023 7:19:17 PM
 * Author : suhtw1
 */ 

#pragma GCC target ("thumb")

#include "sam.h"
#include "uart_print.h"

void GCLK_setup();
void USART_setup();
void PORT_setup();
void TC3_setup();
void TC4_setup();
void print_instructions();

//
// Prof. Suh: 
// Note that PA23 (TC4's WO[1]) is held high via pull-up resistor on Zero board
// Thus, motor is running full-speed after reset
//

unsigned char dashline[]      = {"----------------- Vehicle Control ----------------"};
unsigned char motor_inst_12[]   = {"1. Forward 2. Backward "};
unsigned char motor_inst_34[]   = {"3. Speed Up 4. Speed Down"};
unsigned char motor_inst_567[]   = {"5. Left turn 6. Right turn 7. Stop"};
unsigned char dashline_e[]      = {"-------------------------------------------------"};
unsigned char select[]         = {"Select: "};

int main()
 {   
   unsigned char rx_data;
   _Bool   rxc_flag;   
   
   /* Initialize the SAM system */
    SystemInit();
   
   GCLK_setup();
   
   USART_setup();

   PORT_setup();
   
   TC3_setup(); 

   TC4_setup(); 
   

   print_instructions();
   while(1) {
      rxc_flag = SERCOM5->USART.INTFLAG.bit.RXC;	// check out RXC (Receive Complete) flag
      
      
      if (rxc_flag == 1)   {					// if RXC in 1
         rx_data   = SERCOM5->USART.DATA.reg;	// Read the received data
         SERCOM5->USART.DATA.reg   = rx_data;	// Write the received data (show the received word in tera term)
         
         print_enter();
         
         if (rx_data == '7') {					// 7: Stop
            TC3->COUNT16.CC[1].reg = 0;         // stop by generating no pulse
            TC4->COUNT16.CC[1].reg = 0;         // stop by generating no pulse
            
         }
         else if (rx_data == '1') {				// 1: Forward
            PORT->Group[0].OUT.reg = 0x1 << 7;  // PA07 = 1, PA06 = 0, (IN1 = 1, IN2 = 0), (IN3 = 1, IN4 = 0)
            TC3->COUNT16.CC[1].reg = 200;		// Initial speed = 200
            TC4->COUNT16.CC[1].reg = 200;		// Initial speed = 200
            
         }
         else if (rx_data == '2') {				// 2: Backward
            PORT->Group[0].OUT.reg = 0x1 << 6;  // PA07 = 0, PA06 = 1, (IN1 = 0, IN2 = 1), (IN3 = 0, IN4 = 1)
            TC3->COUNT16.CC[1].reg = 200;		// Initial speed = 200
            TC4->COUNT16.CC[1].reg = 200;		// Initial speed = 200
            
         }
         else if (rx_data == '3') {				// 3: Speed up
            TC3->COUNT16.CC[1].reg = TC3->COUNT16.CC[1].reg + 100;  // Increase speed
            TC4->COUNT16.CC[1].reg = TC4->COUNT16.CC[1].reg + 100;  // Increase speed
            
         }
         else if (rx_data == '4') {				// 4: Speed down
            TC3->COUNT16.CC[1].reg = TC3->COUNT16.CC[1].reg - 100;  // Decrease speed
            TC4->COUNT16.CC[1].reg = TC4->COUNT16.CC[1].reg - 100;  // Decrease speed
            
         }
         else if (rx_data == '5') {				// 5: Left turn
            TC3->COUNT16.CC[1].reg = TC3->COUNT16.CC[1].reg > 200 + 200? TC3->COUNT16.CC[1].reg - 200 : 200;	// Decrease speed: No lower than 100
            TC4->COUNT16.CC[1].reg = TC4->COUNT16.CC[1].reg + 200;												// Increase speed: TC4-> ENA-> control IN1, IN2(right wheels)
            
         }
         else if (rx_data == '6') {				// 6: right turn
            TC3->COUNT16.CC[1].reg = TC3->COUNT16.CC[1].reg + 200;												// Increase speed: TC3-> ENB-> control IN3, IN4(left wheels)
            TC4->COUNT16.CC[1].reg = TC4->COUNT16.CC[1].reg > 200 + 200? TC4->COUNT16.CC[1].reg - 200 : 200;	// Decrease speed: No lower than 100
            
         }
         
         print_instructions();               // print instruction list on tera term
      }
      
   };

   return (0);
}


void print_instructions()
{   
   println_string(dashline,     sizeof(dashline));				// print starting dashline
   println_string(motor_inst_12,  sizeof(motor_inst_12));		// print instruction 1, 2
   println_string(motor_inst_34, sizeof(motor_inst_34));		// print instruction 3, 4
   println_string(motor_inst_567, sizeof(motor_inst_567));		// print instruction 5, 6, 7
   println_string(dashline_e, sizeof(dashline_e));				// print ending dashline
   print_string(select,     sizeof(select));					// print select
   
}


void GCLK_setup() {
   
   SYSCTRL->OSC8M.bit.PRESC = 0;		// prescalar = 1
   SYSCTRL->OSC8M.bit.ONDEMAND = 0;		// oscillator always turned on

   GCLK->GENCTRL.bit.ID = 0;			// Generator #0
   GCLK->GENCTRL.bit.SRC = 6;			// OSC8M
   GCLK->GENCTRL.bit.GENEN = 1;			// Generator Enable
   
   GCLK->CLKCTRL.bit.ID = 0x1B;			// ID = (TCC2, TC3)
   GCLK->CLKCTRL.bit.GEN = 0;			// Generator #0
   GCLK->CLKCTRL.bit.CLKEN = 1;			// clock supplied to TC3
   
   GCLK->CLKCTRL.bit.ID = 0x1C;			// ID = (TC4, TC5)
   GCLK->CLKCTRL.bit.GEN = 0;			// Generator #0
   GCLK->CLKCTRL.bit.CLKEN = 1;			// clock supplied to TC4
   

}

void PORT_setup() {
   
   PORT->Group[0].PINCFG[6].reg = 0x0;	// peripheral mux enable = 0
   PORT->Group[0].PINCFG[7].reg = 0x0;	// peripheral mux enable = 0
   
   PORT->Group[0].DIR.reg = 0x3 << 6;	// Direction: Output
   
   PORT->Group[0].OUT.reg = 0x1 << 7;	// PA07 = 1, PA06 = 0 -> forward
}


void TC3_setup() {

   //
   // PORT setup for PA18 ( TC3's WO[0] )
   //
   PORT->Group[0].PINCFG[18].reg = 0x41;		// peripheral mux: DRVSTR=1, PMUXEN = 1
   PORT->Group[0].PMUX[9].bit.PMUXE = 0x04;		// peripheral function E selected

   //
   // PORT setup for PA19 ( TC3's WO[1] )
   //
   PORT->Group[0].PINCFG[19].reg = 0x41;		// peripheral mux: DRVSTR=1, PMUXEN = 1
   PORT->Group[0].PMUX[9].bit.PMUXO = 0x04;		// peripheral function E selected

   // Power Manager
   PM->APBCMASK.bit.TC3_ = 1 ;					// Clock Enable (APBC clock) for TC3

   //
   // TC3 setup: 16-bit Mode
   //

   TC3->COUNT16.CTRLA.bit.MODE = 0;				// Count16 mode
   TC3->COUNT16.CTRLA.bit.WAVEGEN = 3 ;			// Match PWM (MPWM)
   TC3->COUNT16.CTRLA.bit.PRESCALER = 6;		// Timer Counter clock 31,250 Hz = 8MHz / 256
   TC3->COUNT16.CC[0].reg = 1000;				// CC0 defines the period
   TC3->COUNT16.CC[1].reg = 200;				// CC1 match pulls down WO[1]   
   TC3->COUNT16.CTRLA.bit.ENABLE = 1 ;			// Enable TC3
}


void TC4_setup() {

   //
   // PORT setup for PA22 ( TC4's WO[0] )
   //
   PORT->Group[0].PINCFG[22].reg = 0x41;		// peripheral mux: DRVSTR=1, PMUXEN = 1
   PORT->Group[0].PMUX[11].bit.PMUXE = 0x04;	// peripheral function E selected

   //
   // PORT setup for PA23 ( TC4's WO[1] )
   //
   PORT->Group[0].PINCFG[23].reg = 0x41;		// peripheral mux: DRVSTR=1, PMUXEN = 1
   PORT->Group[0].PMUX[11].bit.PMUXO = 0x04;	// peripheral function E selected

   // Power Manager
   PM->APBCMASK.bit.TC4_ = 1 ;					// Clock Enable (APBC clock) for TC4

   //
   // TC4 setup: 16-bit Mode
   //

   TC4->COUNT16.CTRLA.bit.MODE = 0;				// Count16 mode
   TC4->COUNT16.CTRLA.bit.WAVEGEN = 3 ;			// Match PWM (MPWM)
   TC4->COUNT16.CTRLA.bit.PRESCALER = 6;		// Timer Counter clock 31,250 Hz = 8MHz / 256
   TC4->COUNT16.CC[0].reg = 1000;				// CC0 defines the period
   TC4->COUNT16.CC[1].reg = 200;				// CC1 match pulls down WO[1]
   TC4->COUNT16.CTRLA.bit.ENABLE = 1 ;			// TC4 Enabled
}



void USART_setup() {
   
   //
   // PORT setup for PB22 and PB23 (USART)
   //
   PORT->Group[1].PINCFG[22].reg = 0x41;		// peripheral mux: DRVSTR=1, PMUXEN = 1
   PORT->Group[1].PINCFG[23].reg = 0x41;		// peripheral mux: DRVSTR=1, PMUXEN = 1

   PORT->Group[1].PMUX[11].bit.PMUXE = 0x03;	// peripheral function D selected
   PORT->Group[1].PMUX[11].bit.PMUXO = 0x03;	// peripheral function D selected

   // Power Manager
   PM->APBCMASK.bit.SERCOM5_ = 1 ;				// Clock Enable (APBC clock) for USART
   
   //
   // * SERCOM5: USART
   // * Generator #0 is feeding USART as well
   //
   GCLK->CLKCTRL.bit.ID = 0x19;					// ID #0x19 (SERCOM5: USART): GCLK_SERCOM3_CORE
   GCLK->CLKCTRL.bit.GEN = 0;					// Generator #0 selected for USART
   GCLK->CLKCTRL.bit.CLKEN = 1;					// Now, clock is supplied to USART!

   GCLK->CLKCTRL.bit.ID = 0x13;					// ID #0x13 (SERCOM5: USART): GCLK_SERCOM_SLOW
   GCLK->CLKCTRL.bit.GEN = 0;					// Generator #0 selected for USART
   GCLK->CLKCTRL.bit.CLKEN = 1;					// Now, clock is supplied to USART!
   
   //
   // USART setup
   //
   SERCOM5->USART.CTRLA.bit.MODE = 1 ;			// Internal Clock
   SERCOM5->USART.CTRLA.bit.CMODE = 0 ;			// Asynchronous UART
   SERCOM5->USART.CTRLA.bit.RXPO = 3 ;			// PAD3
   SERCOM5->USART.CTRLA.bit.TXPO = 1 ;			// PAD2
   SERCOM5->USART.CTRLB.bit.CHSIZE = 0 ;		// 8-bit data
   SERCOM5->USART.CTRLA.bit.DORD = 1 ;			// LSB first

   SERCOM5->USART.BAUD.reg = 0Xc504 ;			// 115,200 bps (baud rate) with 8MHz input clock

   SERCOM5->USART.CTRLB.bit.RXEN = 1 ;			// Receive byte enable
   SERCOM5->USART.CTRLB.bit.TXEN = 1 ;			// Transfer byte enable

   SERCOM5->USART.CTRLA.bit.ENABLE = 1;			// USART Enabled
   
}
