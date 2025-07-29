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
void RTC_setup();
int Distance();
void TC3_setup();
void TC4_setup();

int main()
{
    int i;                                 // used for idle count
   unsigned int dist = 0;                     // distance get from Ultrasonic sensor
   
   /* Initialize the SAM system */
    SystemInit();
   
   GCLK_setup();
   
   USART_setup();

   PORT_setup();
   
   TC3_setup();
   
   TC4_setup();

   while (1) {
      // ----------------------------------------------
      // 10 usec pulse to Trigger input in Ultrasonic sensor
      // --------------------------------------------
      PORT->Group[0].OUTCLR.reg = 1 << 17;      // clear bit 17 in out register(PA17: Trigger pin in Ultrasonic sensor) 
   
      RTC_setup();                        // setup and start RTC
   
      while (RTC->MODE0.INTFLAG.bit.CMP0 != 1) ;   // wait until count value in RTC to be 0

      RTC->MODE0.INTFLAG.bit.CMP0 = 1;         // clear overflow interrupt flag
      PORT->Group[0].OUTSET.reg = 1 << 17;      // set bit 17 in out register to 1 (10 usec start)

      while (RTC->MODE0.INTFLAG.bit.CMP0 != 1) ;   // wait until RTC finish counting

      RTC->MODE0.INTFLAG.bit.CMP0 = 1;         // clear overflow interrupt flag
      PORT->Group[0].OUTCLR.reg = 1 << 17;      // clear bit 17 in out register (10 usec end)
      // --------------------------------------------

      // Now use RTC to measure the pulse width of echo input.
      RTC->MODE0.CTRL.bit.ENABLE = 0;            // Disable first
      RTC->MODE0.CTRL.bit.MATCHCLR = 0;         // Now just count...
      RTC->MODE0.COUNT.reg = 0x0;               // initialize the counter to 0
      RTC->MODE0.CTRL.bit.ENABLE = 1;            // Enable   
   
      // Measure the distance   
      dist = Distance();
      
      if (dist <= 10) {                     // Stop
         TC3->COUNT16.CC[1].reg = 0;            // stop by generating no pulse
         TC4->COUNT16.CC[1].reg = 0;            // stop by generating no pulse
         
      }
      else if (dist <= 20) {                  // low speed
         TC3->COUNT16.CC[1].reg = 200;         // low speed: on duration 200 / 1000
         TC4->COUNT16.CC[1].reg = 200;         // low speed: on duration 200 / 1000
         
      }
      else if (dist <= 30) {                  // mid speed
         TC3->COUNT16.CC[1].reg = 600;         // mid speed: on duration 600 / 1000
         TC4->COUNT16.CC[1].reg = 600;         // mid speed: on duration 600 / 1000
         
      }
      else if (dist > 30) {                  // high speed
         TC3->COUNT16.CC[1].reg = 1000;         // high speed: on duration 1000 / 1000 (full speed)
         TC4->COUNT16.CC[1].reg = 1000;         // high speed: on duration 1000 / 1000 (full speed)
         
      }

      for (i= 0; i<1000000; i++ ) ;            // idle roughly for 1 second
   
   };

   return (0);
}


void GCLK_setup() {
   
   // OSC8M
   SYSCTRL->OSC8M.bit.PRESC = 0;               // prescaler to 1
   SYSCTRL->OSC8M.bit.ONDEMAND = 0;            // oscillator always on

   //
   // Generic Clock Controller setup for RTC
   // * RTC ID: #4 
   // * Generator #0 is feeding RTC
   // * Generator #0 is taking the clock source #6 (OSC8M: 8MHz clock input) as an input

   GCLK->GENCTRL.bit.ID = 0;                  // Generator #0
   GCLK->GENCTRL.bit.SRC = 6;                  // OSC8M
   GCLK->GENCTRL.bit.OE = 1 ;                  // Output Enable: GCLK_IO
   GCLK->GENCTRL.bit.GENEN = 1;               // Generator Enable
   
   GCLK->CLKCTRL.bit.ID = 4;                  // ID #4 (RTC)
   GCLK->CLKCTRL.bit.GEN = 0;                  // Generator #0 selected for RTC
   GCLK->CLKCTRL.bit.CLKEN = 1;               // Now, clock is supplied to RTC!   

}

void PORT_setup() {
   
   //
   // PORT setup for PA17: Built-in LED output & Trigger in Ultrasonic Sensor
   //
   PORT->Group[0].PINCFG[17].reg = 0x0;         // PMUXEN = 0
   PORT->Group[0].DIRSET.reg = 0x1 << 17;         // Direction: Output
   PORT->Group[0].OUT.reg = 0 << 17 ;            // Set the Trigger to 0

   //
   // PORT setup for PA16 to take the echo input from Ultrasonic sensor
   //
   PORT->Group[0].PINCFG[16].reg = 0x2;         // INEN = 1, PMUXEN = 0
   PORT->Group[0].DIRCLR.reg = 0x1 << 16;         // Direction: Input

   PORT->Group[0].PINCFG[6].reg = 0x0;            // PMUXEN = 0
   PORT->Group[0].PINCFG[7].reg = 0x0;            // PMUXEN = 0
   
   PORT->Group[0].DIRSET.reg = 0x3 << 6;         // Direction(PA06, PA07): Output
   
   PORT->Group[0].OUTSET.reg = 0x1 << 7;         // car moving forward
}


void RTC_setup() {
   //
   // RTC setup: MODE0 (32-bit counter) with COMPARE 0
   //
   RTC->MODE0.CTRL.bit.ENABLE = 0;               // Disable first
   RTC->MODE0.CTRL.bit.MODE = 0;               // Mode 0
   RTC->MODE0.CTRL.bit.MATCHCLR = 1;            // match clear
   
   // 8MHz RTC clock  --> 10 usec when 80 is counted
   RTC->MODE0.COMP->reg = 80;                  // compare register to set up 10usec interval 
   RTC->MODE0.COUNT.reg = 0x0;                  // initialize the counter to 0
   RTC->MODE0.CTRL.bit.ENABLE = 1;               // Enable
}


int Distance(void)
{
   unsigned int RTC_count, count_start, count_end;   // value in count register in RTC
   unsigned int echo_time_interval, distance, distance_return;      // time interval, distance
   
   // take the echo input from PA16
   while (!(PORT->Group[0].IN.reg & 0x00010000)) ;   // wait until Ultrasonic Sensor is ready to send echo 
      
   count_start = RTC->MODE0.COUNT.reg;            // get start time

   // take the echo input from PA16
   while (PORT->Group[0].IN.reg & 0x00010000);      // send echo & wait until echo signal is end
      
   count_end   = RTC->MODE0.COUNT.reg;            // get end time
   RTC_count = count_end - count_start;         // get time period for echo signal
   echo_time_interval = RTC_count / 8 ;         // echo interval in usec (8MHz clock input)
   distance = (echo_time_interval / 2) * 0.034 ;   // distance in cm
   
   distance_return = distance;
      
   print_decimal(distance / 100);               // print left most decimal
   distance = distance % 100;                  // get rest 2 decimals
   print_decimal(distance / 10);               // print left most decimal
   print_decimal(distance % 10);               // print right most decimal
   print_enter();                           // print next line
   
   return distance_return;                     // return distance 
}


void USART_setup() {
   
   //
   // PORT setup for PB22 and PB23 (USART)
   //
   PORT->Group[1].PINCFG[22].reg = 0x41;         // peripheral mux: DRVSTR=1, PMUXEN = 1
   PORT->Group[1].PINCFG[23].reg = 0x41;         // peripheral mux: DRVSTR=1, PMUXEN = 1

   PORT->Group[1].PMUX[11].bit.PMUXE = 0x03;      // peripheral function D selected
   PORT->Group[1].PMUX[11].bit.PMUXO = 0x03;      // peripheral function D selected

   // Power Manager
   PM->APBCMASK.bit.SERCOM5_ = 1 ;               // Clock Enable (APBC clock) for USART
   
   //
   // * SERCOM5: USART
   // * Generator #0 is feeding USART as well
   //
   GCLK->CLKCTRL.bit.ID = 0x19;               // ID #0x19 (SERCOM5: USART): GCLK_SERCOM3_CORE
   GCLK->CLKCTRL.bit.GEN = 0;                  // Generator #0 selected for USART
   GCLK->CLKCTRL.bit.CLKEN = 1;               // Now, clock is supplied to USART!

   GCLK->CLKCTRL.bit.ID = 0x13;               // ID #0x13 (SERCOM5: USART): GCLK_SERCOM_SLOW
   GCLK->CLKCTRL.bit.GEN = 0;                  // Generator #0 selected for USART
   GCLK->CLKCTRL.bit.CLKEN = 1;               // Now, clock is supplied to USART!
   
   //
   // USART setup
   //
   SERCOM5->USART.CTRLA.bit.MODE = 1 ;            // Internal Clock
   SERCOM5->USART.CTRLA.bit.CMODE = 0 ;         // Asynchronous UART
   SERCOM5->USART.CTRLA.bit.RXPO = 3 ;            // PAD3
   SERCOM5->USART.CTRLA.bit.TXPO = 1 ;            // PAD2
   SERCOM5->USART.CTRLB.bit.CHSIZE = 0 ;         // 8-bit data
   SERCOM5->USART.CTRLA.bit.DORD = 1 ;            // LSB first

   SERCOM5->USART.BAUD.reg = 0Xc504 ;            // 115,200 bps (baud rate) with 8MHz input clock

   SERCOM5->USART.CTRLB.bit.RXEN = 1 ;
   SERCOM5->USART.CTRLB.bit.TXEN = 1 ;

   SERCOM5->USART.CTRLA.bit.ENABLE = 1;
}

void TC3_setup()
{
   PORT->Group[0].PINCFG[18].reg = 0x41;         // peripheral mux: DRVSTR=1, PMUXEN = 1
   PORT->Group[0].PINCFG[19].reg = 0x41;         // peripheral mux: DRVSTR=1, PMUXEN = 1
   
   PORT->Group[0].PMUX[9].reg = 0x44;            // function E: TC3/WO
   
   GCLK->CLKCTRL.bit.ID = 0x1B;               // ID: TCC2, TC3
   GCLK->CLKCTRL.bit.GEN = 0;                  // generator 0 selected
   GCLK->CLKCTRL.bit.CLKEN = 1;               // clock supplied
   
   PM->APBCMASK.bit.TC3_ = 1;                  // clock enable (APBC clock) for TC3
   
   TC3->COUNT16.CTRLA.bit.MODE = 0;            // Count16 mode
   TC3->COUNT16.CTRLA.bit.WAVEGEN = 3;            // match pwm
   TC3->COUNT16.CTRLA.bit.PRESCALER = 6;         // prescaler = 256
   TC3->COUNT16.COUNT.reg = 0;                  // count from 0
   TC3->COUNT16.CC[0].reg = 1000;               // cc0 = 1000(period of counter)
   TC3->COUNT16.CC[1].reg = 0;                  // cc1 = 0(on duration)
   TC3->COUNT16.CTRLA.bit.ENABLE = 1;            // start counter
}

void TC4_setup()
{
   PORT->Group[0].PINCFG[22].reg = 0x41;         // peripheral mux: DRVSTR=1, PMUXEN = 1
   PORT->Group[0].PINCFG[23].reg = 0x41;         // peripheral mux: DRVSTR=1, PMUXEN = 1
   
   PORT->Group[0].PMUX[11].reg = 0x44;            // function E: TC4/WO
   
   GCLK->CLKCTRL.bit.ID = 0x1C;               // ID: TC4, TC5
   GCLK->CLKCTRL.bit.GEN = 0;                  // generator 0
   GCLK->CLKCTRL.bit.CLKEN = 1;               // enable clock to TC4
   
   PM->APBCMASK.bit.TC4_ = 1;                  // clock enable (APBC clock) for TC4
   
   TC4->COUNT16.CTRLA.bit.MODE = 0;            // Count16 mode
   TC4->COUNT16.CTRLA.bit.WAVEGEN = 3;            // match pwm
   TC4->COUNT16.CTRLA.bit.PRESCALER = 6;         // prescaler = 256
   TC4->COUNT16.COUNT.reg = 0;                  // count from 0
   TC4->COUNT16.CC[0].reg = 1000;               // cc0 = 1000(period of counter)
   TC4->COUNT16.CC[1].reg = 0;                  // cc1 = 0(on duration)
   TC4->COUNT16.CTRLA.bit.ENABLE = 1;            // start counter
}