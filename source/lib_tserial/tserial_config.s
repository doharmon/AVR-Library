;---------------------------------------------------------------------------;
; Timer based software serial ports
;
; doharmon 2015
;---------------------------------------------------------------------------;
;
; Terminology
;	- Port	Any time this word is used by itself it refers to either a 
;			transmit or receive serial port
;	- TX	Short for transmit
;	- RX	Short for receive
;
; Features
;	- Can use 8 or 16 bit timers
;	- Two modes: Binary (values 0 - 255) and ASCII (0 - 127)
;	- TX and RX ports are independent of each other, don't need to be paired
;	- All ports always active
;	- Baud rate can configured either during runtime or hardcoded
;
; Limitations
;	- Baud rate configured at timer level, all ports using the same timer
;	  will have the same baud rate
;	- Each TX port needs to be assigned to its own OCR
;	- Each binary mode RX port needs to be assigned to its own OCR
;	- Each OCR can be assigned to only one port
;
;---------------------------------------------------------------------------;
;
; Configuration
;
; A hardcoded baud rate is defined by using tsBAUDRATE
; TX ports are defined by assigning paramters to tsTX
; RX ports are defined by assigning paramters to tsRX and tsRX_PINS
;
; NOTE: tsRX_PINS is only used if tsRX is assigned a PCPORTA-D parameter
;
; PARAMETER				TX	RX 	RX_PINS	DESCRIPTION
; tsconfigTIMER0-5		x	x			Which timer to use
; tsconfigOCR_A-C		x	x			Which OCR to use
; tsconfigASCII			x	x			Port is ASCII mode, default is binary
; tsconfigICP0-5			x			Use input capture pin for RX
; tsconfigINT0-7			x			Use interrupt pin for RX
; tsconfigPCPORTA-D			x			I/O port for pin capture for RX
; tsconfigPCBIT0-7				x		Pin to use for PCINT for RX
;
; Notes
;	- Hardcoding the baud rate can improve performance
;	- The maximum number of TX ports is limited to the number of OCRs
;	- Can currently only create one binary RX port per timer
;	- The maximum number of ASCII mode RX ports is limited to the number
;	  of pins on the I/O port being used
;	- Neither ICP0-5 nor INT0-7 can be used for RX if an I/O port is being
;	- used for RX
;
; Examples
;
; - Hardcoded baud rate of 9600 baud
;	#define tsBAUDRATE	9600
;
; - A TX port that uses timer 0 and OCR A
;	#define tsTX (tsconfigTIMER0 | tsconfigOCR_A)
;
; - An RX port that uses timer 0 and OCR A with input from pin INT2
;	#define tsRX (tsconfigTIMER0 | tsconfigOCR_A | tsconfigINT2)
;
; - TX and RX ports that use timer 1
;	#define tsTX (tsconfigTIMER1 | tsconfigOCR_A)
;	#define tsRX (tsconfigTIMER1 | tsconfigOCR_B | tsconfigICP1)
;
; - ASCII mode TX and RX ports that use timer 2
;	#define tsTX 	  (tsconfigTIMER2 | tsconfigOCR_A   | tsconfigASCII)
;	#define tsRX 	  (tsconfigTIMER2 | tsconfigPCPORTB | tsconfigASCII)
;	#define tsRX_PINS (tsconfigPCBIT0 | tsconfigPCBIT1)
;
;---------------------------------------------------------------------------;
;
; Init Functions - Created when either a TX or RX serial port is configured
;
; uint16_t  TXSerialBegin(uint32_t baud | void)
; void 		TXSerialEnd(void)
;
; TX is T0, T1, T2, or T3 depending on which timer is used.
; 
; Examples:
;
; #define tsTX (tsconfigTIMER0 | tsconfigOCR_A)
;
; uint16_t	T0SerialBegin(void);
; void 		T0SerialEnd(void)
;
; #define tsRX	(tsconfigTIMER2 | tsconfigOCR_B | tsconfigINT2)
;
; uint16_t	T2SerialBegin(void);
; void 		T2SerialEnd(void)
;
;---------------------------------------------------------------------------;
;
; Transmit Functions
;
; void		XSerialWrite(uint8_t b)
; void 		XSerialFlushOutput(void)
; uint16_t 	XSerialPending(void)
;
; X is in the form T[Timer][OCR][Ascii]
;	Where:
;		Timer is 0, 1, 2, or 3
;		OCR is A, B, or C
;		Ascii appears only if port is ASCII mode
;
; Examples:
;
; #define tsTX (tsconfigTIMER0 | tsconfigOCR_A | tsconfigOCR_B | tsconfigASCII)
;
; uint16_t	T0SerialBegin(void);
; void 		T0SerialEnd(void)
; void		T0AAsciiSerialWrite(uint8_t byte);
; void		T0BAsciiSerialWrite(uint8_t byte);
; void		T0AAsciiSerialFlushOutput(void);
; void		T0BAsciiSerialFlushOutput(void);
; uint16_t	T0AAsciiSerialPending(void);
; uint16_t	T0BAsciiSerialPending(void);
;
;
; #define tsTX (tsconfigTIMER2 | tsconfigOCR_A)
;
; uint16_t	T2SerialBegin(void);
; void 		T2SerialEnd(void)
; void		T2ASerialWrite(uint8_t byte);
; void		T2ASerialFlushOutput(void);
; uint16_t	T2ASerialPending(void);
;
;---------------------------------------------------------------------------;
;
; Receive Functions
;
; uint16_t 	XSerialRead(void)
; uint16_t 	XSerialPeek(void)
; uint16_t 	XSerialAvailable(void)
; void 		XSerialFlushInput(void)
; uint8_t 	XSerialOverflow(void)
;
; X is in the form T[Timer][OCR][PXY][Ascii]
;	Where:
;		Timer is 0, 1, 2, or 3
;		OCR is A, B, or C. (Appears only if port is binary and not ASCII)
;		PXY where:
;			X is A, B, C, or D depending on which I/O port is used
;			Y is 0-7 depending on which I/O port pin is used
;		Ascii appears only if port is ASCII mode
;
; Examples:
;
; #define tsRX		(tsconfigTIMER0 | tsconfigPCPORTB | tsconfigASCII)
; #define tsRX_PINS	(tsconfigPCBIT0 | tsconfigPCBIT1)
;
; uint16_t	T0PB0AsciiSerialRead(void);
; uint16_t	T0PB0AsciiSerialPeek(void);
; uint16_t	T0PB0AsciiSerialAvailable(void);
; void		T0PB0AsciiSerialFlushInput(void);
; uint8_t	T0PB0AsciiSerialOverflow(void);
; uint16_t	T0PB1AsciiSerialRead(void);
; uint16_t	T0PB1AsciiSerialPeek(void);
; uint16_t	T0PB1AsciiSerialAvailable(void);
; void		T0PB1AsciiSerialFlushInput(void);
; uint8_t	T0PB1AsciiSerialOverflow(void);
;
;
; #define tsRX	(tsconfigTIMER2 | tsconfigOCR_B | tsconfigINT2)
;
; uint16_t	T2BSerialRead(void);
; uint16_t	T2BSerialPeek(void);
; uint16_t	T2BSerialAvailable(void);
; void		T2BSerialFlushInput(void);
; uint8_t	T2BSerialOverflow(void);
;
;---------------------------------------------------------------------------;
;
;  On the Arduino board, digital pins are also used
;  for the analog output (software PWM).  Analog input
;  pins are a separate set.
;  ATMEL ATMEGA168 & 328 / ARDUINO
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
;---------------------------------------------------------------------------;

#ifndef F_CPU
	#if defined (__AVR_ATmega168__) || defined (__AVR_ATmega328P__)
		#define	 F_CPU	16000000
	#elif defined(__AVR_ATmega1284P__)
		#define	 F_CPU	22118400
	#endif
#endif

.nolist
#include <avr/io.h>					// Include device specific definitions.
#include "tserial_defines.h"
#include "tserial_macros.inc"
.list

#if FLASHEND > 0x1FFFF
#error lib_tserial module does not support 256K devices
#endif

;---------------------------------------------------------------------------;

; Example configurations

; Arduino Uno (328p)
; 
;  Timer 0
; #define tsTX					(tsconfigTIMER0 | tsconfigOCR_A)						;  D6  (Pin 12)
; #define tsRX					(tsconfigTIMER0 | tsconfigOCR_B | tsconfigINT0)			;  D2  (Pin 4)

; #define tsTX					(tsconfigTIMER0 | tsconfigOCR_B)						;  D5  (Pin 11)
; #define tsRX					(tsconfigTIMER0 | tsconfigOCR_A | tsconfigINT1)			;  D3  (Pin 5)

;  Timer 1
; #define tsTX					(tsconfigTIMER1 | tsconfigOCR_A)						;  D9  (Pin 15)
; #define tsRX					(tsconfigTIMER1 | tsconfigOCR_B | tsconfigINT0)			;  D2  (Pin 4)

; #define tsTX					(tsconfigTIMER1 | tsconfigOCR_B)						;  D10 (Pin 16)
; #define tsRX					(tsconfigTIMER1 | tsconfigOCR_A | tsconfigINT1)			;  D3  (Pin 5)

; #define tsTX					(tsconfigTIMER1 | tsconfigOCR_A)						;  D9  (Pin 15)
; #define tsRX					(tsconfigTIMER1 | tsconfigOCR_B | tsconfigICP1)			;  D8  (Pin 14)

; #define tsTX					(tsconfigTIMER1 | tsconfigOCR_B)						;  D10 (Pin 16)
; #define tsRX					(tsconfigTIMER1 | tsconfigOCR_A | tsconfigICP1)			;  D8  (Pin 14)

;  Timer 2
; #define tsTX					(tsconfigTIMER2 | tsconfigOCR_B)						;  D3  (Pin 5)
; #define tsRX					(tsconfigTIMER2 | tsconfigOCR_A | tsconfigPCPORTB)	
; #define tsRX_PINS				(tsconfigPCBIT1)										;  D9  (Pin 15)

; #define tsTX					(tsconfigTIMER2 | tsconfigOCR_B)						;  D3  (Pin 5)
; #define tsRX					(tsconfigTIMER2 | tsconfigOCR_A | tsconfigINT0)			;  D2  (Pin 4)

; #define tsTX					(tsconfigTIMER2 | tsconfigOCR_A)						;  D11 (Pin 17)
; #define tsRX					(tsconfigTIMER2 | tsconfigOCR_B | tsconfigINT1)			;  D3  (Pin 5)

;---------------------------------------------------------------------------;
; Define each Timer's TX and RX routines
;---------------------------------------------------------------------------;

;---------------------------------------------------------------------------;
;  Timer 0
;---------------------------------------------------------------------------;
; Use the define below to hardcode the baud rate. This will reduce the size
; of the serial routines.
#define tsBAUDRATE					9600

; TX and RX buffer sizes must be multiples of 2 and less than 257
#define TX_BUFFER_SIZE				128
#define RX_BUFFER_SIZE				128

#define TX_BUFFER_MASK				(TX_BUFFER_SIZE-1)
#define RX_BUFFER_MASK				(RX_BUFFER_SIZE-1)

#if defined (__AVR_ATmega168__) || defined (__AVR_ATmega328P__)
;#define tsTX					(tsconfigTIMER0 | tsconfigOCR_B)									// TX:5
;#define tsTX					(tsconfigTIMER0 | tsconfigOCR_A	  | tsconfigOCR_B | tsconfigASCII)	// VHF TX:4; GPS TX:5
;#define tsRX					(tsconfigTIMER0 | tsconfigPCPORTB | tsconfigASCII)
;#define tsRX_PINS				(tsconfigPCBIT0 | tsconfigPCBIT1)									// VHF RX:1; GPS RX:2
#endif

#if defined(__AVR_ATmega1284P__)
#define tsTX					(tsconfigTIMER0 | tsconfigOCR_A	  | tsconfigOCR_B | tsconfigASCII)	// VHF TX:4; GPS TX:5
#define tsRX					(tsconfigTIMER0 | tsconfigPCPORTB | tsconfigASCII)
#define tsRX_PINS				(tsconfigPCBIT0 | tsconfigPCBIT1)									// VHF RX:1; GPS RX:2
#endif

#include "tserial_main.inc"
#undef tsBAUDRATE

;---------------------------------------------------------------------------;
;  Timer 1
;---------------------------------------------------------------------------;
; Use the define below to hardcode the baud rate. This will reduce the size
; of the serial routines.
;#define tsBAUDRATE					38400

; TX and RX buffer sizes must be multiples of 2 and less than 257
;#define TX_BUFFER_SIZE				32
;#define RX_BUFFER_SIZE				32

;#define TX_BUFFER_MASK				(TX_BUFFER_SIZE-1)
;#define RX_BUFFER_MASK				(RX_BUFFER_SIZE-1)

#if defined (__AVR_ATmega168__) || defined (__AVR_ATmega328P__)
;#define tsTX					(tsconfigTIMER1 | tsconfigOCR_A)									// TX:9
;#define tsTX					(tsconfigTIMER1 | tsconfigOCR_A | tsconfigASCII)
;#define tsRX					(tsconfigTIMER1 | tsconfigOCR_B | tsconfigINT1)
;#define tsRX					(tsconfigTIMER1 | tsconfigOCR_B | tsconfigICP1 | tsconfigASCII)
#endif

#if defined(__AVR_ATmega1284P__)
;#define tsTX					(tsconfigTIMER1 | tsconfigOCR_A)									// TX:19
;#define tsRX					(tsconfigTIMER1 | tsconfigOCR_B | tsconfigICP1)						// RX:20
#endif

;#include "tserial_main.inc"
#undef tsBAUDRATE

;---------------------------------------------------------------------------;
;  Timer 2
;---------------------------------------------------------------------------;
; Use the define below to hardcode the baud rate. This will reduce the size
; of the serial routines.
#define tsBAUDRATE					57600

; TX and RX buffer sizes must be multiples of 2 and less than 257
#define TX_BUFFER_SIZE				32
#define RX_BUFFER_SIZE				32

#define TX_BUFFER_MASK				(TX_BUFFER_SIZE-1)
#define RX_BUFFER_MASK				(RX_BUFFER_SIZE-1)

#if defined (__AVR_ATmega168__) || defined (__AVR_ATmega328P__)
#define tsTX					(tsconfigTIMER2 | tsconfigOCR_B)								// CMD TX:3
#define tsRX					(tsconfigTIMER2 | tsconfigOCR_A | tsconfigINT0)					// CMD RX:2
#endif

#if defined(__AVR_ATmega1284P__)
#define tsTX					(tsconfigTIMER2 | tsconfigOCR_A)								// CMD TX:21
#define tsRX					(tsconfigTIMER2 | tsconfigOCR_B | tsconfigINT2)					// CMD RX:3
#endif

#include "tserial_main.inc"
#undef tsBAUDRATE

;---------------------------------------------------------------------------;
;  Timer 3
;---------------------------------------------------------------------------;
; Use the define below to hardcode the baud rate. This will reduce the size
; of the serial routines.
#if defined(__AVR_ATmega1284P__)
;#define tsBAUDRATE					38400

; TX and RX buffer sizes must be multiples of 2 and less than 257
;#define TX_BUFFER_SIZE				32
;#define RX_BUFFER_SIZE				32

;#define TX_BUFFER_MASK				(TX_BUFFER_SIZE-1)
;#define RX_BUFFER_MASK				(RX_BUFFER_SIZE-1)

;#define tsTX					(tsconfigTIMER3 | tsconfigOCR_A)								// CMD TX:7
;#define tsRX					(tsconfigTIMER3 | tsconfigOCR_B | tsconfigINT0)					// CMD RX:16

;#include "tserial_main.inc"
#undef tsBAUDRATE
#endif
