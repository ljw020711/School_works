/*
 * class_project.c
 *
 * Created: 2025-06-20 오후 1:09:51
 * Author : joungwon
 */ 
/*
#pragma GCC target ("thumb")

#include "sam.h"
#include "uart_print.h"
#include "ld19.h"

#define PACKET_COUNT 100

void GCLK_setup();
void USART_setup();
void PORT_setup();
void RTC_setup();
void TC3_setup();
void TC4_setup();
void TC5_setup();
void TCC2_setup();
void turn_left();
void turn_right();
void print_lidardata(LiDARFrameTypeDef *);
void initializeSystemFor48MHz();
void I2C_setup();
void LIS2DH_I2C_write(unsigned char, unsigned char);
unsigned char LIS2DH_I2C_read(unsigned char);
void LIS2DH_I2C_read_multiple_data(unsigned char, unsigned char, unsigned char *);
void bytes_to_ints(unsigned char * buffer, short * x_mg, short * y_mg, short * z_mg);
void print_byte(unsigned char data);

LiDARFrameTypeDef ld19packet[PACKET_COUNT];

float previous_speed_x, previous_speed_y, previous_speed_z;

int main()
{	
   unsigned count, msg_size;
   char msg[] = {"Hello World!"};
   char rx_data;
   char speed_low, start_angle_low, end_angle_low, timestamp_low, distance_low;
   _Bool	led, rxc_flag, started;
   unsigned num_packet, uart_count, npoint, offset;
   int i = 0;
   uint16_t speed, start_angle, end_angle, distance_lidar;
   
   // Initialize the SAM system
   SystemInit();
   
   initializeSystemFor48MHz();
   
   GCLK_setup();
   
   USART_setup();

   PORT_setup();
   
   TC3_setup();
   
   TC4_setup();
   
   TC5_setup();
   
   RTC_setup();
   
   I2C_setup();
   
   //TCC2_setup();
   
   num_packet = 0;
   count = 0;
   msg_size = sizeof(msg);
   
   while (1) {
	   if (SERCOM0->USART.INTFLAG.bit.DRE == 1) {
		   if (count == msg_size) {
			   SERCOM0->USART.DATA.reg	= 10; // Line Feed
			   while (!SERCOM0->USART.INTFLAG.bit.DRE); // wait until data register is empty
			   SERCOM0->USART.DATA.reg	= 13; // Carriage Return
			   break;
			   } else {
			   SERCOM0->USART.DATA.reg	= msg[count];
			   count++;
		   }
	   }
   };
   
   //
   // Syncing ld19 packets...
   //
   num_packet = 0;
   uart_count = 0;
   started = 0;
   
   NVIC->ISER[0] = (1 << 3) | (1 << 20) | (1 << 17);  // Interrupt Set Enable
   NVIC->IP[5] = 0x10 << 0;            // priority for TC5: IP5[7:0] = 0x80 (=b1000_0000, 2-bit MSBs)
   NVIC->IP[0] = 0x80 << 24;            // priority for RTC: IP5[7:0] = 0x80 (=b1000_0000, 2-bit MSBs)
   NVIC->IP[4] = 0xC0 << 0;            // priority for RTC: IP5[7:0] = 0x80 (=b1000_0000, 2-bit MSBs)
   
   while(1) {
	   
	   rxc_flag = SERCOM2->USART.INTFLAG.bit.RXC; // check out RXC (Receive Complete) flag
	   if (rxc_flag == 1)	{
		   rx_data	= SERCOM2->USART.DATA.reg; // Read the received data
		   //SERCOM5->USART.DATA.reg	= rx_data; // Write the received data (echo back to PC)

		   switch (uart_count)
		   {
			   case 1:	ld19packet[num_packet].ver_len = rx_data;
			   break;
			   case 2: speed_low = rx_data;
			   break;
			   case 3: ld19packet[num_packet].speed = (uint16_t) ((rx_data << 8) | speed_low);
			   break;
			   case 4: start_angle_low = rx_data;
			   break;
			   case 5: ld19packet[num_packet].start_angle = (uint16_t) ((rx_data << 8) | speed_low);
			   break;
			   case 6 ... 41:
			   npoint = (uart_count - 6) / 3 ;
			   offset = (uart_count - 6) % 3 ;
			   if      (offset == 0) distance_low = rx_data;
			   else if (offset == 1) ld19packet[num_packet].point[npoint].distance =
			   (uint16_t) ((rx_data << 8) | distance_low);
			   else if (offset == 2) ld19packet[num_packet].point[npoint].intensity =
			   (uint8_t) rx_data;
			   break;
			   case 42: end_angle_low = rx_data;
			   break;
			   case 43: ld19packet[num_packet].end_angle = (uint16_t) ((rx_data << 8) | end_angle_low);
			   break;
			   case 44: timestamp_low = rx_data;
			   break;
			   case 45: ld19packet[num_packet].timestamp = (uint16_t) ((rx_data << 8) | timestamp_low);
			   break;
			   case 46: break;
			   default: break;
		   }

		   //
		   // * Trying to sync by finding out the header (0x54) and VerLen (0x2C)
		   // * Note that it is not perfectly syncing though.
		   // * There is a slim chance (1 out of 2^16) of falsely syncing
		   //
		   // * uart_count: count from HEADER to CRC check (47 bytes)
		   if ((uart_count == 1) & rx_data == 0x2C) { // VerLen: 0x2C
			   uart_count++;
			   started = 1;
			   } else if (started & (uart_count > 0) & (uart_count < (packet_len-1))) {
			   uart_count++;
			   } else if (started & (uart_count == (packet_len-1))) {
			   started = 0;
			   uart_count = 0;
			   } else if (rx_data == PKG_HEADER) { // header: 0x54
			   uart_count = 0;
			   uart_count++;
		   }
		   
		   if (num_packet == PACKET_COUNT)	{
			   num_packet = 0;
			  // print_lidardata(ld19packet);
			   //break;
			   } else if (uart_count == (packet_len-1)) {
			   num_packet++;
		   }
		   
	   };
   }; // end of while

   return (0);
}


void GCLK_setup() {
   
   // OSC8M
   SYSCTRL->OSC8M.bit.PRESC = 0;  // prescalar to 1
   SYSCTRL->OSC8M.bit.ONDEMAND = 0;   // oscillator always on

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
   
   GCLK->CLKCTRL.bit.ID = 0x14; // ID #0x14 (SERCOM0: USUART)
   GCLK->CLKCTRL.bit.GEN = 0; // Generator #2 selected for USART
   GCLK->CLKCTRL.bit.CLKEN = 1; // Now, clock is supplied to USART!

   GCLK->CLKCTRL.bit.ID = 0x16; // ID #0x16 (SERCOM2: USUART)
   GCLK->CLKCTRL.bit.GEN = 0; // Generator #2 selected for USART
   GCLK->CLKCTRL.bit.CLKEN = 1; // Now, clock is supplied to USART!

}

void PORT_setup() {
   
   // PORT setup for PA18, PA19, PB08, PB09 for TC3, TC4 (motor driver)
   PORT->Group[0].PINCFG[18].reg = 0x41;      // peripheral mux: DRVSTR=1, PMUXEN = 1
   PORT->Group[0].PINCFG[19].reg = 0x41;      // peripheral mux: DRVSTR=1, PMUXEN = 1
   PORT->Group[0].PMUX[9].bit.PMUXE = 0x4;      // peripheral function E
   PORT->Group[0].PMUX[9].bit.PMUXO = 0x4;      // peripheral function E
   
   PORT->Group[1].PINCFG[8].reg = 0x41;      // peripheral mux: DRVSTR=1, PMUXEN = 1
   PORT->Group[1].PINCFG[9].reg = 0x41;      // peripheral mux: DRVSTR=1, PMUXEN = 1
   PORT->Group[1].PMUX[4].bit.PMUXE = 0x4;   // peripheral function E
   PORT->Group[1].PMUX[4].bit.PMUXO = 0x4;   // peripheral function E
   
   // PORT setup for PA04, PA05, PA08, PA09 for the direction of the motor
   PORT->Group[0].PINCFG[4].reg = 0x0;         // PMUXEN = 0, LEFT WHEEL (BACKWARD)
   PORT->Group[0].PINCFG[5].reg = 0x0;         // PMUXEN = 0, LEFT WHEEL (FORWARD)
   PORT->Group[0].PINCFG[8].reg = 0x0;         // PMUXEN = 0, RIGHT WHEEL (BACKWARD)
   PORT->Group[0].PINCFG[9].reg = 0x0;         // PMUXEN = 0, RIGHT WHEEL (FORWARD)
   
   PORT->Group[0].DIRSET.reg = 0X3 << 8;      // use PA04, PA05, PA08, PA09 as output pin
   PORT->Group[0].DIRSET.reg = 0X3 << 4;      // use PA04, PA05, PA08, PA09 as output pin
   PORT->Group[0].OUTSET.reg = 0X2 << 8;      // PA05 : 1, PA04 : 0, PA09 : 1, PA08 : 0
   PORT->Group[0].OUTSET.reg = 0X2 << 4;      // PA05 : 1, PA04 : 0, PA09 : 1, PA08 : 0
   
   // PORT setup for PA10 and PA11 (USART): SERCOM2
   PORT->Group[0].PINCFG[10].reg = 0x41; // peripheral mux: DRVSTR=1, PMUXEN = 1
   PORT->Group[0].PINCFG[11].reg = 0x41; // peripheral mux: DRVSTR=1, PMUXEN = 1

   PORT->Group[0].PMUX[5].bit.PMUXE = 0x03; // peripheral function D selected
   PORT->Group[0].PMUX[5].bit.PMUXO = 0x03; // peripheral function D selected
   
   // PORT setup for PA06 and PA07 (USART)
   PORT->Group[0].PINCFG[6].reg = 0x41; // peripheral mux: DRVSTR=1, PMUXEN = 1
   PORT->Group[0].PINCFG[7].reg = 0x41; // peripheral mux: DRVSTR=1, PMUXEN = 1

   PORT->Group[0].PMUX[3].bit.PMUXE = 0x03; // peripheral function D selected
   PORT->Group[0].PMUX[3].bit.PMUXO = 0x03; // peripheral function D selected
}

void RTC_setup() {
   //
   // RTC setup: MODE0 (32-bit counter) with COMPARE 0
   //

   RTC->MODE0.CTRL.bit.ENABLE = 0; // Disable first
   RTC->MODE0.CTRL.bit.MODE = 0; // Mode 0
   RTC->MODE0.CTRL.bit.MATCHCLR = 1; // match clear
   
   RTC->MODE0.CTRL.bit.PRESCALER = 0x8;
   RTC->MODE0.COMP->reg = 2000; // compare register to set up 10usec interval
   RTC->MODE0.COUNT.reg = 0x0; // initialize the counter to 0
   RTC->MODE0.INTENSET.bit.CMP0 = 1;
   RTC->MODE0.CTRL.bit.ENABLE = 1; // Enable
}

void USART_setup() {

	// Power Manager
	PM->APBCMASK.bit.SERCOM0_ = 1 ; // Clock Enable (APBC clock)
	PM->APBCMASK.bit.SERCOM2_ = 1 ; // Clock Enable (APBC clock)
	
	// USART setup
	SERCOM0->USART.CTRLA.bit.MODE = 1 ; // Internal Clock
	SERCOM0->USART.CTRLA.bit.CMODE = 0 ; // Asynchronous UART
	SERCOM0->USART.CTRLA.bit.RXPO = 3 ; // PAD3
	SERCOM0->USART.CTRLA.bit.TXPO = 1 ; // PAD2
	SERCOM0->USART.CTRLB.bit.CHSIZE = 0 ; // 8-bit data
	SERCOM0->USART.CTRLA.bit.DORD = 1 ; // LSB first

	SERCOM0->USART.BAUD.reg = 0Xfb15 ; // 9,600 bps (baud rate) with 8MHz input clock

	SERCOM0->USART.CTRLB.bit.RXEN = 1 ;
	SERCOM0->USART.CTRLB.bit.TXEN = 1 ;

	SERCOM0->USART.CTRLA.bit.ENABLE = 1;
	
	// USART setup: SERCOM2
	SERCOM2->USART.CTRLA.bit.MODE = 1 ; // Internal Clock
	SERCOM2->USART.CTRLA.bit.CMODE = 0 ; // Asynchronous UART
	SERCOM2->USART.CTRLA.bit.RXPO = 3 ; // PAD3
	SERCOM2->USART.CTRLA.bit.TXPO = 1 ; // PAD2
	SERCOM2->USART.CTRLB.bit.CHSIZE = 0 ; // 8-bit data
	SERCOM2->USART.CTRLA.bit.DORD = 1 ; // LSB first

	SERCOM2->USART.BAUD.reg = 0X8a09 ; // 230,400 bps (baud rate) with 8MHz input clock
	SERCOM2->USART.CTRLB.bit.RXEN = 1 ;
	SERCOM2->USART.CTRLB.bit.TXEN = 1 ;

	SERCOM2->USART.CTRLA.bit.ENABLE = 1;	
	
}

void TC3_setup()
{
   GCLK->CLKCTRL.bit.ID = 0x1B;            // select TC3
   GCLK->CLKCTRL.bit.GEN = 0;            // select generator 0
   GCLK->CLKCTRL.bit.CLKEN = 1;            // enable clock (8MHz)
   
   PM->APBCMASK.bit.TC3_ = 1;            // clock enable (APBC clock) for TC3
   
   TC3->COUNT16.CTRLA.bit.MODE = 0;         // 16bit counter mode
   TC3->COUNT16.CTRLA.bit.WAVEGEN = 3;      // match pwm
   TC3->COUNT16.CTRLA.bit.PRESCALER = 6;   // prescaler 256
   
   TC3->COUNT16.COUNT.reg = 0;            // count start from 0
   TC3->COUNT16.CC[0].reg = 1000;         // total period = 1000
   TC3->COUNT16.CC[1].reg = 450;            // on duration = 0
   TC3->COUNT16.CTRLA.bit.ENABLE = 1;      // enable TC3
}

void TC4_setup()
{
   GCLK->CLKCTRL.bit.ID = 0x1C;            // select TC4, TC5
   GCLK->CLKCTRL.bit.GEN = 0;            // select generator 0
   GCLK->CLKCTRL.bit.CLKEN = 1;            // enable clock (8MHz)
   
   PM->APBCMASK.bit.TC4_ = 1;            // clock enable (APBC clock) for TC4
   
   TC4->COUNT16.CTRLA.bit.MODE = 0;         // 16bit counter mode
   TC4->COUNT16.CTRLA.bit.WAVEGEN = 3;      // match pwm
   TC4->COUNT16.CTRLA.bit.PRESCALER = 6;   // prescaler 256
   
   TC4->COUNT16.COUNT.reg = 0;            // count start from 0
   TC4->COUNT16.CC[0].reg = 1000;         // total period = 1000
   TC4->COUNT16.CC[1].reg = 450;            // on duration = 0
   TC4->COUNT16.CTRLA.bit.ENABLE = 1;      // enable tc4
}

void TC5_setup()
{
   PM->APBCMASK.bit.TC5_ = 1;               // clock enable (APBC clock) for TC5
   
   TC5->COUNT16.CTRLA.bit.MODE = 0;         // 16bit counter mode
   TC5->COUNT16.CTRLA.bit.WAVEGEN = 1;      // MFRQ
   TC5->COUNT16.CTRLA.bit.PRESCALER = 6;    // prescaler = 256, 8MHz / 256 = 31250
   
   TC5->COUNT16.INTENSET.bit.OVF = 1;       // Enable interrupt
   
   TC5->COUNT16.COUNT.reg = 0;            // count from 0
   TC5->COUNT16.CC[0].reg = 3125;          // count until cc0 and overflow(interrupt)
   TC5->COUNT16.CTRLA.bit.ENABLE = 1;       // start counter
}

unsigned stop = 0;

void TC5_Handler(void) {
	uint16_t start_angle, end_angle, distance_lidar, intensity;
	TC5->COUNT16.INTFLAG.bit.OVF = 1 ;         // Clear the interrupt flag
	int count = 0;
	
	if(stop)
	{
		return;	
	}
	
	for(int i = 0; i < 100; i++)
	{
		for(int j = 0; j < 12; j++)
		{
			if(distance_lidar >= 9999 || distance_lidar == 0)
			{
				continue;
			}
			
			distance_lidar += ld19packet[i].point[j].distance;
			count++;
		}
		distance_lidar /= count;
		
		intensity =  ld19packet[i].point[6].intensity;
		
		if(distance_lidar >= 9999 || distance_lidar <= 100 || intensity <= 165 && intensity >= 270)
		{
			continue;
		}
		
		start_angle = ld19packet[i].start_angle;
		end_angle = ld19packet[i].end_angle;
		if(start_angle >= 8800 && end_angle <= 9500)
		{
			if(distance_lidar < 220)
			{
				PORT->Group[0].OUTCLR.reg = 0x33 << 4;     //
				PORT->Group[0].OUTSET.reg = 0x21 << 4;     // PA06 : 1, PA07 : 0, PA08 : 0, PA09 : 1 (LEFT FORWARD, RIGHT BACKWARD)
				
				TC3->COUNT16.CC[1].reg = 650;
				TC4->COUNT16.CC[1].reg = 450;
				
				for(int i = 0; i < 32000; i++);
				
				PORT->Group[0].OUTCLR.reg = 0x33 << 4;
				PORT->Group[0].OUTSET.reg = 0X2 << 8;      // PA05 : 1, PA04 : 0, PA09 : 1, PA08 : 0
				PORT->Group[0].OUTSET.reg = 0X2 << 4;      // PA05 : 1, PA04 : 0, PA09 : 1, PA08 : 0
				
				TC3->COUNT16.CC[1].reg = 450;         // mid speed: on duration 600 / 1000
				TC4->COUNT16.CC[1].reg = 450;         // mid speed: on duration 600 / 1000
			}
			else if(distance_lidar > 600)
			{
				turn_right();
			}
			else
			{
				TC3->COUNT16.CC[1].reg = 450;
				TC4->COUNT16.CC[1].reg = 450;
			}
			
		}
		
		if(start_angle >= 3500 && end_angle <= 3700)
		{
			if(distance_lidar > 200)
			{
				PORT->Group[0].OUTCLR.reg = 0x33 << 4;     //
				PORT->Group[0].OUTSET.reg = 0x12 << 4;     // PA06 : 1, PA07 : 0, PA08 : 0, PA09 : 1 (LEFT FORWARD, RIGHT BACKWARD)
				
				TC3->COUNT16.CC[1].reg = 450;
				TC4->COUNT16.CC[1].reg = 450;
				
				for(int i = 0; i < 20000; i++);
				
				PORT->Group[0].OUTCLR.reg = 0x33 << 4;
				PORT->Group[0].OUTSET.reg = 0X2 << 8;      // PA05 : 1, PA04 : 0, PA09 : 1, PA08 : 0
				PORT->Group[0].OUTSET.reg = 0X2 << 4;      // PA05 : 1, PA04 : 0, PA09 : 1, PA08 : 0
				
				TC3->COUNT16.CC[1].reg = 450;         // mid speed: on duration 600 / 1000
				TC4->COUNT16.CC[1].reg = 450;         // mid speed: on duration 600 / 1000
			}
			else if(distance_lidar < 160)
			{
				PORT->Group[0].OUTCLR.reg = 0x33 << 4;     //
				PORT->Group[0].OUTSET.reg = 0x21 << 4;     // PA06 : 1, PA07 : 0, PA08 : 0, PA09 : 1 (LEFT FORWARD, RIGHT BACKWARD)
				
				TC3->COUNT16.CC[1].reg = 650;
				TC4->COUNT16.CC[1].reg = 450;
				
				for(int i = 0; i < 30000; i++);
				
				PORT->Group[0].OUTCLR.reg = 0x33 << 4;
				PORT->Group[0].OUTSET.reg = 0X2 << 8;      // PA05 : 1, PA04 : 0, PA09 : 1, PA08 : 0
				PORT->Group[0].OUTSET.reg = 0X2 << 4;      // PA05 : 1, PA04 : 0, PA09 : 1, PA08 : 0
				
				TC3->COUNT16.CC[1].reg = 450;         // mid speed: on duration 600 / 1000
				TC4->COUNT16.CC[1].reg = 450;         // mid speed: on duration 600 / 1000
			}
		}
	}
	
}

unsigned turning = 0;

void RTC_Handler(void) {
	RTC->MODE0.INTFLAG.bit.CMP0 = 1;
	
	uint16_t start_angle, end_angle, distance_lidar, intensity;
	uint16_t front, left, right;
	
	int changed = 0;
	int changed2 = 0;
	int changed3 = 0;
	
	for(int i = 0; i < 100; i++)
	{
		start_angle = ld19packet[i].start_angle;
		end_angle = ld19packet[i].end_angle;
		distance_lidar = ld19packet[i].point[6].distance;
		intensity = ld19packet[i].point[6].intensity;
		
		if(distance_lidar >= 8000 || distance_lidar == 0 || distance_lidar < 100 || intensity <= 165 && intensity >= 270)
		{
			continue;
		}
		
		if(start_angle >= 35000 && end_angle <= 260)
		{
			front = distance_lidar;
			changed = 1;
		}
		
		if(start_angle >= 8200 && start_angle <= 9500)
		{
			right = distance_lidar;
			changed2 = 1;
		}
		
		if(start_angle >= 26000 && start_angle <= 28500)
		{
			left = distance_lidar;
			changed3 = 1;
		}
	}
	
	if((turning == 0 || turning == 1) && front < 350)
	{
		TC3->COUNT16.CC[1].reg = 0;
		TC4->COUNT16.CC[1].reg = 0;
		
		stop = 1;
		if(right > 200 && left > 200)
		{
			turn_right();
		}
		else if(right > left)
		{
			turn_right();
		}
		else if(left > right)
		{
			turn_left();
		}
	}
	
	if(turning == 1 && front > 350)
	{
		turning = 0;
		PORT->Group[0].OUTCLR.reg = 0x33 << 4;
		PORT->Group[0].OUTSET.reg = 0X2 << 8;      // PA05 : 1, PA04 : 0, PA09 : 1, PA08 : 0
		PORT->Group[0].OUTSET.reg = 0X2 << 4;      // PA05 : 1, PA04 : 0, PA09 : 1, PA08 : 0
		
		TC3->COUNT16.CC[1].reg = 450;         // mid speed: on duration 600 / 1000
		TC4->COUNT16.CC[1].reg = 450;         // mid speed: on duration 600 / 1000
		
		stop = 0;
	}
}

void turn_right()
{
	uint16_t speed, start_angle, end_angle, distance_lidar;
	
	turning = 1;
	PORT->Group[0].OUTCLR.reg = 0x33 << 4;     //
	PORT->Group[0].OUTSET.reg = 0x12 << 4;      // PA04 : 0, PA05 : 1, PA08 : 1, PA09 : 0 (LEFT BACKWARD, RIGHT FORWARD)
	
	TC3->COUNT16.CC[1].reg = 480;         // mid speed: on duration 600 / 1000
	TC4->COUNT16.CC[1].reg = 480;         // mid speed: on duration 600 / 1000
	
	//for(int i = 0; i < 300000; i++);
}

void turn_left()
{
	uint16_t speed, start_angle, end_angle, distance_lidar;
	
	turning = 1;
	PORT->Group[0].OUTCLR.reg = 0x33 << 4;     //
	PORT->Group[0].OUTSET.reg = 0x21 << 4;     // PA06 : 1, PA07 : 0, PA08 : 0, PA09 : 1 (LEFT FORWARD, RIGHT BACKWARD)
	
	TC3->COUNT16.CC[1].reg = 450;         // mid speed: on duration 600 / 1000
	TC4->COUNT16.CC[1].reg = 450;         // mid speed: on duration 600 / 1000
	
	//for(int i = 0; i < 300000; i++);
}

void print_lidardata(LiDARFrameTypeDef * ld19data) {
	
	uint16_t speed, start_angle, end_angle, distance;
	unsigned char speed_msg[] = " speed = ";
	unsigned char startangle_msg[] = " start angle = ";
	unsigned char endangle_msg[] = " end angle = ";
	unsigned char point_msg[] = " point = ";
	int i;
	
	for (i=0; i<100; i++) {
		print_string(speed_msg, sizeof(speed_msg));
		speed = ld19data[i].speed;
		print_unsigned_int(speed);

		print_string(startangle_msg, sizeof(startangle_msg));
		start_angle = ld19data[i].start_angle;
		print_unsigned_int(start_angle);

		print_string(endangle_msg, sizeof(endangle_msg));
		end_angle = ld19data[i].end_angle;
		print_unsigned_int(end_angle);

		print_string(point_msg, sizeof(point_msg));
		distance = ld19data[i].point[0].distance;
		print_unsigned_int(distance);
		
		print_enter();
	}
}

void initializeSystemFor48MHz()
{
	// Change the timing of the NVM access
	NVMCTRL->CTRLB.bit.RWS = NVMCTRL_CTRLB_RWS_HALF_Val; // 1 wait state for operating at 2.7-3.3V at 48MHz.

	// Enable the bus clock for the clock system.
	//PM->APBAMASK.bit.GCLK_ = true;
	PM->APBAMASK.bit.GCLK_ = 1;

	// Initialise the DFLL to run in closed-loop mode at 48MHz
	// 1. Make a software reset of the clock system.
	//GCLK->CTRL.bit.SWRST = true;
	GCLK->CTRL.bit.SWRST = 1;
	while (GCLK->CTRL.bit.SWRST && GCLK->STATUS.bit.SYNCBUSY) {};
	// 2. Make sure the OCM8M keeps running.
	SYSCTRL->OSC8M.bit.ONDEMAND = 0;
	
	// 3. Set the division factor to 64, which reduces the 1MHz source to 15.625kHz
	GCLK->GENDIV.reg =
	GCLK_GENDIV_ID(3) | // Select generator 3
	GCLK_GENDIV_DIV(64); // Set the division factor to 64
	
	// 4. Create generic clock generator 3 for the 15KHz signal of the DFLL
	GCLK->GENCTRL.reg =
	GCLK_GENCTRL_ID(3) | // Select generator 3
	GCLK_GENCTRL_SRC_OSC8M | // Select source OSC8M
	GCLK_GENCTRL_GENEN; // Enable this generic clock generator
	while (GCLK->STATUS.bit.SYNCBUSY) {}; // Wait for synchronization
	
	// 5. Configure DFLL with the
	GCLK->CLKCTRL.reg =
	//GCLK_CLKCTRL_ID_DFLL48M | // Target is DFLL48M
	0x00 | // Target is DFLL48M
	GCLK_CLKCTRL_GEN(3) | // Select generator 3 as source.
	GCLK_CLKCTRL_CLKEN; // Enable the DFLL48M
	while (GCLK->STATUS.bit.SYNCBUSY) {}; // Wait for synchronization
	
	// 6. Workaround to be able to configure the DFLL.
	//SYSCTRL->DFLLCTRL.bit.ONDEMAND = false;
	SYSCTRL->DFLLCTRL.bit.ONDEMAND = 0;
	while (!SYSCTRL->PCLKSR.bit.DFLLRDY) {}; // Wait for synchronization.
	
	// 7. Change the multiplication factor.
	SYSCTRL->DFLLMUL.bit.MUL = 3072; // 48MHz / (1MHz / 64)
	SYSCTRL->DFLLMUL.bit.CSTEP = 1; // Coarse step = 1
	SYSCTRL->DFLLMUL.bit.FSTEP = 1; // Fine step = 1
	while (!SYSCTRL->PCLKSR.bit.DFLLRDY) {}; // Wait for synchronization.
	
	// 8. Start closed-loop mode
	SYSCTRL->DFLLCTRL.reg |=
	SYSCTRL_DFLLCTRL_MODE | // 1 = Closed loop mode.
	SYSCTRL_DFLLCTRL_QLDIS; // 1 = Disable quick lock.
	while (!SYSCTRL->PCLKSR.bit.DFLLRDY) {}; // Wait for synchronization.
	
	// 9. Clear the lock flags.
	SYSCTRL->INTFLAG.bit.DFLLLCKC = 1;
	SYSCTRL->INTFLAG.bit.DFLLLCKF = 1;
	SYSCTRL->INTFLAG.bit.DFLLRDY = 1;
	
	// 10. Enable the DFLL
	//SYSCTRL->DFLLCTRL.bit.ENABLE = true;
	SYSCTRL->DFLLCTRL.bit.ENABLE = 1;
	while (!SYSCTRL->PCLKSR.bit.DFLLRDY) {}; // Wait for synchronization.
	
	// 11. Wait for the fine and coarse locks.
	while (!SYSCTRL->INTFLAG.bit.DFLLLCKC && !SYSCTRL->INTFLAG.bit.DFLLLCKF) {};
	
	// 12. Wait until the DFLL is ready.
	while (!SYSCTRL->INTFLAG.bit.DFLLRDY) {};

	// Switch the main clock speed.
	// 1. Set the divisor of generic clock 0 to 0
	GCLK->GENDIV.reg =
	GCLK_GENDIV_ID(0) | // Select generator 0
	GCLK_GENDIV_DIV(0);
	while (GCLK->STATUS.bit.SYNCBUSY) {}; // Wait for synchronization
	
	// 2. Switch generic clock 0 to the DFLL
	GCLK->GENCTRL.reg =
	GCLK_GENCTRL_ID(0) | // Select generator 0
	GCLK_GENCTRL_SRC_DFLL48M | // Select source DFLL
	GCLK_GENCTRL_IDC | // Set improved duty cycle 50/50
	GCLK_GENCTRL_GENEN; // Enable this generic clock generator
	GCLK->GENCTRL.bit.OE = 1 ;  // Output Enable: GCLK_I
	
	// Prof. Suh added start
	//GCLK->GENDIV.bit.DIV = 8 ;  // 48MHz / 8 (divided by 8), check out DIVSEL for details
	//GCLK->GENCTRL.bit.DIVSEL = 0;  //
	// Prof. Suh added end
	
	while (GCLK->STATUS.bit.SYNCBUSY) {}; // Wait for synchronization
}

void I2C_setup() {

	//
	// PORT setup for PA22 and PA23 (I2C)
	//

	PORT->Group[0].PINCFG[22].reg = 0x41; // peripheral mux: DRVSTR=1, PMUXEN = 1
	PORT->Group[0].PINCFG[23].reg = 0x41; // peripheral mux: DRVSTR=1, PMUXEN = 1

	PORT->Group[0].PMUX[11].bit.PMUXE = 0x02; // peripheral function C selected
	PORT->Group[0].PMUX[11].bit.PMUXO = 0x02; // peripheral function C selected

	// Power Manager
	PM->APBCMASK.bit.SERCOM3_ = 1 ; // Clock Enable for I2C
	
	//
	// * SERCOM3: I2C
	// * Generator #0 is feeding I2C

	GCLK->CLKCTRL.bit.ID = 0x17; // ID #17 (SERCOM3: I2C)
	GCLK->CLKCTRL.bit.GEN = 0; // Generator #0 selected for I2C
	GCLK->CLKCTRL.bit.CLKEN = 1; // Now, clock is supplied to I2C!
	
	// I2C Setup (Host mode)
	SERCOM3->I2CM.CTRLA.bit.SWRST = 1 ; // software reset
	SERCOM3->I2CM.CTRLA.bit.ENABLE = 0 ; // Disable
	//	SERCOM3->I2CM.CTRLA.bit.LOWTOUTEN = 1 ; // SCL Low Timeout Enable

	SERCOM3->I2CM.CTRLA.bit.MODE = 0x5 ; // Host mode
	//SERCOM3->I2CM.BAUD.bit.BAUD = 0x27 ; // 100KHz SCL (0x27 = d'39)
	SERCOM3->I2CM.BAUD.bit.BAUD = 0x20 ; // 100KHz SCL (0x20 = d'32)
	SERCOM3->I2CM.CTRLA.bit.ENABLE = 1 ; // Enable
	SERCOM3->I2CM.STATUS.bit.BUSSTATE = 1 ; // IDLE state
}

void LIS2DH_I2C_write(unsigned char reg_addr, unsigned char data) {
	
	//
	SERCOM3->I2CM.ADDR.bit.ADDR = 0x30 ; // LIS2DH address (0x18) + Write (0)
	while((SERCOM3->I2CM.INTFLAG.bit.MB != 1) || (SERCOM3->I2CM.STATUS.bit.RXNACK != 0));
	SERCOM3->I2CM.DATA.bit.DATA = reg_addr ; //
	while((SERCOM3->I2CM.INTFLAG.bit.MB != 1) || (SERCOM3->I2CM.STATUS.bit.RXNACK != 0));

	//
	SERCOM3->I2CM.DATA.bit.DATA = data ; //
	while((SERCOM3->I2CM.INTFLAG.bit.MB != 1) || (SERCOM3->I2CM.STATUS.bit.RXNACK != 0));
	
}

unsigned char LIS2DH_I2C_read(unsigned char reg_addr) {
	
	//
	SERCOM3->I2CM.ADDR.bit.ADDR = 0x30 ; // LIS2DH address (0x18) + Write (0)
	// MB (Host on Bus) is set when a byte is transmitted in Host Write mode.
	// RXNAK (Received Not Acknowledge) indicates whether the last address or data packet sent was acked or not
	while((SERCOM3->I2CM.INTFLAG.bit.MB != 1) || (SERCOM3->I2CM.STATUS.bit.RXNACK != 0));

	SERCOM3->I2CM.DATA.bit.DATA = reg_addr ; //
	while((SERCOM3->I2CM.INTFLAG.bit.MB != 1) || (SERCOM3->I2CM.STATUS.bit.RXNACK != 0));

	//
	SERCOM3->I2CM.ADDR.bit.ADDR = 0x31 ; // LIS2DH address (0x18) + Read (1)
	// SB (Client on Bus) is set when a byte is successfully received in Host Read mode
	while (SERCOM3->I2CM.INTFLAG.bit.SB != 1); // Check out SB (Client on Bus) flag
	
	SERCOM3->I2CM.CTRLB.bit.ACKACT = 1 ; // Send NACK
	SERCOM3->I2CM.CTRLB.bit.CMD = 3 ; // Execute ACK succeeded by STOP condition
	return (SERCOM3->I2CM.DATA.bit.DATA); //
}

void LIS2DH_I2C_read_multiple_data(unsigned char reg_addr, unsigned char size, unsigned char * buffer) {
	
	//
	SERCOM3->I2CM.ADDR.bit.ADDR = 0x30 ; // LIS2DH address (0x18) + Write (0)
	while((SERCOM3->I2CM.INTFLAG.bit.MB != 1) || (SERCOM3->I2CM.STATUS.bit.RXNACK != 0));

	SERCOM3->I2CM.DATA.bit.DATA = reg_addr | 0x80; // MSB in register address should be '1'
	while((SERCOM3->I2CM.INTFLAG.bit.MB != 1) || (SERCOM3->I2CM.STATUS.bit.RXNACK != 0));

	//
	SERCOM3->I2CM.ADDR.bit.ADDR = 0x31 ; // LIS2DH address (0x18) + Read (1)

	while (size != 0 ) {

		while (SERCOM3->I2CM.INTFLAG.bit.SB != 1); // Check out SB (Client on Bus) flag: a byte is successfully received!
		
		*buffer = SERCOM3->I2CM.DATA.reg;
		buffer++;
		
		if (size == 1) {
			SERCOM3->I2CM.CTRLB.bit.ACKACT = 1 ; // Send NACK
			SERCOM3->I2CM.CTRLB.bit.CMD = 3 ; // Execute ACK succeeded by STOP condition
		}
		else {
			SERCOM3->I2CM.CTRLB.bit.ACKACT = 0; // Send ACK
			SERCOM3->I2CM.CTRLB.bit.CMD = 2 ; // Execute ACK
			
		}
		
		size--;
	};
}

void bytes_to_ints(unsigned char * buffer, short * x_mg, short * y_mg, short * z_mg) {

	short x, y, z ;

	x = (short) ((buffer[1] << 8) | buffer[0]) >> 6; // 10-bit data
	y = (short) ((buffer[3] << 8) | buffer[2]) >> 6; // 10-bit data
	z = (short) ((buffer[5] << 8) | buffer[4]) >> 6; // 10-bit data
	
	*x_mg = x << 2 ; // in normal mode: +-2g, 4mg/digit
	*y_mg = y << 2 ; // in normal mode: +-2g, 4mg/digit
	*z_mg = z << 2 ; // in normal mode: +-2g, 4mg/digit
}

void TCC2_setup()
{
	PM->APBCMASK.bit.TCC2_ = 1;               // clock enable (APBC clock) for TCC2
	
	TCC2->CTRLA.reg = 0;
	TCC2->CTRLA.bit.PRESCALER = 6;			 // prescaler = 256, 8MHz / 256 = 31250
	
	TCC2->WAVE.bit.WAVEGEN = 0x1;
	while(TCC2->SYNCBUSY.bit.WAVE);

	TCC2->INTENSET.bit.OVF = 1;				 // Enable interrupt
	
	TCC2->COUNT.reg = 0;					 // count from 0
	while (TCC2->SYNCBUSY.bit.COUNT);
	
	TCC2->CC[0].reg = 6250;					 // count until cc0 and overflow(interrupt)
	while (TCC2->SYNCBUSY.bit.CC0);
	
	TCC2->CTRLA.bit.ENABLE = 1;				 // start counter
	while (TCC2->SYNCBUSY.bit.ENABLE);
}

void TCC2_Handler(void) {
	
	char rx_data;
	unsigned char reg_data;
	unsigned char buffer[6];
	short x_mg, y_mg, z_mg;
	unsigned char speed_msg[] = {" -- speed measurement --"};
	unsigned char x_msg[] = {"x = "};
	unsigned char y_msg[] = {"y = "};
	unsigned char z_msg[] = {"z = "};
	
	TCC2->INTFLAG.bit.OVF = 1 ;         // Clear the interrupt flag
	
	reg_data = LIS2DH_I2C_read(0x0f); // read from 0x0f (=WHO_AM_I) register

	//  * temperature sensor configuration
	//    set TEMP_EN1, TEMP_EN0 to b'11 in TEMP_CFG_REG (0x1F)
	LIS2DH_I2C_write(0x23, 0x80); // write 0x80 (BDU) to 0x23 (=CTRL_REG4) register
	LIS2DH_I2C_write(0x1f, 0xc0); // write 0xC0 to 0x1F (=TEMP_CFG_REG) register


	// * 3-axis accelerometer configuration
	//   ODR = 0101 --> 100Hz, Normal Mode (LPen = 0, HR = 0), X,Y,Z enable
	LIS2DH_I2C_write(0x20, 0x57); // write 0x50 to 0x20 (=CTRL_REG1) register
	
	// Read temperature
	reg_data = LIS2DH_I2C_read(0x0c); // read from 0x0c (=OUT_TEMP_L) register
	reg_data = LIS2DH_I2C_read(0x0d); // read from 0x0d (=OUT_TEMP_H) register
	
	// Read accelerometer
	LIS2DH_I2C_read_multiple_data(0x28, 6, buffer); //
	bytes_to_ints(buffer, &x_mg, &y_mg, &z_mg);
	
	previous_speed_x = previous_speed_x + (x_mg * 0.0098f * 0.2);
	previous_speed_y = previous_speed_y + (y_mg * 0.0098f * 0.2);
	previous_speed_z = previous_speed_z + (z_mg * 0.0098f * 0.2);

	
	print_string(speed_msg, sizeof(speed_msg));
	print_enter();
	
	print_string(x_msg, sizeof(x_msg));
	if(x_mg < 0)
	{
		x_mg *= -1;
		print_string('-', sizeof('-'));
	}
	print_unsigned_int(x_mg);
	print_enter();
	
	print_string(y_msg, sizeof(y_msg));
	if(y_mg < 0)
	{
		y_mg *= -1;
		print_string('-', sizeof('-'));
	}
	print_unsigned_int(y_mg);
	print_enter();
	
	print_string(z_msg, sizeof(z_msg));
	if(z_mg < 0)
	{
		z_mg *= -1;
		print_string('-', sizeof('-'));
	}
	print_unsigned_int(z_mg);
	print_enter();
	
	
}*/