;---------------------------------------------------------------------------;
; Timer based software serial ports
;
; doharmon 2015
;---------------------------------------------------------------------------;
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

#ifndef F_CPU
	#if defined (__AVR_ATmega168__) || defined (__AVR_ATmega328P__)
		#define	 F_CPU	16000000
	#elif defined(__AVR_ATmega1284P__)
		#define	 F_CPU	22118400
	#else
		#error "Please define F_CPU"
	#endif
#endif

.nolist
#include <avr/io.h>					// Include device specific definitions.
#include "SeaTalk_defines.h"
#include "SeaTalk_macros.inc"
.list

#if FLASHEND > 0x1FFFF
#error lib_stserial module does not support 256K devices
#endif

; Use the define below to hard code the baud rate. This will reduce the size
; of the serial routines. Must be defined before each include of tserial.inc
#define stsBAUDRATE					4800

; TX and RX buffer sizes must be multiples of 2 and less than 257
#define TX_BUFFER_SIZE				128
#define RX_BUFFER_SIZE				128

#define TX_BUFFER_MASK				(TX_BUFFER_SIZE-1)
#define RX_BUFFER_MASK				(RX_BUFFER_SIZE-1)

; Uncomment the define below to enable frame error checking in the TX routine.
; The method used to check for frame errors is not very useful. It is probably
; best to leave it disabled.
;#define stsCHECK_FRAME_ERROR

; Example configurations

; Arduino Uno (328p)
; 
;  Timer 0
; #define stsTX					(stconfigTIMER0 | stconfigOCR_A)						;  D6  (Pin 12)
; #define stsRX					(stconfigTIMER0 | stconfigOCR_B | stconfigINT0)		;  D2  (Pin 4)

; #define stsTX					(stconfigTIMER0 | stconfigOCR_B)						;  D5  (Pin 11)
; #define stsRX					(stconfigTIMER0 | stconfigOCR_A | stconfigINT1)		;  D3  (Pin 5)

;  Timer 1
; #define stsTX					(stconfigTIMER1 | stconfigOCR_A)						;  D9  (Pin 15)
; #define stsRX					(stconfigTIMER1 | stconfigOCR_B | stconfigINT0)		;  D2  (Pin 4)

; #define stsTX					(stconfigTIMER1 | stconfigOCR_B)						;  D10 (Pin 16)
; #define stsRX					(stconfigTIMER1 | stconfigOCR_A | stconfigINT1)		;  D3  (Pin 5)

; #define stsTX					(stconfigTIMER1 | stconfigOCR_A)						;  D9  (Pin 15)
; #define stsRX					(stconfigTIMER1 | stconfigOCR_B | stconfigICP1)		;  D8  (Pin 14)

; #define stsTX					(stconfigTIMER1 | stconfigOCR_B)						;  D10 (Pin 16)
; #define stsRX					(stconfigTIMER1 | stconfigOCR_A | stconfigICP1)		;  D8  (Pin 14)

;  Timer 2
; #define stsTX					(stconfigTIMER2 | stconfigOCR_B)						;  D3  (Pin 5)
; #define stsRX					(stconfigTIMER2 | stconfigOCR_A | stconfigPCPORTB)	
; #define stsRX_PINS			(stconfigPCBIT1)										;  D9  (Pin 15)

; #define stsTX					(stconfigTIMER2 | stconfigOCR_B)						;  D3  (Pin 5)
; #define stsRX					(stconfigTIMER2 | stconfigOCR_A | stconfigINT0)		;  D2  (Pin 4)

; #define stsTX					(stconfigTIMER2 | stconfigOCR_A)						;  D11 (Pin 17)
; #define stsRX					(stconfigTIMER2 | stconfigOCR_B | stconfigINT1)		;  D3 (Pin 5)

;---------------------------------------------------------------------------;
; Define each Timer's TX and RX routines
;---------------------------------------------------------------------------;

;---------------------------------------------------------------------------;
;  Timer 0
;---------------------------------------------------------------------------;
;#define stsTX					(stconfigTIMER0 | stconfigOCR_A)
;#define stsRX					(stconfigTIMER0 | stconfigOCR_B | stconfigINT0)
;#include "SeaTalk_main.inc"

;---------------------------------------------------------------------------;
;  Timer 1
;---------------------------------------------------------------------------;
#if defined(__AVR_ATmega1284P__)
#define stsTX					(stconfigTIMER1 | stconfigOCR_A)				// SeaTalk TX:19
#define stsRX					(stconfigTIMER1 | stconfigOCR_B | stconfigICP1)	// SeaTalk RX:20
#include "SeaTalk_main.inc"
#endif

;---------------------------------------------------------------------------;
;  Timer 2
;---------------------------------------------------------------------------;
#if defined (__AVR_ATmega168__) || defined (__AVR_ATmega328P__)
#define stsTX					(stconfigTIMER2 | stconfigOCR_B)				// SeaTalk TX:D3
#define stsRX					(stconfigTIMER2 | stconfigOCR_A | stconfigINT0)	// SeaTalk RX:D2
#include "SeaTalk_main.inc"
#endif
