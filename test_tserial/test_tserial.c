/*
 * test_tserial.c
 *
 * Created: 12/21/2015 9:46:23 PM
 *  Author: dharmon
 */ 

/*
;
;                                     +-\/-+
;               (PCINT14/RESET) PC6  1|    |28 PC5 (ADC5/SCL/PCINT13) (AI5)
;      (D0)       (PCINT16/RXD) PD0  2|    |27 PC4 (ADC4/SDA/PCINT12) (AI4)
;      (D1)       (PCINT17/TXD) PD1  3|    |26 PC3 (ADC3/PCINT11)     (AI3)
;      (D2)      (PCINT18/INT0) PD2  4|    |25 PC2 (ADC2/PCINT10)     (AI2)
;  PWM (D3) (PCINT19/OC2B/INT1) PD3  5|    |24 PC1 (ADC1/PCINT9)      (AI1)
;      (D4)    (PCINT20/XCK/T0) PD4  6|    |23 PC0 (ADC0/PCINT8)      (AI0)
;                               VCC  7|    |22 GND
;                               GND  8|    |21 AREF
;          (PCINT6/XTAL1/TOSC1) PB6  9|    |20 AVCC
;          (PCINT7/XTAL2/TOSC2) PB7 10|    |19 PB5 (SCK/PCINT5)       (D13)
;  PWM (D5)   (PCINT21/OC0B/T1) PD5 11|    |18 PB4 (MISO/PCINT4)      (D12)
;  PWM (D6) (PCINT22/OC0A/AIN0) PD6 12|    |17 PB3 (MOSI/OC2A/PCINT3) (D11) PWM
;      (D7)      (PCINT23/AIN1) PD7 13|    |16 PB2 (SS/OC1B/PCINT2)   (D10) PWM
;      (D8)  (PCINT0/CLKO/ICP1) PB0 14|    |15 PB1 (OC1A/PCINT1)      (D9)  PWM
;                                     +----+
;
;  ATMEL ATMEGA1284
;
;                                     +-\/-+
;              (PCINT8/XCK0/T0) PB0  1|    |40 PA0 (ADC0/PCINT0)
;              (PCINT9/CLKO/T1) PB1  2|    |39 PA1 (ADC1/PCINT1)
;           (PCINT10/INT2/AIN0) PB2  3|    |38 PA2 (ADC2/PCINT2)
;           (PCINT11/OC0A/AIN1) PB3  4|    |37 PA3 (ADC3/PCINT3)
;             (PCINT12/OC0B/SS) PB4  5|    |36 PA4 (ADC4/PCINT4)
;           (PCINT13/ICP3/MOSI) PB5  6|    |35 PA5 (ADC5/PCINT5)
;           (PCINT14/OC3A/MISO) PB6  7|    |34 PA6 (ADC6/PCINT6)
;            (PCINT15/OC3B/SCK) PB7  8|    |33 PA7 (ADC7/PCINT7)
;                             RESET  9|    |32 AREF
;                               VCC 10|    |31 GND
;                               GND 11|    |30 AVCC
;                             XTAL1 12|    |29 PC7 (TOSC2/PCINT23)
;                             XTAL2 13|    |28 PC6 (TOSC1/PCINT22)
;             (PCINT24/RXD0/T3) PD0 14|    |27 PC5 (TDI/PCINT21)
;                (PCINT25/TXD0) PD1 15|    |26 PC4 (TDO/PCINT20)
;           (PCINT26/RXD1/INT0) PD2 16|    |25 PC3 (TMS/PCINT19)
;           (PCINT27/TXD1/INT1) PD3 17|    |24 PC2 (TCK/PCINT18)
;           (PCINT28/XCK1/OC1B) PD4 18|    |23 PC1 (SDA/PCINT17)
;                (PCINT29/OC1A) PD5 19|    |22 PC0 (SCL/PCINT16)
;            (PCINT30/OC2B/ICP) PD6 20|    |21 PD7 (OC2A/PCINT31)
;                                     +----+
;
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

#include "../source/lib_tserial/tserial.h"
#include "../source/lib_xio/xio.h"

//									 1		   2		 3		   4		 5		   6		 7		   8		8 8	
//							123456789012345678901234567890123456789012345678901234567890123456789012345678901234567 8 9
const __flash char sz1[] = "This is a very long test message in order to test receive and transmit buffer overflows\r\n";
const __flash char sz2[] = "This is another long test message. This one tests using the pending function for xmit. \r\n";

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
//#define Write		T2BAsciiSerialWrite
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

#define FlushOutput	T2ASerialFlushOutput
#define Pending		T2ASerialPending
#define Write		T2ASerialWrite
#endif

int main(void)
{
	uint8_t		i, j;
	uint16_t	a;
	
	// Debug on pins AI0 - AI5 (Port C)
//	DBGPORT	|= _BV(DBGPIN0) | _BV(DBGPIN1) | _BV(DBGPIN2) | _BV(DBGPIN3) | _BV(DBGPIN4) | _BV(DBGPIN5);	// Set pins high
	DBGPORT = 0; // Set pins low
	DBGDDR	|= _BV(DBGPIN0) | _BV(DBGPIN1) | _BV(DBGPIN2) | _BV(DBGPIN3) | _BV(DBGPIN4) | _BV(DBGPIN5);	// Configure as output

	DBGPORT = 0xFF;
//	_delay_ms(1000);
	DBGPORT	= 0;
	
	sei();
	
//	T0SerialBegin();
//	T1SerialBegin();
	T2SerialBegin();
//	T3SerialBegin();

// 	for (a = 0; a <= 0x1FF; a++)
// 		T0AAsciiSerialWrite(a);
// 
// 	for (a = 0; a <= 0x1FF; a++)
// 		T0BAsciiSerialWrite(a);
// 
// 	for (a = 0; a <= 0x1FF; a++)
// 		T1ASerialWrite(a);

	for (a = 0; a <= 0x1FF; a++)
		Write(a);

// 	for (a = 0; a <= 0x1FF; a++)
// 		T3ASerialWrite(a);

	DBGPORT = 0xFF;
	_delay_ms(1000);
	DBGPORT	= 0;

	i = 0x00;
	while (1)
	{
// 		T0AAsciiSerialWrite(i);
// 		T0BAsciiSerialWrite(i);
// 		T1ASerialWrite(i);
		Write(i);
//		T3ASerialWrite(i);

//		_delay_ms(5);
//		while (T2ASerialAvailable())
// 		{
// 			xfprintf_P(T0BSerialWrite,PSTR("TX:%02hX RX:%02hX\r\n"), i, T2ASerialRead());
// 		}

		i++;
	}

/*
	while (1)	
	{
		for (i = 0; i < 10; i++)
		{
//			DBGPIN |= _BV(DBGPIN0);
			FlushOutput();
//			DBGPIN |= _BV(DBGPIN0);
		
//			DBGPIN |= _BV(DBGPIN1);
			for (j = 0; j < sizeof(sz1)-1; j++)
			{
//				DBGPIN |= _BV(DBGPIN2);
				Write(sz1[j]);
			}
//			DBGPIN |= _BV(DBGPIN1);
		}

		for (i = 0; i < 10; i++)
		{
//			DBGPIN |= _BV(DBGPIN3);
			while(Pending())
				;
//			DBGPIN |= _BV(DBGPIN3);
		
//			DBGPIN |= _BV(DBGPIN4);
			for (j = 0; j < sizeof(sz2)-1; j++)
			{
//				DBGPIN |= _BV(DBGPIN5);
				Write(sz2[j]);
			}
//			DBGPIN |= _BV(DBGPIN4);
		}
	}
*/
}
