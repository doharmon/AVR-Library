/*---------------------------------------------------------------------------
 *
 * tserial_timers.h
 *
 * Created: 12/25/2015 5:39:34 PM
 *  Author: dharmon
 *
 *-------------------------------------------------------------------------*/

//#ifndef TSERIAL_TIMERS_H_
//#define TSERIAL_TIMERS_H_

//////////////////////////////////////////////////////////////////////////////
// Verify that only one timer is being defined at a time
//
#if ((tsconfigTX & tsconfigTIMER_MASK)-1) & (tsconfigTX & tsconfigTIMER_MASK)
	#error "Multiple TX timers defined. Only one at a time can be defined."
#endif

#if ((tsconfigRX & tsconfigTIMER_MASK)-1) & (tsconfigRX & tsconfigTIMER_MASK)
	#error "Multiple RX timers defined. Only one at a time can be defined."
#endif

//////////////////////////////////////////////////////////////////////////////
// Timer Size
//
#if (tsconfigTX & (tsconfigTIMER0 | tsconfigTIMER2)) || (tsconfigRX & (tsconfigTIMER0 | tsconfigTIMER2))

	#define	tsBITS		8						// 8 Bit Timer/Counter
	#define	tsDS		.ds.b					// Allocate 8 bits for variable

#else

	#define	tsBITS		16						// 16 Bit Timer/Counter
	#define	tsDS		.ds.w					// Allocate 16 bits for variable

#endif

//////////////////////////////////////////////////////////////////////////////
// Timer defines regardless if TX or RX is being used
//
#if (tsconfigTX & tsconfigTIMER0) || (tsconfigRX & tsconfigTIMER0)

	#define	tsTCCRB	TCCR0B						// Timer/Counter Control Register B
	#define	tsTIMSK	TIMSK0						// Timer/Counter Interrupt Mask

	#define tsNO_PRESCALE		(1<<CS00)
	#define tsPRESCALE_8		(1<<CS01)
	#define tsPRESCALE_64		(1<<CS01) | (1<<CS00)
	#define tsPRESCALE_256		(1<<CS02)
	#define tsPRESCALE_1024		(1<<CS02) | (1<<CS00)
	
	#define tsBSS_START			__bss_tserial0_start
	#define tsBSS_END			__bss_tserial0_end
	#define tsBSS_SECTIONNAME	.bss.tserial0
	#define tsDATA_SECTIONNAME	.data.tserial0
	#define tsTEXT_SECTIONNAME	.text.tserial0

	#define tsVAR_TIMER_PREFIX	t0
	#define tsFUNC_TIMER_PREFIX	T0

#elif (tsconfigTX & tsconfigTIMER1) || (tsconfigRX & tsconfigTIMER1)

	#define	tsTCCRB	TCCR1B						// Timer/Counter Control Register B
	#define	tsTIMSK	TIMSK1						// Timer/Counter Interrupt Mask

	#define tsNO_PRESCALE		(1<<CS10)
	#define tsPRESCALE_8		(1<<CS11)
	#define tsPRESCALE_64		(1<<CS11) | (1<<CS10)
	#define tsPRESCALE_256		(1<<CS12)
	#define tsPRESCALE_1024		(1<<CS12) | (1<<CS10)

	#define tsBSS_START			__bss_tserial1_start
	#define tsBSS_END			__bss_tserial1_end
	#define tsBSS_SECTIONNAME	.bss.tserial1
	#define tsDATA_SECTIONNAME	.data.tserial1
	#define tsTEXT_SECTIONNAME	.text.tserial1

	#define tsVAR_TIMER_PREFIX	t1
	#define tsFUNC_TIMER_PREFIX	T1

#elif (tsconfigTX & tsconfigTIMER2) || (tsconfigRX & tsconfigTIMER2)

	#define	tsTCCRB	TCCR2B						// Timer/Counter Control Register B
	#define	tsTIMSK	TIMSK2						// Timer/Counter Interrupt Mask

	#define tsNO_PRESCALE		(1<<CS20)
	#define tsPRESCALE_8		(1<<CS21)
	#define tsPRESCALE_32		(1<<CS21) | (1<<CS20)
	#define tsPRESCALE_64		(1<<CS22)
	#define tsPRESCALE_128		(1<<CS22) | (1<<CS20)
	#define tsPRESCALE_256		(1<<CS22) | (1<<CS21)
	#define tsPRESCALE_1024		(1<<CS22) | (1<<CS21) | (1<<CS20)

	#define tsBSS_START			__bss_tserial2_start
	#define tsBSS_END			__bss_tserial2_end
	#define tsBSS_SECTIONNAME	.bss.tserial2
	#define tsDATA_SECTIONNAME	.data.tserial2
	#define tsTEXT_SECTIONNAME	.text.tserial2
	
	#define tsVAR_TIMER_PREFIX	t2
	#define tsFUNC_TIMER_PREFIX	T2
	
#elif (tsconfigTX & tsconfigTIMER3) || (tsconfigRX & tsconfigTIMER3)

	#define	tsTCCRB	TCCR3B						// Timer/Counter Control Register B
	#define	tsTIMSK	TIMSK3						// Timer/Counter Interrupt Mask

	#define tsNO_PRESCALE		(1<<CS30)
	#define tsPRESCALE_8		(1<<CS31)
	#define tsPRESCALE_64		(1<<CS31) | (1<<CS30)
	#define tsPRESCALE_256		(1<<CS32)
	#define tsPRESCALE_1024		(1<<CS32) | (1<<CS30)

	#define tsBSS_START			__bss_tserial3_start
	#define tsBSS_END			__bss_tserial3_end
	#define tsBSS_SECTIONNAME	.bss.tserial3
	#define tsDATA_SECTIONNAME	.data.tserial3
	#define tsTEXT_SECTIONNAME	.text.tserial3
	
	#define tsVAR_TIMER_PREFIX	t3
	#define tsFUNC_TIMER_PREFIX	T3

#endif

//////////////////////////////////////////////////////////////////////////////
// TX Configuration
//
#if tsconfigTX & tsconfigTIMER0

	#define	tsTXTCNT	TCNT0					// Timer/Counter
	#define	tsTXTCCRA	TCCR0A					// Timer/Counter Control Register A
	#define tsTXFOCR	TCCR0B					// Timer/Counter Control Register B for Forced Output
	#define	tsTXTIFR	TIFR0					// Timer/Counter Interrupt Flag Register

	#if tsconfigTX & tsconfigOCR_A

		#define	tsTX_IR_A	TIMER0_COMPA_vect	// TX Interrupt Routine Vector
		#define	tsTXOCR_A	OCR0A				// Output Compare Register
		#define	tsTXOCF_A	OCF0A				// Output Compare Flag
		#define	tsTXCOM0_A	COM0A0				// Compare Match Output Mode 0
		#define	tsTXCOM1_A	COM0A1				// Compare Match Output Mode 1
		#define	tsTXFOC_A	FOC0A				// Force Output Compare
		#define tsTXOCIE_A	OCIE0A				// Timer/Counter Output Compare Match Interrupt Enable

		#define tsVAR_TX_PREFIX_A	t0a_		// Prefix to add to variable names

	#endif
	
	#if tsconfigTX & tsconfigOCR_B

		#define	tsTX_IR_B	TIMER0_COMPB_vect	// TX Interrupt Routine Vector
		#define	tsTXOCR_B	OCR0B				// Output Compare Register
		#define	tsTXOCF_B	OCF0B				// Output Compare Flag
		#define	tsTXCOM0_B	COM0B0				// Compare Match Output Mode 0
		#define	tsTXCOM1_B	COM0B1				// Compare Match Output Mode 1
		#define	tsTXFOC_B	FOC0B				// Force Output Compare
		#define tsTXOCIE_B	OCIE0B				// Timer/Counter Output Compare Match Interrupt Enable

		#define tsVAR_TX_PREFIX_B	t0b_		// Prefix to add to variable names

	#endif

#elif tsconfigTX & tsconfigTIMER1

	#define	tsTXTCNT	TCNT1L					// Timer/Counter Low Byte
	#define	tsTXTCNTH	TCNT1H					// Timer/Counter High Byte
	#define	tsTXTCCRA	TCCR1A					// Timer/Counter Control Register A
	#define tsTXFOCR	TCCR1C					// Timer/Counter Control Register C for Forced Output
	#define	tsTXTIFR	TIFR1					// Timer/Counter Interrupt Flag Register

	#if tsconfigTX & tsconfigOCR_A

		#define	tsTX_IR_A	TIMER1_COMPA_vect	// TX Interrupt Routine Vector
		#define	tsTXOCR_A	OCR1AL				// Output Compare Register Low Byte
		#define	tsTXOCRH_A	OCR1AH				// Output Compare Register High Byte
		#define	tsTXOCF_A	OCF1A				// Output Compare Flag
		#define	tsTXCOM0_A	COM1A0				// Compare Match Output Mode 0
		#define	tsTXCOM1_A	COM1A1				// Compare Match Output Mode 1
		#define	tsTXFOC_A	FOC1A				// Force Output Compare
		#define tsTXOCIE_A	OCIE1A				// Timer/Counter Output Compare Match Interrupt Enable

		#define tsVAR_TX_PREFIX_A	t1a_		// Prefix to add to variable names

	#endif
	
	#if tsconfigTX & tsconfigOCR_B

		#define	tsTX_IR_B	TIMER1_COMPB_vect	// TX Interrupt Routine Vector
		#define	tsTXOCR_B	OCR1BL				// Output Compare Register
		#define	tsTXOCRH_B	OCR1BH				// Output Compare Register
		#define	tsTXOCF_B	OCF1B				// Output Compare Flag
		#define	tsTXCOM0_B	COM1B0				// Compare Match Output Mode 0
		#define	tsTXCOM1_B	COM1B1				// Compare Match Output Mode 1
		#define	tsTXFOC_B	FOC1B				// Force Output Compare
		#define tsTXOCIE_B	OCIE1B				// Timer/Counter Output Compare Match Interrupt Enable

		#define tsVAR_TX_PREFIX_B	t1b_		// Prefix to add to variable names

	#endif

#elif tsconfigTX & tsconfigTIMER2

	#define	tsTXTCNT	TCNT2					// Timer/Counter
	#define	tsTXTCCRA	TCCR2A					// Timer/Counter Control Register A
	#define tsTXFOCR	TCCR2B					// Timer/Counter Control Register B for Forced Output
	#define	tsTXTIFR	TIFR2					// Timer/Counter Interrupt Flag Register

	#if tsconfigTX & tsconfigOCR_A

		#define	tsTX_IR_A	TIMER2_COMPA_vect	// TX Interrupt Routine Vector
		#define	tsTXOCR_A	OCR2A				// Output Compare Register
		#define	tsTXOCF_A	OCF2A				// Output Compare Flag
		#define	tsTXCOM0_A	COM2A0				// Compare Match Output Mode 0
		#define	tsTXCOM1_A	COM2A1				// Compare Match Output Mode 1
		#define	tsTXFOC_A	FOC2A				// Force Output Compare
		#define tsTXOCIE_A	OCIE2A				// Timer/Counter Output Compare Match Interrupt Enable

		#define tsVAR_TX_PREFIX_A	t2a_		// Prefix to add to variable names

	#endif
	
	#if tsconfigTX & tsconfigOCR_B

		#define	tsTX_IR_B	TIMER2_COMPB_vect	// TX Interrupt Routine Vector
		#define	tsTXOCR_B	OCR2B				// Output Compare Register
		#define	tsTXOCF_B	OCF2B				// Output Compare Flag
		#define	tsTXCOM0_B	COM2B0				// Compare Match Output Mode 0
		#define	tsTXCOM1_B	COM2B1				// Compare Match Output Mode 1
		#define	tsTXFOC_B	FOC2B				// Force Output Compare
		#define tsTXOCIE_B	OCIE2B				// Timer/Counter Output Compare Match Interrupt Enable

		#define tsVAR_TX_PREFIX_B	t2b_		// Prefix to add to variable names

	#endif

#elif tsconfigTX & tsconfigTIMER3

	#define	tsTXTCNT	TCNT3L					// Timer/Counter Low Byte
	#define	tsTXTCNTH	TCNT3H					// Timer/Counter High Byte
	#define	tsTXTCCRA	TCCR3A					// Timer/Counter Control Register A
	#define tsTXFOCR	TCCR3C					// Timer/Counter Control Register C for Forced Output
	#define	tsTXTIFR	TIFR3					// Timer/Counter Interrupt Flag Register

	#if tsconfigTX & tsconfigOCR_A

		#define	tsTX_IR_A	TIMER3_COMPA_vect	// TX Interrupt Routine Vector
		#define	tsTXOCR_A	OCR3AL				// Output Compare Register Low Byte
		#define	tsTXOCRH_A	OCR3AH				// Output Compare Register High Byte
		#define	tsTXOCF_A	OCF3A				// Output Compare Flag
		#define	tsTXCOM0_A	COM3A0				// Compare Match Output Mode 0
		#define	tsTXCOM1_A	COM3A1				// Compare Match Output Mode 1
		#define	tsTXFOC_A	FOC3A				// Force Output Compare
		#define tsTXOCIE_A	OCIE3A				// Timer/Counter Output Compare Match Interrupt Enable

		#define tsVAR_TX_PREFIX_A	t3a_		// Prefix to add to variable names

	#endif
	
	#if tsconfigTX & tsconfigOCR_B

		#define	tsTX_IR_B	TIMER3_COMPB_vect	// TX Interrupt Routine Vector
		#define	tsTXOCR_B	OCR3BL				// Output Compare Register
		#define	tsTXOCRH_B	OCR3BH				// Output Compare Register
		#define	tsTXOCF_B	OCF3B				// Output Compare Flag
		#define	tsTXCOM0_B	COM3B0				// Compare Match Output Mode 0
		#define	tsTXCOM1_B	COM3B1				// Compare Match Output Mode 1
		#define	tsTXFOC_B	FOC3B				// Force Output Compare
		#define tsTXOCIE_B	OCIE3B				// Timer/Counter Output Compare Match Interrupt Enable

		#define tsVAR_TX_PREFIX_B	t3b_		// Prefix to add to variable names

	#endif

#endif

//////////////////////////////////////////////////////////////////////////////
// RX Configuration
//
#if tsconfigRX & tsconfigTIMER0

	#define	tsRXTCNT	TCNT0					// Timer/Counter
	#define	tsRXTIFR	TIFR0					// Timer/Counter Interrupt Flag Register

	#if tsconfigRX & tsconfigOCR_A

		#define	tsRX_SB_IR	TIMER0_COMPA_vect	// RX Stop Bit Interrupt Routine Vector
		#define	tsRXOCR		OCR0A				// Output Compare Register
		#define	tsRXOCF		OCF0A				// Output Compare Flag
		#define	tsRXOCIE	OCIE0A				// Timer/Counter Output Compare Match Interrupt Enable

		#define tsVAR_RX_PREFIX_A	t0a_		// Prefix to add to variable names

	#elif tsconfigRX & tsconfigOCR_B

		#define	tsRX_SB_IR	TIMER0_COMPB_vect	// RX Stop Bit Interrupt Routine Vector
		#define	tsRXOCR		OCR0B				// Output Compare Register
		#define	tsRXOCF		OCF0B				// Output Compare Flag
		#define	tsRXOCIE	OCIE0B				// Timer/Counter Output Compare Match Interrupt Enable

		#define tsVAR_RX_PREFIX_B	t0b_		// Prefix to add to variable names

	#endif

#elif tsconfigRX & tsconfigTIMER1

	#define	tsRXTIFR		TIFR1				// Timer/Counter Interrupt Flag Register
	#define	tsRXEICRAICP	TCCR1B				// Timer/Counter Control Register B
	#define tsRXEIFRICP		TIFR1				// Timer Counter Interrupt Flag Register
	#define tsRXEIMSKICP	TIMSK1				// Timer/Counter Interrupt Mask Register
	#define	tsRXICESICP_1	ICES1				// Input Capture Edge Select
	#define tsRXINTFICP_1	ICF1				// Input Capture Flag
	#define tsRXINTICP_1	ICIE1				// Input Capture Interrupt Enable

	#if tsconfigRX & tsconfigICP1
	
		#define	tsRX_IR		TIMER1_CAPT_vect	// RX Interrupt Routine Vector
		#define	tsRXTCNT	ICR1L				// Input Capture Register Low Byte
		#define	tsRXTCNTH	ICR1H				// Input Capture Register High Byte

		#define tsREG_ICP_SUFFIX	_1
	
	#else
	
		#define	tsRXTCNT	TCNT1L				// Timer/Counter Low Byte
		#define	tsRXTCNTH	TCNT1H				// Timer/Counter High Byte

	#endif

	#if tsconfigRX & tsconfigOCR_A

		#define	tsRX_SB_IR	TIMER1_COMPA_vect	// RX Stop Bit Interrupt Routine Vector
		#define	tsRXOCR		OCR1AL				// Output Compare Register Low Byte
		#define	tsRXOCRH	OCR1AH				// Output Compare Register High Byte
		#define	tsRXOCF		OCF1A				// Output Compare Flag
		#define	tsRXOCIE	OCIE1A				// Timer/Counter Output Compare Match Interrupt Enable

		#define tsVAR_RX_PREFIX_A	t1a_		// Prefix to add to variable names

	#elif tsconfigRX & tsconfigOCR_B

		#define	tsRX_SB_IR	TIMER1_COMPB_vect	// RX Stop Bit Interrupt Routine Vector
		#define	tsRXOCR		OCR1BL				// Output Compare Register Low Byte
		#define	tsRXOCRH	OCR1BH				// Output Compare Register High Byte
		#define	tsRXOCF		OCF1B				// Output Compare Flag
		#define	tsRXOCIE	OCIE1B				// Timer/Counter Output Compare Match Interrupt Enable

		#define tsVAR_RX_PREFIX_B	t1b_		// Prefix to add to variable names

	#endif
	
#elif tsconfigRX & tsconfigTIMER2

	#define	tsRXTCNT	TCNT2					// Timer/Counter
	#define	tsRXTIFR	TIFR2					// Timer/Counter Interrupt Flag Register

	#if tsconfigRX & tsconfigOCR_A

		#define	tsRX_SB_IR	TIMER2_COMPA_vect	// RX Stop Bit Interrupt Routine Vector
		#define	tsRXOCR		OCR2A				// Output Compare Register
		#define	tsRXOCF		OCF2A				// Output Compare Flag
		#define	tsRXOCIE	OCIE2A				// Timer/Counter Output Compare Match Interrupt Enable

		#define tsVAR_RX_PREFIX_A	t2a_		// Prefix to add to variable names

	#elif tsconfigRX & tsconfigOCR_B

		#define	tsRX_SB_IR	TIMER2_COMPB_vect	// RX Stop Bit Interrupt Routine Vector
		#define	tsRXOCR		OCR2B				// Output Compare Register
		#define	tsRXOCF		OCF2B				// Output Compare Flag
		#define	tsRXOCIE	OCIE2B				// Timer/Counter Output Compare Match Interrupt Enable

		#define tsVAR_RX_PREFIX_B	t2b_		// Prefix to add to variable names

	#endif

#elif tsconfigRX & tsconfigTIMER3

	#define	tsRXTIFR		TIFR3				// Timer/Counter Interrupt Flag Register
	#define	tsRXEICRAICP	TCCR3B				// Timer/Counter Control Register B
	#define tsRXEIFRICP		TIFR3				// Timer Counter Interrupt Flag Register
	#define tsRXEIMSKICP	TIMSK3				// Timer/Counter Interrupt Mask Register
	#define	tsRXICESICP_1	ICES3				// Input Capture Edge Select
	#define tsRXINTFICP_1	ICF3				// Input Capture Flag
	#define tsRXINTICP_1	ICIE3				// Input Capture Interrupt Enable

	#if tsconfigRX & tsconfigICP3
	
		#define	tsRX_IR		TIMER3_CAPT_vect	// RX Interrupt Routine Vector
		#define	tsRXTCNT	ICR3L				// Input Capture Register Low Byte
		#define	tsRXTCNTH	ICR3H				// Input Capture Register High Byte

		#define tsREG_ICP_SUFFIX	_3
	
	#else
	
		#define	tsRXTCNT	TCNT3L				// Timer/Counter Low Byte
		#define	tsRXTCNTH	TCNT3H				// Timer/Counter High Byte

	#endif

	#if tsconfigRX & tsconfigOCR_A

		#define	tsRX_SB_IR	TIMER3_COMPA_vect	// RX Stop Bit Interrupt Routine Vector
		#define	tsRXOCR		OCR3AL				// Output Compare Register Low Byte
		#define	tsRXOCRH	OCR3AH				// Output Compare Register High Byte
		#define	tsRXOCF		OCF3A				// Output Compare Flag
		#define	tsRXOCIE	OCIE3A				// Timer/Counter Output Compare Match Interrupt Enable

		#define tsVAR_RX_PREFIX_A	t3a_		// Prefix to add to variable names

	#elif tsconfigRX & tsconfigOCR_B

		#define	tsRX_SB_IR	TIMER3_COMPB_vect	// RX Stop Bit Interrupt Routine Vector
		#define	tsRXOCR		OCR3BL				// Output Compare Register Low Byte
		#define	tsRXOCRH	OCR3BH				// Output Compare Register High Byte
		#define	tsRXOCF		OCF3B				// Output Compare Flag
		#define	tsRXOCIE	OCIE3B				// Timer/Counter Output Compare Match Interrupt Enable

		#define tsVAR_RX_PREFIX_B	t3b_		// Prefix to add to variable names

	#endif
	
#endif

#if tsconfigRX & tsconfigINT_MASK

	#define	tsRXEICRA	EICRA					// External Interrupt Control Register A
	#define tsRXEIMSK	EIMSK					// External Interrupt Mask Register
	#define tsRXEIFR	EIFR					// External Interrupt Flag Register
	#define	tsRXISC0_0	ISC00					// Interrupt Sense Control Bit 0
	#define	tsRXISC1_0	ISC01					// Interrupt Sense Control Bit 1
	#define tsRXINT_0	INT0					// External Interrupt Request Enable
	#define tsRXINTF_0	INTF0					// External Interrupt Flag
	#define	tsRXISC0_1	ISC10					// Interrupt Sense Control Bit 0
	#define	tsRXISC1_1	ISC11					// Interrupt Sense Control Bit 1
	#define tsRXINT_1	INT1					// External Interrupt Request Enable
	#define tsRXINTF_1	INTF1					// External Interrupt Flag
	#define	tsRXISC0_2	ISC20					// Interrupt Sense Control Bit 0
	#define	tsRXISC1_2	ISC21					// Interrupt Sense Control Bit 1
	#define tsRXINT_2	INT2					// External Interrupt Request Enable
	#define tsRXINTF_2	INTF2					// External Interrupt Flag

	#if tsconfigRX & tsconfigINT0

		#define	tsRX_IR				INT0_vect	// RX Interrupt Routine Vector
		#define tsREG_INT_SUFFIX	_0

	#elif tsconfigRX & tsconfigINT1

		#define	tsRX_IR				INT1_vect	// RX Interrupt Routine Vector
		#define tsREG_INT_SUFFIX	_1

	#elif tsconfigRX & tsconfigINT2

		#define	tsRX_IR				INT2_vect	// RX Interrupt Routine Vector
		#define tsREG_INT_SUFFIX	_2
	#endif

#endif

#if tsconfigRX & tsconfigPCPORT_MASK

	#define tsRXEIMSK	PCICR					// Pin Change Interrupt Control Register
	#define tsRXEIFR	PCIFR					// Pin Change Interrupt Flag Register

	#if tsPORTCOUNT == 3

		#if tsconfigRX & tsconfigPCPORTB

			#define	tsRX_IR		PCINT0_vect		// RX Interrupt Routine Vector
			#define tsRXPCMSK	PCMSK0			// Pin Change Mask Register
			#define tsRXINT		PCIE0			// Pin Change Interrupt Enable
			#define tsRXINTF	PCIF0			// Pin Change Interrupt Flag

		#elif tsconfigRX & tsconfigPCPORTC

			#define	tsRX_IR		PCINT1_vect		// RX Interrupt Routine Vector
			#define tsRXPCMSK	PCMSK1			// Pin Change Mask Register
			#define tsRXINT		PCIE1			// Pin Change Interrupt Enable
			#define tsRXINTF	PCIF1			// Pin Change Interrupt Flag

		#elif tsconfigRX & tsconfigPCPORTD

			#define	tsRX_IR		PCINT2_vect		// RX Interrupt Routine Vector
			#define tsRXPCMSK	PCMSK2			// Pin Change Mask Register
			#define tsRXINT		PCIE2			// Pin Change Interrupt Enable
			#define tsRXINTF	PCIF2			// Pin Change Interrupt Flag

		#endif

	#elif tsPORTCOUNT == 4

		#if tsconfigRX & tsconfigPCPORTA

			#define	tsRX_IR		PCINT0_vect		// RX Interrupt Routine Vector
			#define tsRXPCMSK	PCMSK0			// Pin Change Mask Register
			#define tsRXINT		PCIE0			// Pin Change Interrupt Enable
			#define tsRXINTF	PCIF0			// Pin Change Interrupt Flag

		#elif tsconfigRX & tsconfigPCPORTB

			#define	tsRX_IR		PCINT1_vect		// RX Interrupt Routine Vector
			#define tsRXPCMSK	PCMSK1			// Pin Change Mask Register
			#define tsRXINT		PCIE1			// Pin Change Interrupt Enable
			#define tsRXINTF	PCIF1			// Pin Change Interrupt Flag

		#elif tsconfigRX & tsconfigPCPORTC

			#define	tsRX_IR		PCINT2_vect		// RX Interrupt Routine Vector
			#define tsRXPCMSK	PCMSK2			// Pin Change Mask Register
			#define tsRXINT		PCIE2			// Pin Change Interrupt Enable
			#define tsRXINTF	PCIF2			// Pin Change Interrupt Flag

		#elif tsconfigRX & tsconfigPCPORTD

			#define	tsRX_IR		PCINT3_vect		// RX Interrupt Routine Vector
			#define tsRXPCMSK	PCMSK3			// Pin Change Mask Register
			#define tsRXINT		PCIE3			// Pin Change Interrupt Enable
			#define tsRXINTF	PCIF3			// Pin Change Interrupt Flag

		#endif

	#endif
	
#endif

//#endif /* TSERIAL_TIMERS_H_ */
