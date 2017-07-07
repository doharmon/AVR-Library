/*---------------------------------------------------------------------------
 *
 * SeaTalk_timers.h
 *
 * Created: 12/25/2015 5:39:34 PM
 *  Author: dharmon
 *
 *-------------------------------------------------------------------------*/

//#ifndef SEATALK_TIMERS_H_
//#define SEATALK_TIMERS_H_

//////////////////////////////////////////////////////////////////////////////
// Verify that only one timer is being defined at a time
//
#if ((stconfigTX & stconfigTIMER_MASK)-1) & (stconfigTX & stconfigTIMER_MASK)
	#error "Multiple TX timers defined. Only one at a time can be defined."
#endif

#if ((stconfigRX & stconfigTIMER_MASK)-1) & (stconfigRX & stconfigTIMER_MASK)
	#error "Multiple RX timers defined. Only one at a time can be defined."
#endif

//////////////////////////////////////////////////////////////////////////////
// Timer Size
//
#if (stconfigTX & (stconfigTIMER0 | stconfigTIMER2)) || (stconfigRX & (stconfigTIMER0 | stconfigTIMER2))

	#define	stsBITS		8						// 8 Bit Timer/Counter
	#define	stsDS		.ds.b					// Allocate 8 bits for variable

#else

	#define	stsBITS		16						// 16 Bit Timer/Counter
	#define	stsDS		.ds.w					// Allocate 16 bits for variable

#endif

//////////////////////////////////////////////////////////////////////////////
// Timer defines regardless if TX or RX is being used
//
#if (stconfigTX & stconfigTIMER0) || (stconfigRX & stconfigTIMER0)

	#define	stsTCCRB	TCCR0B						// Timer/Counter Control Register B
	#define	stsTIMSK	TIMSK0						// Timer/Counter Interrupt Mask

	#define stsNO_PRESCALE		(1<<CS00)
	#define stsPRESCALE_8		(1<<CS01)
	#define stsPRESCALE_64		(1<<CS01) | (1<<CS00)
	#define stsPRESCALE_256		(1<<CS02)
	#define stsPRESCALE_1024	(1<<CS02) | (1<<CS00)
	
	#define stsBSS_START		__bss_stserial0_start
	#define stsBSS_END			__bss_stserial0_end
	#define stsBSS_SECTIONNAME	.bss.stserial0
	#define stsDATA_SECTIONNAME	.data.stserial0
	#define stsTEXT_SECTIONNAME	.text.stserial0

	#define stsVAR_TIMER_PREFIX		st0
	#define stsFUNC_TIMER_PREFIX	ST0

#elif (stconfigTX & stconfigTIMER1) || (stconfigRX & stconfigTIMER1)

	#define	stsTCCRB	TCCR1B						// Timer/Counter Control Register B
	#define	stsTIMSK	TIMSK1						// Timer/Counter Interrupt Mask

	#define stsNO_PRESCALE		(1<<CS10)
	#define stsPRESCALE_8		(1<<CS11)
	#define stsPRESCALE_64		(1<<CS11) | (1<<CS10)
	#define stsPRESCALE_256		(1<<CS12)
	#define stsPRESCALE_1024	(1<<CS12) | (1<<CS10)

	#define stsBSS_START		__bss_stserial1_start
	#define stsBSS_END			__bss_stserial1_end
	#define stsBSS_SECTIONNAME	.bss.stserial1
	#define stsDATA_SECTIONNAME	.data.stserial1
	#define stsTEXT_SECTIONNAME	.text.stserial1

	#define stsVAR_TIMER_PREFIX		st1
	#define stsFUNC_TIMER_PREFIX	ST1

#elif (stconfigTX & stconfigTIMER2) || (stconfigRX & stconfigTIMER2)

	#define	stsTCCRB	TCCR2B						// Timer/Counter Control Register B
	#define	stsTIMSK	TIMSK2						// Timer/Counter Interrupt Mask

	#define stsNO_PRESCALE		(1<<CS20)
	#define stsPRESCALE_8		(1<<CS21)
	#define stsPRESCALE_32		(1<<CS21) | (1<<CS20)
	#define stsPRESCALE_64		(1<<CS22)
	#define stsPRESCALE_128		(1<<CS22) | (1<<CS20)
	#define stsPRESCALE_256		(1<<CS22) | (1<<CS21)
	#define stsPRESCALE_1024	(1<<CS22) | (1<<CS21) | (1<<CS20)

	#define stsBSS_START		__bss_stserial2_start
	#define stsBSS_END			__bss_stserial2_end
	#define stsBSS_SECTIONNAME	.bss.stserial2
	#define stsDATA_SECTIONNAME	.data.stserial2
	#define stsTEXT_SECTIONNAME	.text.stserial2
	
	#define stsVAR_TIMER_PREFIX		st2
	#define stsFUNC_TIMER_PREFIX	ST2
	
#elif (stconfigTX & stconfigTIMER3) || (stconfigRX & stconfigTIMER3)

	#define	stsTCCRB	TCCR3B						// Timer/Counter Control Register B
	#define	stsTIMSK	TIMSK3						// Timer/Counter Interrupt Mask

	#define stsBSS_START		__bss_stserial3_start
	#define stsBSS_END			__bss_stserial3_end
	#define stsBSS_SECTIONNAME	.bss.stserial3
	#define stsDATA_SECTIONNAME	.data.stserial3
	#define stsTEXT_SECTIONNAME	.text.stserial3
	
	#define stsVAR_TIMER_PREFIX		st3
	#define stsFUNC_TIMER_PREFIX	ST3

#endif

//////////////////////////////////////////////////////////////////////////////
// TX Configuration
//
#if stconfigTX & stconfigTIMER0

	#define	stsTXTCNT	TCNT0					// Timer/Counter
	#define	stsTXTCCRA	TCCR0A					// Timer/Counter Control Register A
	#define stsTXFOCR	TCCR0B					// Timer/Counter Control Register B for Forced Output
	#define	stsTXTIFR	TIFR0					// Timer/Counter Interrupt Flag Register

	#if stconfigTX & stconfigOCR_A

		#define	stsTX_IR_A	TIMER0_COMPA_vect	// TX Interrupt Routine Vector
		#define	stsTXOCR_A	OCR0A				// Output Compare Register
		#define	stsTXOCF_A	OCF0A				// Output Compare Flag
		#define	stsTXCOM0_A	COM0A0				// Compare Match Output Mode 0
		#define	stsTXCOM1_A	COM0A1				// Compare Match Output Mode 1
		#define	stsTXFOC_A	FOC0A				// Force Output Compare
		#define stsTXOCIE_A	OCIE0A				// Timer/Counter Output Compare Match Interrupt Enable

		#define stsVAR_TX_PREFIX_A	st0a_		// Prefix to add to variable names

	#endif
	
	#if stconfigTX & stconfigOCR_B

		#define	stsTX_IR_B	TIMER0_COMPB_vect	// TX Interrupt Routine Vector
		#define	stsTXOCR_B	OCR0B				// Output Compare Register
		#define	stsTXOCF_B	OCF0B				// Output Compare Flag
		#define	stsTXCOM0_B	COM0B0				// Compare Match Output Mode 0
		#define	stsTXCOM1_B	COM0B1				// Compare Match Output Mode 1
		#define	stsTXFOC_B	FOC0B				// Force Output Compare
		#define stsTXOCIE_B	OCIE0B				// Timer/Counter Output Compare Match Interrupt Enable

		#define stsVAR_TX_PREFIX_B	st0b_		// Prefix to add to variable names

	#endif

#elif stconfigTX & stconfigTIMER1

	#define	stsTXTCNT	TCNT1L					// Timer/Counter Low Byte
	#define	stsTXTCNTH	TCNT1H					// Timer/Counter High Byte
	#define	stsTXTCCRA	TCCR1A					// Timer/Counter Control Register A
	#define stsTXFOCR	TCCR1C					// Timer/Counter Control Register C for Forced Output
	#define	stsTXTIFR	TIFR1					// Timer/Counter Interrupt Flag Register

	#if stconfigTX & stconfigOCR_A

		#define	stsTX_IR_A	TIMER1_COMPA_vect	// TX Interrupt Routine Vector
		#define	stsTXOCR_A	OCR1AL				// Output Compare Register Low Byte
		#define	stsTXOCRH_A	OCR1AH				// Output Compare Register High Byte
		#define	stsTXOCF_A	OCF1A				// Output Compare Flag
		#define	stsTXCOM0_A	COM1A0				// Compare Match Output Mode 0
		#define	stsTXCOM1_A	COM1A1				// Compare Match Output Mode 1
		#define	stsTXFOC_A	FOC1A				// Force Output Compare
		#define stsTXOCIE_A	OCIE1A				// Timer/Counter Output Compare Match Interrupt Enable

		#define stsVAR_TX_PREFIX_A	st1a_		// Prefix to add to variable names

	#endif
	
	#if stconfigTX & stconfigOCR_B

		#define	stsTX_IR_B	TIMER1_COMPB_vect	// TX Interrupt Routine Vector
		#define	stsTXOCR_B	OCR1BL				// Output Compare Register
		#define	stsTXOCRH_B	OCR1BH				// Output Compare Register
		#define	stsTXOCF_B	OCF1B				// Output Compare Flag
		#define	stsTXCOM0_B	COM1B0				// Compare Match Output Mode 0
		#define	stsTXCOM1_B	COM1B1				// Compare Match Output Mode 1
		#define	stsTXFOC_B	FOC1B				// Force Output Compare
		#define stsTXOCIE_B	OCIE1B				// Timer/Counter Output Compare Match Interrupt Enable

		#define stsVAR_TX_PREFIX_B	st1b_		// Prefix to add to variable names

	#endif

#elif stconfigTX & stconfigTIMER2

	#define	stsTXTCNT	TCNT2					// Timer/Counter
	#define	stsTXTCCRA	TCCR2A					// Timer/Counter Control Register A
	#define stsTXFOCR	TCCR2B					// Timer/Counter Control Register B for Forced Output
	#define	stsTXTIFR	TIFR2					// Timer/Counter Interrupt Flag Register

	#if stconfigTX & stconfigOCR_A

		#define	stsTX_IR_A	TIMER2_COMPA_vect	// TX Interrupt Routine Vector
		#define	stsTXOCR_A	OCR2A				// Output Compare Register
		#define	stsTXOCF_A	OCF2A				// Output Compare Flag
		#define	stsTXCOM0_A	COM2A0				// Compare Match Output Mode 0
		#define	stsTXCOM1_A	COM2A1				// Compare Match Output Mode 1
		#define	stsTXFOC_A	FOC2A				// Force Output Compare
		#define stsTXOCIE_A	OCIE2A				// Timer/Counter Output Compare Match Interrupt Enable

		#define stsVAR_TX_PREFIX_A	st2a_		// Prefix to add to variable names

	#endif
	
	#if stconfigTX & stconfigOCR_B

		#define	stsTX_IR_B	TIMER2_COMPB_vect	// TX Interrupt Routine Vector
		#define	stsTXOCR_B	OCR2B				// Output Compare Register
		#define	stsTXOCF_B	OCF2B				// Output Compare Flag
		#define	stsTXCOM0_B	COM2B0				// Compare Match Output Mode 0
		#define	stsTXCOM1_B	COM2B1				// Compare Match Output Mode 1
		#define	stsTXFOC_B	FOC2B				// Force Output Compare
		#define stsTXOCIE_B	OCIE2B				// Timer/Counter Output Compare Match Interrupt Enable

		#define stsVAR_TX_PREFIX_B	st2b_		// Prefix to add to variable names

	#endif

#elif stconfigTX & stconfigTIMER3

	#define	stsTXTCNT	TCNT3L					// Timer/Counter Low Byte
	#define	stsTXTCNTH	TCNT3H					// Timer/Counter High Byte
	#define	stsTXTCCRA	TCCR3A					// Timer/Counter Control Register A
	#define stsTXFOCR	TCCR3C					// Timer/Counter Control Register C for Forced Output
	#define	stsTXTIFR	TIFR3					// Timer/Counter Interrupt Flag Register

	#if stconfigTX & stconfigOCR_A

		#define	stsTX_IR_A	TIMER3_COMPA_vect	// TX Interrupt Routine Vector
		#define	stsTXOCR_A	OCR3AL				// Output Compare Register Low Byte
		#define	stsTXOCRH_A	OCR3AH				// Output Compare Register High Byte
		#define	stsTXOCF_A	OCF3A				// Output Compare Flag
		#define	stsTXCOM0_A	COM3A0				// Compare Match Output Mode 0
		#define	stsTXCOM1_A	COM3A1				// Compare Match Output Mode 1
		#define	stsTXFOC_A	FOC3A				// Force Output Compare
		#define stsTXOCIE_A	OCIE3A				// Timer/Counter Output Compare Match Interrupt Enable

		#define stsVAR_TX_PREFIX_A	t3a_		// Prefix to add to variable names

	#endif
	
	#if stconfigTX & stconfigOCR_B

		#define	stsTX_IR_B	TIMER3_COMPB_vect	// TX Interrupt Routine Vector
		#define	stsTXOCR_B	OCR3BL				// Output Compare Register
		#define	stsTXOCRH_B	OCR3BH				// Output Compare Register
		#define	stsTXOCF_B	OCF3B				// Output Compare Flag
		#define	stsTXCOM0_B	COM3B0				// Compare Match Output Mode 0
		#define	stsTXCOM1_B	COM3B1				// Compare Match Output Mode 1
		#define	stsTXFOC_B	FOC3B				// Force Output Compare
		#define stsTXOCIE_B	OCIE3B				// Timer/Counter Output Compare Match Interrupt Enable

		#define stsVAR_TX_PREFIX_B	t3b_		// Prefix to add to variable names

	#endif

#endif

//////////////////////////////////////////////////////////////////////////////
// RX Configuration
//
#if stconfigRX & stconfigTIMER0

	#define	stsRXTCNT	TCNT0					// Timer/Counter
	#define	stsRXTIFR	TIFR0					// Timer/Counter Interrupt Flag Register

	#if stconfigRX & stconfigOCR_A

		#define	stsRX_SB_IR	TIMER0_COMPA_vect	// RX Stop Bit Interrupt Routine Vector
		#define	stsRXOCR	OCR0A				// Output Compare Register
		#define	stsRXOCF	OCF0A				// Output Compare Flag
		#define	stsRXOCIE	OCIE0A				// Timer/Counter Output Compare Match Interrupt Enable

		#define stsVAR_RX_PREFIX_A	st0a_		// Prefix to add to variable names

	#elif stconfigRX & stconfigOCR_B

		#define	stsRX_SB_IR	TIMER0_COMPB_vect	// RX Stop Bit Interrupt Routine Vector
		#define	stsRXOCR	OCR0B				// Output Compare Register
		#define	stsRXOCF	OCF0B				// Output Compare Flag
		#define	stsRXOCIE	OCIE0B				// Timer/Counter Output Compare Match Interrupt Enable

		#define stsVAR_RX_PREFIX_B	st0b_		// Prefix to add to variable names

	#endif

#elif stconfigRX & stconfigTIMER1

	#define	stsRXTIFR		TIFR1				// Timer/Counter Interrupt Flag Register
	#define	stsRXEICRAICP	TCCR1B				// Timer/Counter Control Register B
	#define stsRXEIFRICP	TIFR1				// Timer Counter Interrupt Flag Register
	#define stsRXEIMSKICP	TIMSK1				// Timer/Counter Interrupt Mask Register
	#define	stsRXICESICP_1	ICES1				// Input Capture Edge Select
	#define stsRXINTFICP_1	ICF1				// Input Capture Flag
	#define stsRXINTICP_1	ICIE1				// Input Capture Interrupt Enable

	#if stconfigRX & stconfigICP1
	
		#define	stsRX_IR	TIMER1_CAPT_vect	// RX Interrupt Routine Vector
		#define	stsRXTCNT	ICR1L				// Input Capture Register Low Byte
		#define	stsRXTCNTH	ICR1H				// Input Capture Register High Byte

		#define stsREG_ICP_SUFFIX	_1
	
	#else
	
		#define	stsRXTCNT	TCNT1L				// Timer/Counter Low Byte
		#define	stsRXTCNTH	TCNT1H				// Timer/Counter High Byte

	#endif

	#if stconfigRX & stconfigOCR_A

		#define	stsRX_SB_IR	TIMER1_COMPA_vect	// RX Stop Bit Interrupt Routine Vector
		#define	stsRXOCR	OCR1AL				// Output Compare Register Low Byte
		#define	stsRXOCRH	OCR1AH				// Output Compare Register High Byte
		#define	stsRXOCF	OCF1A				// Output Compare Flag
		#define	stsRXOCIE	OCIE1A				// Timer/Counter Output Compare Match Interrupt Enable

		#define stsVAR_RX_PREFIX_A	st1a_		// Prefix to add to variable names

	#elif stconfigRX & stconfigOCR_B

		#define	stsRX_SB_IR	TIMER1_COMPB_vect	// RX Stop Bit Interrupt Routine Vector
		#define	stsRXOCR	OCR1BL				// Output Compare Register Low Byte
		#define	stsRXOCRH	OCR1BH				// Output Compare Register High Byte
		#define	stsRXOCF	OCF1B				// Output Compare Flag
		#define	stsRXOCIE	OCIE1B				// Timer/Counter Output Compare Match Interrupt Enable

		#define stsVAR_RX_PREFIX_B	st1b_		// Prefix to add to variable names

	#endif
	
#elif stconfigRX & stconfigTIMER2

	#define	stsRXTCNT	TCNT2					// Timer/Counter
	#define	stsRXTIFR	TIFR2					// Timer/Counter Interrupt Flag Register

	#if stconfigRX & stconfigOCR_A

		#define	stsRX_SB_IR	TIMER2_COMPA_vect	// RX Stop Bit Interrupt Routine Vector
		#define	stsRXOCR	OCR2A				// Output Compare Register
		#define	stsRXOCF	OCF2A				// Output Compare Flag
		#define	stsRXOCIE	OCIE2A				// Timer/Counter Output Compare Match Interrupt Enable

		#define stsVAR_RX_PREFIX_A	st2a_		// Prefix to add to variable names

	#elif stconfigRX & stconfigOCR_B

		#define	stsRX_SB_IR	TIMER2_COMPB_vect	// RX Stop Bit Interrupt Routine Vector
		#define	stsRXOCR	OCR2B				// Output Compare Register
		#define	stsRXOCF	OCF2B				// Output Compare Flag
		#define	stsRXOCIE	OCIE2B				// Timer/Counter Output Compare Match Interrupt Enable

		#define stsVAR_RX_PREFIX_B	st2b_		// Prefix to add to variable names

	#endif

#elif stconfigRX & stconfigTIMER3

	#define	stsRXTIFR		TIFR3				// Timer/Counter Interrupt Flag Register
	#define	stsRXEICRAICP	TCCR3B				// Timer/Counter Control Register B
	#define stsRXEIFRICP	TIFR3				// Timer Counter Interrupt Flag Register
	#define stsRXEIMSKICP	TIMSK3				// Timer/Counter Interrupt Mask Register
	#define	stsRXICESICP_1	ICES3				// Input Capture Edge Select
	#define stsRXINTFICP_1	ICF3				// Input Capture Flag
	#define stsRXINTICP_1	ICIE3				// Input Capture Interrupt Enable

	#if stconfigRX & stconfigICP3
	
		#define	stsRX_IR	TIMER3_CAPT_vect	// RX Interrupt Routine Vector
		#define	stsRXTCNT	ICR3L				// Input Capture Register Low Byte
		#define	stsRXTCNTH	ICR3H				// Input Capture Register High Byte

		#define stsREG_ICP_SUFFIX	_3
	
	#else
	
		#define	stsRXTCNT	TCNT3L				// Timer/Counter Low Byte
		#define	stsRXTCNTH	TCNT3H				// Timer/Counter High Byte

	#endif

	#if stconfigRX & stconfigOCR_A

		#define	stsRX_SB_IR	TIMER3_COMPA_vect	// RX Stop Bit Interrupt Routine Vector
		#define	stsRXOCR	OCR3AL				// Output Compare Register Low Byte
		#define	stsRXOCRH	OCR3AH				// Output Compare Register High Byte
		#define	stsRXOCF	OCF3A				// Output Compare Flag
		#define	stsRXOCIE	OCIE3A				// Timer/Counter Output Compare Match Interrupt Enable

		#define stsVAR_RX_PREFIX_A	t3a_		// Prefix to add to variable names

	#elif stconfigRX & stconfigOCR_B

		#define	stsRX_SB_IR	TIMER3_COMPB_vect	// RX Stop Bit Interrupt Routine Vector
		#define	stsRXOCR	OCR3BL				// Output Compare Register Low Byte
		#define	stsRXOCRH	OCR3BH				// Output Compare Register High Byte
		#define	stsRXOCF	OCF3B				// Output Compare Flag
		#define	stsRXOCIE	OCIE3B				// Timer/Counter Output Compare Match Interrupt Enable

		#define stsVAR_RX_PREFIX_B	t3b_		// Prefix to add to variable names

	#endif
	
#endif

#if stconfigRX & stconfigINT_MASK

	#define	stsRXEICRA	EICRA					// External Interrupt Control Register A
	#define stsRXEIMSK	EIMSK					// External Interrupt Mask Register
	#define stsRXEIFR	EIFR					// External Interrupt Flag Register
	#define	stsRXISC0_0	ISC00					// Interrupt Sense Control Bit 0
	#define	stsRXISC1_0	ISC01					// Interrupt Sense Control Bit 1
	#define stsRXINT_0	INT0					// External Interrupt Request Enable
	#define stsRXINTF_0	INTF0					// External Interrupt Flag
	#define	stsRXISC0_1	ISC10					// Interrupt Sense Control Bit 0
	#define	stsRXISC1_1	ISC11					// Interrupt Sense Control Bit 1
	#define stsRXINT_1	INT1					// External Interrupt Request Enable
	#define stsRXINTF_1	INTF1					// External Interrupt Flag
	#define	stsRXISC0_2	ISC20					// Interrupt Sense Control Bit 0
	#define	stsRXISC1_2	ISC21					// Interrupt Sense Control Bit 1
	#define stsRXINT_2	INT2					// External Interrupt Request Enable
	#define stsRXINTF_2	INTF2					// External Interrupt Flag

	#if stconfigRX & stconfigINT0

		#define	stsRX_IR			INT0_vect	// RX Interrupt Routine Vector
		#define stsREG_INT_SUFFIX	_0

	#elif stconfigRX & stconfigINT1

		#define	stsRX_IR			INT1_vect	// RX Interrupt Routine Vector
		#define stsREG_INT_SUFFIX	_1

	#elif stconfigRX & stconfigINT2

		#define	stsRX_IR			INT2_vect	// RX Interrupt Routine Vector
		#define stsREG_INT_SUFFIX	_2
	#endif

#endif

#if stconfigRX & stconfigPCPORT_MASK

	#define stsRXEIMSK	PCICR					// Pin Change Interrupt Control Register
	#define stsRXEIFR	PCIFR					// Pin Change Interrupt Flag Register

	#if stsPORTCOUNT == 3

		#if stconfigRX & stconfigPCPORTB

			#define	stsRX_IR	PCINT0_vect		// RX Interrupt Routine Vector
			#define stsRXPCMSK	PCMSK0			// Pin Change Mask Register
			#define stsRXINT	PCIE0			// Pin Change Interrupt Enable
			#define stsRXINTF	PCIF0			// Pin Change Interrupt Flag

		#elif stconfigRX & stconfigPCPORTC

			#define	stsRX_IR	PCINT1_vect		// RX Interrupt Routine Vector
			#define stsRXPCMSK	PCMSK1			// Pin Change Mask Register
			#define stsRXINT	PCIE1			// Pin Change Interrupt Enable
			#define stsRXINTF	PCIF1			// Pin Change Interrupt Flag

		#elif stconfigRX & stconfigPCPORTD

			#define	stsRX_IR	PCINT2_vect		// RX Interrupt Routine Vector
			#define stsRXPCMSK	PCMSK2			// Pin Change Mask Register
			#define stsRXINT	PCIE2			// Pin Change Interrupt Enable
			#define stsRXINTF	PCIF2			// Pin Change Interrupt Flag

		#endif

	#elif stsPORTCOUNT == 4

		#if stconfigRX & stconfigPCPORTA

			#define	stsRX_IR	PCINT0_vect		// RX Interrupt Routine Vector
			#define stsRXPCMSK	PCMSK0			// Pin Change Mask Register
			#define stsRXINT	PCIE0			// Pin Change Interrupt Enable
			#define stsRXINTF	PCIF0			// Pin Change Interrupt Flag

		#elif stconfigRX & stconfigPCPORTB

			#define	stsRX_IR	PCINT1_vect		// RX Interrupt Routine Vector
			#define stsRXPCMSK	PCMSK1			// Pin Change Mask Register
			#define stsRXINT	PCIE1			// Pin Change Interrupt Enable
			#define stsRXINTF	PCIF1			// Pin Change Interrupt Flag

		#elif stconfigRX & stconfigPCPORTC

			#define	stsRX_IR	PCINT2_vect		// RX Interrupt Routine Vector
			#define stsRXPCMSK	PCMSK2			// Pin Change Mask Register
			#define stsRXINT	PCIE2			// Pin Change Interrupt Enable
			#define stsRXINTF	PCIF2			// Pin Change Interrupt Flag

		#elif stconfigRX & stconfigPCPORTD

			#define	stsRX_IR	PCINT3_vect		// RX Interrupt Routine Vector
			#define stsRXPCMSK	PCMSK3			// Pin Change Mask Register
			#define stsRXINT	PCIE3			// Pin Change Interrupt Enable
			#define stsRXINTF	PCIF3			// Pin Change Interrupt Flag

		#endif

	#endif
	
#endif

//#endif /* SEATALK_TIMERS_H_ */
