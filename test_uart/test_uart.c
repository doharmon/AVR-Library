/*
 * test_uart.c
 *
 * Created: 3/17/2016 2:25:27 AM
 *  Author: dharmon
 */ 

#ifdef __AVR_ATmega328P__
#define	 F_CPU	16000000UL
#endif

#ifdef __AVR_ATmega1284P__
#define	 F_CPU	22118400UL
#endif

#include <avr/interrupt.h>
#include <util/delay.h>

#include "../source/lib_uart/uart.h"

//									 1		   2		 3		   4		 5		   6		 7		   8		8 8
//							123456789012345678901234567890123456789012345678901234567890123456789012345678901234567 8 9
const __flash char sz1[] = "This is a very long test message in order to test receive and transmit buffer overflows\r\n";

#ifdef __AVR_ATmega328P__
#define DBGPORT		PORTC
#define DBGDDR		DDRC
#define DBGPIN		PINC
#define DBGPIN0		PINC0
#define DBGPIN1		PINC1
#define DBGPIN2		PINC2
#define DBGPIN3		PINC3
#define DBGPIN4		PINC4
#define DBGPIN5		PINC5

#define FlushOutput	uart0_flushOutput
#define Write		uart0_putc
#endif

#ifdef __AVR_ATmega1284P__
#define DBGPORT		PORTA
#define DBGDDR		DDRA
#define DBGPIN		PINA
#define DBGPIN0		PINA0
#define DBGPIN1		PINA1
#define DBGPIN2		PINA2
#define DBGPIN3		PINA3
#define DBGPIN4		PINA4
#define DBGPIN5		PINA5
#define DBGPIN6		PINA6

#define FlushOutput	uart0_flushOutput
#define Write		uart0_putc
#endif

int main(void)
{
	volatile uint8_t* x;
	uint8_t  y;

	uint8_t	i, j;
	
	// Debug on pins AI0 - AI5 (Port C)
//	DBGPORT	|= _BV(DBGPIN0) | _BV(DBGPIN1) | _BV(DBGPIN2) | _BV(DBGPIN3) | _BV(DBGPIN4) | _BV(DBGPIN5);	// Set pins high
	DBGPORT	= 0;	// Set pins low
	DBGDDR	|= _BV(DBGPIN0) | _BV(DBGPIN1) | _BV(DBGPIN2) | _BV(DBGPIN3) | _BV(DBGPIN4) | _BV(DBGPIN5) | _BV(DBGPIN6);	// Configure as output
	
	DBGPORT = 0xFF;
_delay_ms(250);
	DBGPORT	= 0;	// Set pins low
	

	sei();
/*
while (1)
{	
	y = 0xFF;
	do
	{
		// PIN0 New value
		DBGPIN |= _BV(DBGPIN0);

		// Set memory to value
		for (x = (uint8_t*)RAMEND; x > (uint8_t*)RAMSTART; x--)
		{
			*x = y;
			if (*x != y)
			{
				// PIN1 Error on setting memory
				DBGPIN |= _BV(DBGPIN1);

				while (1)
					;
			}
		}
		
		// Test memory still set to value
		for (x = (uint8_t*)RAMEND; x > (uint8_t*)RAMSTART; x--)
		{
			if (*x != y)
			{
				// PIN2 Error on verifying memory
				DBGPIN |= _BV(DBGPIN2);

				while (1)
				;
			}
		}
		
		y--;
	} while (y != 0xFF);
			
	// PIN3 Success
	DBGPIN |= _BV(DBGPIN3);
}
*/
_delay_ms(1000);
	uart0_init(UART_BAUD_SELECT(57600, F_CPU));

	while (1)
	{
		for (i = 0; i < 10; i++)
		{
			DBGPORT |= _BV(DBGPIN0);
			FlushOutput();
			DBGPORT &= ~_BV(DBGPIN0);
			
			DBGPORT |= _BV(DBGPIN1);
			for (j = 0; j < sizeof(sz1)-1; j++)
			{
				DBGPORT |= _BV(DBGPIN2);
				Write(sz1[j]);
				DBGPORT &= ~_BV(DBGPIN2);
			}
			DBGPORT &= ~_BV(DBGPIN1);
		}
		
/*
		for (i = 0; i < 10; i++)
		{
			DBGPIN |= _BV(DBGPIN3);
			while(Pending())
			;
			DBGPIN |= _BV(DBGPIN3);
			
			DBGPIN |= _BV(DBGPIN4);
			for (j = 0; j < sizeof(sz2)-1; j++)
			{
				DBGPIN |= _BV(DBGPIN5);
				Write(sz2[j]);
			}
			DBGPIN |= _BV(DBGPIN4);
		}
*/
	}
}

