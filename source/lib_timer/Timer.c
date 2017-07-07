/*! @file
 *
 * Timer.c
 *
 * Created: 11/15/2014 11:11:05 AM
 *  Author: doharmon
 *
 * Based on the Arduino wiring.c file
 *
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#include "Timer.h"

//*****************************************************************************
//
//! \addtogroup Timer_API
//! @{
//
//*****************************************************************************

#ifdef __AVR_ATmega1284P__
#define PRESCALER			(1)
#define MAX_COUNTER			(65535)
#define TICKS_TO_OVERFLOW	(65536)
#define TIMER_COUNTER_WIDTH	(16)
#define TIMER_INT_FLAG_REG	(TIFR3)
#define TIMER_OVERFLOW_FLAG	(TOV3)
#define TIMER_INT_MASK_REG	(TIMSK3)
#define TIMER_OVF_INT_EN	(TOIE3)
#define GET_TIMER_COUNT()	(TCNT3)
#define TIMER_OVERFLOW_INT	TIMER3_OVF_vect

#define CONFIG_TIMER_NOPRESCALE()		(TIMSK3 = 0, TCCR3A = 0, TCCR3B = (1<<CS30))
#define CONFIG_TIMER_PRESCALE_8()		(TIMSK3 = 0, TCCR3A = 0, TCCR3B = (1<<CS31))
#define CONFIG_TIMER_PRESCALE_64()		(TIMSK3 = 0, TCCR3A = 0, TCCR3B = (1<<CS31) | (1<<CS30))
#define CONFIG_TIMER_PRESCALE_256()		(TIMSK3 = 0, TCCR3A = 0, TCCR3B = (1<<CS32))
#define CONFIG_TIMER_PRESCALE_1024()	(TIMSK3 = 0, TCCR3A = 0, TCCR3B = (1<<CS32) | (1<<CS30))

#define SET_PRESCALE()					CONFIG_TIMER_NOPRESCALE()

#elif defined(__AVR_ATmega328P__)
#define PRESCALER			(1)
#define MAX_COUNTER			(65535)
#define TICKS_TO_OVERFLOW	(65536)
#define TIMER_COUNTER_WIDTH	(16)
#define TIMER_INT_FLAG_REG	(TIFR1)
#define TIMER_OVERFLOW_FLAG	(TOV1)
#define TIMER_INT_MASK_REG	(TIMSK1)
#define TIMER_OVF_INT_EN	(TOIE1)
#define GET_TIMER_COUNT()	(TCNT1)
#define TIMER_OVERFLOW_INT	TIMER1_OVF_vect

#define CONFIG_TIMER_NOPRESCALE()		(TIMSK1 = 0, TCCR1A = 0, TCCR1B = (1<<CS10))
#define CONFIG_TIMER_PRESCALE_8()		(TIMSK1 = 0, TCCR1A = 0, TCCR1B = (1<<CS11))
#define CONFIG_TIMER_PRESCALE_64()		(TIMSK1 = 0, TCCR1A = 0, TCCR1B = (1<<CS11) | (1<<CS10))
#define CONFIG_TIMER_PRESCALE_256()		(TIMSK1 = 0, TCCR1A = 0, TCCR1B = (1<<CS12))
#define CONFIG_TIMER_PRESCALE_1024()	(TIMSK1 = 0, TCCR1A = 0, TCCR1B = (1<<CS12) | (1<<CS10))

#define SET_PRESCALE()					CONFIG_TIMER_NOPRESCALE()

#else
#error "Unsupported device"
#endif // __AVR_ATmega1284P__

#define clockCyclesPerMicrosecond()		( F_CPU / 1000000UL )
#define clockCyclesToMicroseconds(a)	( ((a) * 1000000ULL + F_CPU / 2) / F_CPU )	// ( (a) / clockCyclesPerMicrosecond() )

//! The prescaler is set so that the timer ticks every PRESCALER clock cycles, and
//! the overflow handler is called every TICKS_TO_OVERFLOW ticks.
#define MICROSECONDS_PER_TIMER_OVERFLOW (clockCyclesToMicroseconds(PRESCALER * TICKS_TO_OVERFLOW))

//! The whole number of milliseconds per timer overflow
#define MILLIS_INC (MICROSECONDS_PER_TIMER_OVERFLOW / 1000)

//! The fractional number of milliseconds per timer overflow.
#define FRACT_INC (MICROSECONDS_PER_TIMER_OVERFLOW % 1000)
#define FRACT_MAX (1000)

volatile uint32_t timer_overflow_count = 0;
volatile uint32_t timer_millis         = 0;
static   uint16_t timer_fract          = 0;

ISR(TIMER_OVERFLOW_INT)
{
	// Copy these to local variables so they can be stored in registers
	// (volatile variables must be read from memory on every access)
	uint32_t m = timer_millis;
	uint16_t f = timer_fract;

	f += FRACT_INC;

	if (f >= FRACT_MAX)
	{
		f -= FRACT_MAX;
		m += (MILLIS_INC + 1);
	}
	else
	{
		m += MILLIS_INC;
	}

	timer_fract  = f;
	timer_millis = m;
	timer_overflow_count++;
}

unsigned long millis(void)
{
	uint32_t m;
	uint8_t  oldSREG = SREG;

	// Disable interrupts while we read timer_millis or we might get an
	// inconsistent value (e.g. in the middle of a write to timer_millis)
	cli();
	m    = timer_millis;
	SREG = oldSREG;

	return m;
}

unsigned long micros(void) 
{
	uint32_t m;
	uint16_t t;
	uint8_t  oldSREG = SREG;
	
	cli();

	m = timer_overflow_count;
	t = GET_TIMER_COUNT();

	
	if ((TIMER_INT_FLAG_REG & _BV(TIMER_OVERFLOW_FLAG)) && (t < MAX_COUNTER))
		m++;

	SREG = oldSREG;

	// If PRESCALER is smaller then the division results in zero.	
#if PRESCALER > clockCyclesPerMicrosecond()
	return ((m << TIMER_COUNTER_WIDTH) + t) * (PRESCALER / clockCyclesPerMicrosecond());
#else // PRESCALER > clockCyclesPerMicrosecond()
	return ((m << TIMER_COUNTER_WIDTH) + t) / clockCyclesPerMicrosecond() * PRESCALER;
#endif // PRESCALER > clockCyclesPerMicrosecond()
}

void delay(unsigned long ms)
{
	uint16_t start = (uint16_t)micros();

	while (ms > 0) 
	{
		if (((uint16_t)micros() - start) >= 1000) 
		{
			ms--;
			start += 1000;
		}
	}
}

void initTimer(void)
{
#warning "initTimer clears all interrupts for timer"

	// Set timer prescale factor
	SET_PRESCALE();

	// Enable timer overflow interrupt
	TIMER_INT_MASK_REG	|= _BV(TIMER_OVF_INT_EN);
	
	// Set counter to zero
	GET_TIMER_COUNT() = 0;
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
