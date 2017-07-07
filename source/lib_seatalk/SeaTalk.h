/*---------------------------------------------------------------------------
	STimer based software serial ports

	doharmon 2015
  ---------------------------------------------------------------------------*/

#ifndef SEATALK_H_
#define SEATALK_H_

// Disable rest of file if including into an assembler file
#ifndef stserialASSEMBLER

#include <stdarg.h>
#include <inttypes.h>
#include <avr/pgmspace.h>

#if defined(__AVR_ATmega1284P__)
uint16_t	ST1SerialBegin(void);
uint16_t	ST1BSerialAvailable(void);
uint16_t	ST1BSerialPeek(void);
uint8_t		ST1BSerialOverflow(void);
uint16_t	ST1BSerialRead(void);
void		ST1ASerialWrite(uint8_t byte);
void		ST1ASerialCheckTxBuffer(void);
void		ST1ASerialSetCommand(void);
void		ST1ASerialClearCommand(void);
void		ST1ASerialFlushOutput(void);
#endif

#if defined (__AVR_ATmega168__) || defined (__AVR_ATmega328P__)
uint16_t	ST2SerialBegin(void);
uint16_t	ST2ASerialAvailable(void);
uint16_t	ST2ASerialPeek(void);
uint8_t		ST2ASerialOverflow(void);
uint16_t	ST2ASerialRead(void);
void		ST2BSerialWrite(uint8_t byte);
void		ST2BSerialCheckTxBuffer(void);
void		ST2BSerialSetCommand(void);
void		ST2BSerialClearCommand(void);
void		ST2BSerialFlushOutput(void);
#endif

#endif /* stserialASSEMBLER */

#endif	/* SEATALK_H_ */
