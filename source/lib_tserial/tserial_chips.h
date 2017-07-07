/*---------------------------------------------------------------------------
 *
 * tserial_boards.h
 *
 * Created: 12/25/2015 5:39:34 PM
 *  Author: dharmon
 *
 *-------------------------------------------------------------------------*/

//#ifndef TSERIAL_BOARDS_H_
//#define TSERIAL_BOARDS_H_

// Arduino Uno, Duemilanove, LilyPad, etc
//
#if defined (__AVR_ATmega168__) || defined (__AVR_ATmega328P__)

	#if tsconfigTX & tsconfigTIMER0

		#if tsconfigTX & tsconfigOCR_A
	
			#define tsTXDDRA	DDRD
			#define tsTXPORTA	PORTD
			#define tsTXPINA	PIND6
	
		#endif

		#if tsconfigTX & tsconfigOCR_B
	
			#define tsTXDDRB	DDRD
			#define tsTXPORTB	PORTD
			#define tsTXPINB	PIND5
	
		#endif
	
	#elif tsconfigTX & tsconfigTIMER1

		#if tsconfigTX & tsconfigOCR_A
	
			#define tsTXDDRA	DDRB
			#define tsTXPORTA	PORTB
			#define tsTXPINA	PINB1
			
		#endif
	
		#if tsconfigTX & tsconfigOCR_B
	
			#define tsTXDDRB	DDRB
			#define tsTXPORTB	PORTB
			#define tsTXPINB	PINB2
	
		#endif
	
	#elif tsconfigTX & tsconfigTIMER2

		#if tsconfigTX & tsconfigOCR_A
	
			#define tsTXDDRA	DDRB
			#define tsTXPORTA	PORTB
			#define tsTXPINA	PINB3
		
		#endif
		
		#if tsconfigTX & tsconfigOCR_B
	
			#define tsTXDDRB	DDRD
			#define tsTXPORTB	PORTD
			#define tsTXPINB	PIND3
			
		#endif
		
	#endif

	#if (tsconfigRX & tsconfigTIMER1) && (tsconfigRX & tsconfigICP1)
	
		#define tsRXDDRICP_1	DDRB
		#define tsRXPORTICP_1	PORTB
		#define tsRXPINICP_1	PINB0
	
	#endif

	#if tsconfigRX & tsconfigINT0

		#define tsRXDDR_0	DDRD
		#define tsRXPORT_0	PORTD
		#define tsRXPIN_0	PIND2

	#endif

	#if tsconfigRX & tsconfigINT1

		#define tsRXDDR_1	DDRD
		#define tsRXPORT_1	PORTD
		#define tsRXPIN_1	PIND3

	#endif

	#if tsconfigRX
	
		#define tsPORTCOUNT	3
	
	#endif
	
	#if tsconfigRX & tsconfigPCPORTB
	
		#define tsRXDDR		DDRB
		#define tsRXPORT	PORTB
		#define tsRXINPUT	PINB
	
	#elif tsconfigRX & tsconfigPCPORTC
	
		#define tsRXDDR		DDRC
		#define tsRXPORT	PORTC
		#define tsRXINPUT	PINC
		
	#elif tsconfigRX & tsconfigPCPORTD
	
		#define tsRXDDR		DDRD
		#define tsRXPORT	PORTD
		#define tsRXINPUT	PIND
		
	#endif

// 1284P
//
#elif defined(__AVR_ATmega1284P__)

	#if tsconfigTX & tsconfigTIMER0

		#if tsconfigTX & tsconfigOCR_A
	
			#define tsTXDDRA	DDRB
			#define tsTXPORTA	PORTB
			#define tsTXPINA	PINB3			// OC0A
	
		#endif

		#if tsconfigTX & tsconfigOCR_B
	
			#define tsTXDDRB	DDRB
			#define tsTXPORTB	PORTB
			#define tsTXPINB	PINB4			// OC0B
	
		#endif
	
	#elif tsconfigTX & tsconfigTIMER1

		#if tsconfigTX & tsconfigOCR_A
	
			#define tsTXDDRA	DDRD
			#define tsTXPORTA	PORTD
			#define tsTXPINA	PIND5			// OC1A
			
		#endif
	
		#if tsconfigTX & tsconfigOCR_B
	
			#define tsTXDDRB	DDRD
			#define tsTXPORTB	PORTD
			#define tsTXPINB	PIND6			// OC1B
	
		#endif
	
	#elif tsconfigTX & tsconfigTIMER2

		#if tsconfigTX & tsconfigOCR_A

			#define tsTXDDRA	DDRD
			#define tsTXPORTA	PORTD
			#define tsTXPINA	PIND7			// OC2A
		
		#endif
		
		#if tsconfigTX & tsconfigOCR_B
	
			#define tsTXDDRB	DDRD
			#define tsTXPORTB	PORTD
			#define tsTXPINB	PIND6			// OC2B
			
		#endif
		
	#elif tsconfigTX & tsconfigTIMER3

		#if tsconfigTX & tsconfigOCR_A
	
			#define tsTXDDRA	DDRB
			#define tsTXPORTA	PORTB
			#define tsTXPINA	PINB6			// OC3A
		
		#endif
		
		#if tsconfigTX & tsconfigOCR_B
	
			#define tsTXDDRB	DDRB
			#define tsTXPORTB	PORTB
			#define tsTXPINB	PINB7			// OC3B
			
		#endif
		
	#endif

	#if (tsconfigRX & tsconfigTIMER1) && (tsconfigRX & tsconfigICP1)
	
		#define tsRXDDRICP_1	DDRD
		#define tsRXPORTICP_1	PORTD
		#define tsRXPINICP_1	PIND6			// ICP1
	
	#endif

	#if (tsconfigRX & tsconfigTIMER3) && (tsconfigRX & tsconfigICP3)
	
		#define tsRXDDRICP_3	DDRB
		#define tsRXPORTICP_3	PORTB
		#define tsRXPINICP_3	PINB5			// ICP3
	
	#endif

	#if tsconfigRX & tsconfigINT0

		#define tsRXDDR_0	DDRD
		#define tsRXPORT_0	PORTD
		#define tsRXPIN_0	PIND2				// INT0

	#endif

	#if tsconfigRX & tsconfigINT1

		#define tsRXDDR_1	DDRD
		#define tsRXPORT_1	PORTD
		#define tsRXPIN_1	PIND3				// INT1

	#endif

	#if tsconfigRX & tsconfigINT2

		#define tsRXDDR_2	DDRB
		#define tsRXPORT_2	PORTB
		#define tsRXPIN_2	PINB2				// INT2

	#endif

	#if tsconfigRX
	
		#define tsPORTCOUNT	4
	
	#endif
	
	#if tsconfigRX & tsconfigPCPORTA
	
		#define tsRXDDR		DDRA
		#define tsRXPORT	PORTA
		#define tsRXINPUT	PINA				// PCINT0-7
	
	#elif tsconfigRX & tsconfigPCPORTB
	
		#define tsRXDDR		DDRB
		#define tsRXPORT	PORTB
		#define tsRXINPUT	PINB				// PCINT8-15
	
	#elif tsconfigRX & tsconfigPCPORTC
	
		#define tsRXDDR		DDRC
		#define tsRXPORT	PORTC
		#define tsRXINPUT	PINC				// PCINT16-23
		
	#elif tsconfigRX & tsconfigPCPORTD
	
		#define tsRXDDR		DDRD
		#define tsRXPORT	PORTD
		#define tsRXINPUT	PIND				// PCINT24-31
		
	#endif

// Unknown board
#else
#error "Please define board timer and pins"
#endif

//#endif /* TSERIAL_BOARDS_H_ */
