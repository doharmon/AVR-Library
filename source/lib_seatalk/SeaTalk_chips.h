/*---------------------------------------------------------------------------
 *
 * SeaTalk_boards.h
 *
 * Created: 12/25/2015 5:39:34 PM
 *  Author: dharmon
 *
 *-------------------------------------------------------------------------*/

//#ifndef SEATALK_BOARDS_H_
//#define SEATALK_BOARDS_H_

// Arduino Uno, Duemilanove, LilyPad, etc
//
#if defined (__AVR_ATmega168__) || defined (__AVR_ATmega328P__)

	#if stconfigTX & stconfigTIMER0

		#if stconfigTX & stconfigOCR_A
	
			#define stsTXDDRA	DDRD
			#define stsTXPORTA	PORTD
			#define stsTXPINA	PIND6
	
		#endif

		#if stconfigTX & stconfigOCR_B
	
			#define stsTXDDRB	DDRD
			#define stsTXPORTB	PORTD
			#define stsTXPINB	PIND5
	
		#endif
	
	#elif stconfigTX & stconfigTIMER1

		#if stconfigTX & stconfigOCR_A
	
			#define stsTXDDRA	DDRB
			#define stsTXPORTA	PORTB
			#define stsTXPINA	PINB1
			
		#endif
	
		#if stconfigTX & stconfigOCR_B
	
			#define stsTXDDRB	DDRB
			#define stsTXPORTB	PORTB
			#define stsTXPINB	PINB2
	
		#endif
	
	#elif stconfigTX & stconfigTIMER2

		#if stconfigTX & stconfigOCR_A
	
			#define stsTXDDRA	DDRB
			#define stsTXPORTA	PORTB
			#define stsTXPINA	PINB3
		
		#endif
		
		#if stconfigTX & stconfigOCR_B
	
			#define stsTXDDRB	DDRD
			#define stsTXPORTB	PORTD
			#define stsTXPINB	PIND3
			
		#endif
		
	#endif

	#if (stconfigRX & stconfigTIMER1) && (stconfigRX & stconfigICP1)
	
		#define stsRXDDRICP_1	DDRB
		#define stsRXPORTICP_1	PORTB
		#define stsRXPINICP_1	PINB0
	
	#endif

	#if stconfigRX & stconfigINT0

		#define stsRXDDR_0	DDRD
		#define stsRXPORT_0	PORTD
		#define stsRXPIN_0	PIND2

	#endif

	#if stconfigRX & stconfigINT1

		#define stsRXDDR_1	DDRD
		#define stsRXPORT_1	PORTD
		#define stsRXPIN_1	PIND3

	#endif

	#if stconfigRX
	
		#define stsPORTCOUNT	3
	
	#endif
	
	#if stconfigRX & stconfigPCPORTB
	
		#define stsRXDDR	DDRB
		#define stsRXPORT	PORTB
		#define stsRXINPUT	PINB
	
	#elif stconfigRX & stconfigPCPORTC
	
		#define stsRXDDR	DDRC
		#define stsRXPORT	PORTC
		#define stsRXINPUT	PINC
		
	#elif stconfigRX & stconfigPCPORTD
	
		#define stsRXDDR	DDRD
		#define stsRXPORT	PORTD
		#define stsRXINPUT	PIND
		
	#endif

// 1284P
//
#elif defined(__AVR_ATmega1284P__)

	#if stconfigTX & stconfigTIMER0

		#if stconfigTX & stconfigOCR_A
	
			#define stsTXDDRA	DDRB
			#define stsTXPORTA	PORTB
			#define stsTXPINA	PINB3			// OC0A
	
		#endif

		#if stconfigTX & stconfigOCR_B
	
			#define stsTXDDRB	DDRB
			#define stsTXPORTB	PORTB
			#define stsTXPINB	PINB4			// OC0B
	
		#endif
	
	#elif stconfigTX & stconfigTIMER1

		#if stconfigTX & stconfigOCR_A

			#define stsTXDDRA	DDRD
			#define stsTXPORTA	PORTD
			#define stsTXPINA	PIND5			// OC1A
			
		#endif
	
		#if stconfigTX & stconfigOCR_B
	
			#define stsTXDDRB	DDRD
			#define stsTXPORTB	PORTD
			#define stsTXPINB	PIND6			// OC1B
	
		#endif
	
	#elif stconfigTX & stconfigTIMER2

		#if stconfigTX & stconfigOCR_A
	
			#define stsTXDDRA	DDRD
			#define stsTXPORTA	PORTD
			#define stsTXPINA	PIND7			// OC2A
		
		#endif
		
		#if stconfigTX & stconfigOCR_B
	
			#define stsTXDDRB	DDRD
			#define stsTXPORTB	PORTD
			#define stsTXPINB	PIND6			// OC2B
			
		#endif
		
	#elif stconfigTX & stconfigTIMER3

		#if stconfigTX & stconfigOCR_A
	
			#define stsTXDDRA	DDRB
			#define stsTXPORTA	PORTB
			#define stsTXPINA	PINB6			// OC3A
		
		#endif
		
		#if stconfigTX & stconfigOCR_B
	
			#define stsTXDDRB	DDRB
			#define stsTXPORTB	PORTB
			#define stsTXPINB	PINB7			// OC3B
			
		#endif
		
	#endif

	#if (stconfigRX & stconfigTIMER1) && (stconfigRX & stconfigICP1)

		#define stsRXDDRICP_1	DDRD
		#define stsRXPORTICP_1	PORTD
		#define stsRXPINICP_1	PIND6			// ICP1
	
	#endif

	#if (stconfigRX & stconfigTIMER3) && (stconfigRX & stconfigICP3)
	
		#define stsRXDDRICP_3	DDRB
		#define stsRXPORTICP_3	PORTB
		#define stsRXPINICP_3	PINB5			// ICP3
	
	#endif

	#if stconfigRX & stconfigINT0

		#define stsRXDDR_0	DDRD
		#define stsRXPORT_0	PORTD
		#define stsRXPIN_0	PIND2				// INT0

	#endif

	#if stconfigRX & stconfigINT1

		#define stsRXDDR_1	DDRD
		#define stsRXPORT_1	PORTD
		#define stsRXPIN_1	PIND3				// INT1

	#endif

	#if stconfigRX & stconfigINT2

		#define stsRXDDR_2	DDRB
		#define stsRXPORT_2	PORTB
		#define stsRXPIN_2	PINB2				// INT2

	#endif

	#if stconfigRX
	
		#define stsPORTCOUNT	4
	
	#endif
	
	#if stconfigRX & stconfigPCPORTA
	
		#define stsRXDDR	DDRA
		#define stsRXPORT	PORTA
		#define stsRXINPUT	PINA				// PCINT0-7
	
	#elif stconfigRX & stconfigPCPORTB
	
		#define stsRXDDR	DDRB
		#define stsRXPORT	PORTB
		#define stsRXINPUT	PINB				// PCINT8-15
	
	#elif stconfigRX & stconfigPCPORTC
	
		#define stsRXDDR	DDRC
		#define stsRXPORT	PORTC
		#define stsRXINPUT	PINC				// PCINT16-23
		
	#elif stconfigRX & stconfigPCPORTD
	
		#define stsRXDDR	DDRD
		#define stsRXPORT	PORTD
		#define stsRXINPUT	PIND				// PCINT24-31
		
	#endif

// Unknown board
#else
#error "Please define board timer and pins"
#endif

//#endif /* SEATALK_BOARDS_H_ */
