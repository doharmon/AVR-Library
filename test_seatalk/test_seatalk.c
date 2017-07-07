/*
 * test_seatalk.c
 *
 * Created: 3/17/2016 1:06:19 AM
 *  Author: dharmon
 */ 

/* Example hardware timer code
void timeout_init(void)
{	
	// call at startup, let it run until needed
	TCCR1A = 0;		// "Normal" mode
	TCCR1B = (1 << CS12) | (0 << CS11) | (1 << CS10); // prescale /1024
	return;
}
#define	TICKS_PER_SEC (F_CPU/1024)			// ticks/sec with prescale /1024
#define	TIMEOUT_TIME (1 * TICKS_PER_SEC)	// timeout: 1 second
#define	reset_timeout() do { TCNT1 = 0; } while (0)
#define	timeout_event() (TCNT1 >= TIMEOUT_TIME)
*/

#ifdef __AVR_ATmega328P__
#define	 F_CPU	16000000UL
#endif

#ifdef __AVR_ATmega1284P__
#define	 F_CPU	22118400UL
#endif

#include <avr/interrupt.h>
#include <util/delay.h>

#include "../source/lib_seatalk/SeaTalk.h"
#include "../source/lib_xio/xio.h"
#include "../source/lib_uart/uart.h"
#include "../source/lib_timer/Timer.h"

// Initialized data needed so that rx_count will be initialized too
uint8_t cbData = 0xFF;

const __flash uint8_t * const __flash abDatagrams[] =
{
	//	(const __flash uint8_t[]) {  },
	(const __flash uint8_t[]) { 0x84, 0x16, 0x20, 0x8F, 0x4A, 0x04, 0xE8, 0x02, 0x07 },
	(const __flash uint8_t[]) { 0x84, 0x96, 0x20, 0x8F, 0x4A, 0x00, 0xE8, 0x02, 0x07 },
	(const __flash uint8_t[]) { 0x00, 0x02, 0x20, 0x51, 0x00 },
	(const __flash uint8_t[]) { 0x00, 0x02, 0xFF, 0x53, 0x00 },
	(const __flash uint8_t[]) { 0x00, 0x02, 0x20, 0x52, 0x00 },
	(const __flash uint8_t[]) { 0x00, 0x02, 0x20, 0x50, 0x00 },
	(const __flash uint8_t[]) { 0x00, 0x02, 0x20, 0x4F, 0x00 },
	(const __flash uint8_t[]) { 0x00, 0x02, 0x20, 0x4C, 0x00 },
	(const __flash uint8_t[]) { 0x01, 0x05, 0x00, 0x00, 0x00, 0x60, 0x01, 0x00 },
	(const __flash uint8_t[]) { 0x10, 0x01, 0x01, 0xE6 },
	(const __flash uint8_t[]) { 0x11, 0x01, 0x03, 0x02 },
	(const __flash uint8_t[]) { 0x20, 0x01, 0x00, 0x00 },
	(const __flash uint8_t[]) { 0x23, 0x01, 0x09, 0x30 },
	(const __flash uint8_t[]) { 0x24, 0x02, 0x00, 0x00 },
	(const __flash uint8_t[]) { 0x25, 0x04, 0xF5, 0x68, 0xF5, 0x68, 0x00 },
	(const __flash uint8_t[]) { 0x26, 0x04, 0x00, 0x00, 0x00, 0x00, 0x49 },
	(const __flash uint8_t[]) { 0x27, 0x01, 0xBB, 0x00 },
	(const __flash uint8_t[]) { 0x36, 0x00, 0x01 },
	(const __flash uint8_t[]) { 0x50, 0x02, 0x1D, 0xCD, 0x0C },
	(const __flash uint8_t[]) { 0x51, 0x02, 0x5F, 0xF2, 0x00 },
	(const __flash uint8_t[]) { 0x54, 0x01, 0x40, 0x12 },
	(const __flash uint8_t[]) { 0x56, 0x21, 0x06, 0x0F },
	(const __flash uint8_t[]) { 0x57, 0x90, 0x01 },
	(const __flash uint8_t[]) { 0x58, 0x05, 0x1D, 0x80, 0x01, 0x5F, 0x09, 0x6F },
	(const __flash uint8_t[]) { 0x59, 0x22, 0x0A, 0x00, 0x80 },
	(const __flash uint8_t[]) { 0x61, 0x03, 0x03, 0x00, 0x00, 0x00 },
	(const __flash uint8_t[]) { 0x65, 0x00, 0x00 },
	(const __flash uint8_t[]) { 0x65, 0x00, 0x02 },
	(const __flash uint8_t[]) { 0x6C, 0x05, 0x04, 0xBA, 0x20, 0x28, 0x2D, 0x2D },
	(const __flash uint8_t[]) { 0x6E, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	(const __flash uint8_t[]) { 0x81, 0x01, 0x00, 0x00 },
	(const __flash uint8_t[]) { 0x82, 0x05, 0x00, 0xFF, 0x00, 0xFF, 0x04, 0xFB },
	(const __flash uint8_t[]) { 0x84, 0x16, 0x20, 0x8F, 0x4A, 0x04, 0xE8, 0x02, 0x07 },
	(const __flash uint8_t[]) { 0x84, 0x96, 0x20, 0x8F, 0x4A, 0x00, 0xE8, 0x02, 0x07 },
	(const __flash uint8_t[]) { 0x85, 0x06, 0x00, 0x91, 0xA1, 0x31, 0x17, 0x00, 0xE8 },
	(const __flash uint8_t[]) { 0x86, 0x41, 0x02, 0xFD },
	(const __flash uint8_t[]) { 0x87, 0x00, 0x01 },
	(const __flash uint8_t[]) { 0x90, 0x00, 0x03 },
	(const __flash uint8_t[]) { 0x93, 0x00, 0x00 },
	(const __flash uint8_t[]) { 0x99, 0x00, 0x02 },
	(const __flash uint8_t[]) { 0x99, 0x00, 0xFD },
	(const __flash uint8_t[]) { 0x99, 0x00, 0xFE },
	(const __flash uint8_t[]) { 0x9C, 0x91, 0x20, 0xE8 },
	(const __flash uint8_t[]) { 0x9E, 0x0D, 0x00, 0x00, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	(const __flash uint8_t[]) { 0xA1, 0x1D, 0x30, 0x30, 0x30, 0x30, 0x30, 0x31, 0x52, 0x65, 0x64, 0x20, 0x46, 0x69, 0x73, 0x68 },
	(const __flash uint8_t[]) { 0xA1, 0x3D, 0x30, 0x30, 0x30, 0x30, 0x30, 0x31, 0x20, 0x49, 0x73, 0x6C, 0x61, 0x6E, 0x64, 0x00 },
	(const __flash uint8_t[]) { 0xA2, 0x04, 0x01, 0x30, 0x30, 0x30, 0x31 },
	(const __flash uint8_t[]) { 0xA4, 0x02, 0x00, 0x00, 0x00 },
	(const __flash uint8_t[]) { 0xA4, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	(const __flash uint8_t[]) { 0xA5, 0x61, 0x04, 0xE2 },
	(const __flash uint8_t[]) { 0xA8, 0x53, 0x80, 0x00, 0x00, 0xD3, 0x00 },
	(const __flash uint8_t[]) { 0xAB, 0x53, 0x80, 0x00, 0x00, 0xD3, 0x00 },
	//	(const __flash uint8_t[]) {  },
	(const __flash uint8_t[]) { 0xFF }
};

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

#define FlushOutput	T2BSerialFlushOutput
#define Pending		T2BSerialPending
#define Write		T2BSerialWrite
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

#define FlushOutput	T1ASerialFlushOutput
#define Pending		T1ASerialPending
#define Write		T1ASerialWrite
#endif

int main(void)
{
	int		i;
	int		byte;
	int		inByte;
	int		command = 0;

	uint8_t	bIndex = 0;
	uint8_t bI;
	unsigned long g_ulMillis, ulDelay = 0;
	
	// Debug on pins AI0 - AI5 (Port C 328P)
	//	DBGPORT	|= _BV(DBGPIN0) | _BV(DBGPIN1) | _BV(DBGPIN2) | _BV(DBGPIN3) | _BV(DBGPIN4) | _BV(DBGPIN5);	// Set pins high
	DBGPORT = 0; // Set pins low
	DBGDDR	|= _BV(DBGPIN0) | _BV(DBGPIN1) | _BV(DBGPIN2) | _BV(DBGPIN3) | _BV(DBGPIN4) | _BV(DBGPIN5);	// Configure as output

	DBGPORT = 0xFF;
	_delay_ms(1000);
	DBGPORT	= 0;	// Set pins low

	sei();
	initTimer();
	
	uart0_init(UART_BAUD_SELECT(38400, F_CPU));
	ST1SerialBegin();

	#define SerialWriteByte uart0_putc

	xdev_out(SerialWriteByte);

	xputs_P(SerialWriteByte, PSTR("\r\n\r\nSeaTalk Test Program\r\n\r\n"));

	while (1)
	{

// 		xputs_P(SerialWriteByte, PSTR("\r\n\r\nAll Binary Values\r\n\r\n"));
// 		byte = 0x00;
// 		for (i = 0; i < 0x400; i++)
// 		{
// 			while (ST2ASerialAvailable())
// 			{
// 				inByte = ST2ASerialRead();
// 
//  				if ((inByte & 0x0F) == 0x00)
//  					xputs_P(SerialWriteByte, PSTR("\r\n"));
//  
//  				xprintf_P(PSTR("%03X "), inByte);
// 			}
// 		
// 			if (byte > 0xFF)
// 			{
// 				byte &= 0xFF;
// 				command ^= 0xFF;
// 			}
// 
// 			if (command)
// 				ST2BSerialSetCommand();
// 
// 			ST2BSerialWrite(byte++);
// 
// 			_delay_ms(5);
// 		}


		xputs_P(SerialWriteByte, PSTR("\r\n\r\nTest SeaTalk Datagrams\r\n\r\n"));
 		bIndex = 0;
 		while (0xFF != abDatagrams[bIndex][0])
 		{
 			while (ST1BSerialAvailable())
 			{
 				inByte = ST1BSerialRead();
 
 				if (inByte > 0xFF)
 					xputs_P(SerialWriteByte, PSTR("\r\n"));
 
 				xprintf_P(PSTR("%03X "), inByte);
 			}
 		
			g_ulMillis = millis();
		
			if (g_ulMillis > ulDelay)
			{
				ulDelay = g_ulMillis + 50;
				ST1ASerialSetCommand();
				for (bI = 0; bI < (abDatagrams[bIndex][1] & 0x0F)+3; bI++)
					ST1ASerialWrite(abDatagrams[bIndex][bI]);
				bIndex++;
			}
 		}

	}

/*
	while (1)
	{
		ST1ASerialCheckTxBuffer();
		inByte = ST1BSerialRead();
		if (-1 != inByte)
		{
			if (inByte > 0xFF)
			xputs_P(uart0_putc, PSTR("\r\n"));
			xprintf_P(PSTR("%03X "), inByte);
		}

	}
*/
}
