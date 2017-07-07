/*---------------------------------------------------------------------------
	Timer based software serial ports

	doharmon 2015
  ---------------------------------------------------------------------------*/

#ifndef TSERIAL_H_
#define TSERIAL_H_

// Disable rest of file if including into an assembler file
#ifndef tserialASSEMBLER

#include <stdarg.h>
#include <inttypes.h>
#include <avr/pgmspace.h>

void		T0BSerialWrite(uint8_t byte);
void		T0BSerialFlushOutput(void);
uint16_t	T0BSerialPending(void);
void		T0BAsciiSerialWrite(uint8_t byte);
void		T0BAsciiSerialFlushOutput(void);
uint16_t	T0BAsciiSerialPending(void);

uint16_t	T0SerialBegin(void);
uint16_t	T0PB0AsciiSerialAvailable(void);
uint8_t		T0PB0AsciiSerialOverflow(void);
uint16_t	T0PB0AsciiSerialRead(void);
uint16_t	T0PB1AsciiSerialAvailable(void);
uint8_t		T0PB1AsciiSerialOverflow(void);
uint16_t	T0PB1AsciiSerialRead(void);
void		T0AAsciiSerialWrite(uint8_t byte);
void		T0BAsciiSerialWrite(uint8_t byte);
void		T0AAsciiSerialFlushOutput(void);
void		T0BAsciiSerialFlushOutput(void);
uint16_t	T0AAsciiSerialPending(void);
uint16_t	T0BAsciiSerialPending(void);

uint16_t	T1SerialBegin(void);
void		T1ASerialWrite(uint8_t byte);
void		T1ASerialFlushOutput(void);
uint16_t	T1ASerialPending(void);

uint16_t	T2SerialBegin(void);
uint16_t	T2BSerialAvailable(void);
uint8_t		T2BSerialOverflow(void);
uint16_t	T2BSerialRead(void);
void		T2ASerialWrite(uint8_t byte);
void		T2ASerialFlushOutput(void);
uint16_t	T2ASerialPending(void);

uint16_t	T2ASerialAvailable(void);
uint8_t		T2ASerialOverflow(void);
uint16_t	T2ASerialRead(void);
uint16_t	T2AAsciiSerialAvailable(void);
uint8_t		T2AAsciiSerialOverflow(void);
uint16_t	T2AAsciiSerialRead(void);
void		T2BSerialWrite(uint8_t byte);
void		T2BAsciiSerialWrite(uint8_t byte);
void		T2BSerialFlushOutput(void);
uint16_t	T2BSerialPending(void);

uint16_t	T3SerialBegin(void);
void		T3ASerialWrite(uint8_t byte);
void		T3ASerialFlushOutput(void);
uint16_t	T3ASerialPending(void);

#endif /* tserialASSEMBLER */

#endif	/* TSERIAL_H_ */
