/*
 * uart_print_api.c
 *
 * Created: 2024-01-01 6:58:39 PM
 *  Author: suhtw
 */ 

#pragma GCC target ("thumb")

#include "sam.h"

void print_decimal(unsigned int data) {

	unsigned char conv_data;

	while (SERCOM0->USART.INTFLAG.bit.DRE != 1) ; // DRE (Data Register Empty) is set when DATA is empty

	conv_data = (unsigned char) (data & 0x000000FF)  ; //
	conv_data += 48 ; // 0, 1, ... 9
	
	SERCOM0->USART.DATA.reg	= conv_data ; // transmit the first number (ASCII code)
	
};

void print_unsigned_int(unsigned data) {

	unsigned size ;
	
	if      ( data >= 1000000000)                        size = 10;
	else if ((data >= 100000000) && (data <= 999999999)) size = 9;
	else if ((data >= 10000000)  && (data <= 99999999))  size = 8;
	else if ((data >= 1000000)   && (data <= 9999999))   size = 7;
	else if ((data >= 100000)    && (data <= 999999))    size = 6;
	else if ((data >= 10000)     && (data <= 99999))     size = 5;
	else if ((data >= 1000)      && (data <= 9999))      size = 4;
	else if ((data >= 100)       && (data <= 999))       size = 3;
	else if ((data >= 10)        && (data <= 99))        size = 2;
	else if (                        data <= 9)          size = 1;

	while (size != 0) {
		switch (size) {
			case 10: print_decimal(data / 1000000000); data = data % 1000000000; break;
			case 9:  print_decimal(data / 100000000);  data = data % 100000000;  break;
			case 8:  print_decimal(data / 10000000);   data = data % 10000000;   break;
			case 7:  print_decimal(data / 1000000);    data = data % 1000000;    break;
			case 6:  print_decimal(data / 100000);     data = data % 100000;     break;
			case 5:  print_decimal(data / 10000);      data = data % 10000;      break;
			case 4:  print_decimal(data / 1000);       data = data % 1000;       break;
			case 3:  print_decimal(data / 100);        data = data % 100;        break;
			case 2:  print_decimal(data / 10);         data = data % 10;         break;
			case 1:  print_decimal(data);                                        break;
			default:															 break;
		}
		size--;
	}
};


void print_enter() {

	while (SERCOM0->USART.INTFLAG.bit.DRE != 1) ; // DRE (Data Register Empty) is set when DATA is empty
	SERCOM0->USART.DATA.reg	= 10 ; // transmit LF (Line Feed)
	//
	while (SERCOM0->USART.INTFLAG.bit.DRE != 1) ;
	SERCOM0->USART.DATA.reg	= 13 ; // transmit CR (Carriage Return)
};

void print_string(unsigned char * msg, unsigned int msg_size) {

	unsigned count;

	count = 0;
	
	while (1) {
		if (SERCOM0->USART.INTFLAG.bit.DRE == 1) {
			if (count == msg_size)  break;
			else {
				SERCOM0->USART.DATA.reg	= msg[count];
				count++;
			}
		}
	};
}

void println_string(unsigned char * msg, unsigned int msg_size) {

	unsigned count;

	count = 0;
	
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
}

// int → 문자열 변환 (음수 포함)
// buf는 최소 12바이트 이상 (정수 10자리 + 부호 + 널문자)
void int_to_str(int value, char *buf) {
	int is_negative = 0;
	int i = 0;

	if (value == 0) {
		buf[i++] = '0';
		buf[i] = '\0';
		return;
	}

	if (value < 0) {
		is_negative = 1;
		value = -value;
	}

	// 숫자를 역순으로 저장
	while (value > 0) {
		buf[i++] = (value % 10) + '0';
		value /= 10;
	}

	if (is_negative) {
		buf[i++] = '-';
	}

	// 역순을 뒤집기
	for (int j = 0; j < i / 2; j++) {
		char tmp = buf[j];
		buf[j] = buf[i - j - 1];
		buf[i - j - 1] = tmp;
	}

	buf[i] = '\0';
}
