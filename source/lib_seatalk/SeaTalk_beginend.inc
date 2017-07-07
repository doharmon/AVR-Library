;---------------------------------------------------------------------------;
; Timer based SeaTalk software serial ports
;
; Code and variables to configure timer as a serial port
;
; doharmon 2015
;---------------------------------------------------------------------------;
;
; Data types:	char is 8 bits, 
;				int is 16 bits, 
;				long is 32 bits, 
;				long long is 64 bits, 
;				float and double are 32 bits
;				pointers are 16 bits (function pointers are word addresses,
;									  to allow addressing up to 128K program
;									  memory space). 
;
; Call-used registers (r18-r27, r30-r31): Can be clobbered
;	May be allocated by gcc for local data. You may use them freely in
;	assembler subroutines. Calling C subroutines can clobber any of them - 
;	the caller is responsible for saving and restoring.
;
; Call-saved registers (r2-r17, r28-r29): Not clobbered
;	May be allocated by gcc for local data. Calling C subroutines leaves them
;	unchanged. Assembler subroutines are responsible for saving and restoring 
;	these registers, if changed. r29:r28 (Y pointer) is used as a frame
;	pointer (points to local data on stack) if necessary. The requirement for
;	the callee to save/preserve the contents of these registers even applies 
;	in situations where the compiler assigns them for argument passing.
;
; Fixed registers (r0, r1): 
;	Never allocated by gcc for local data, but often used for fixed purposes: 
;
;	r0 -	temporary register, can be clobbered by any C code 
;			(except interrupt handlers which save it), may be used to 
;			remember something for a while within one piece of assembler code
;
;	r1 -	assumed to be always zero in any C code, may be used to remember 
;			something for a while within one piece of assembler code, but 
;			must then be cleared after use (clr r1). This includes any use of
;			the [f]mul[s[u]] instructions, which return their result in r1:r0
;			Interrupt handlers save and clear r1 on entry, and restore r1 on
;			exit (in case it was non-zero).
;
; Function call conventions: 
;	Arguments - allocated left to right, r25 to r8. All arguments are aligned 
;	to start in even-numbered registers (odd-sized arguments, including char,
;	have one free register above them). This allows making better use of the
;	movw instruction on the enhanced core. 
;
;	If too many, those that don't fit are passed on the stack.
;
; Return values:	8-bit in r24 (not r25!)
;					16-bit in r25:r24
;					up to 32 bits in r22-r25
;					up to 64 bits in r18-r25. 
;
;	8-bit return values are zero/sign-extended to 16 bits by the called 
;	function (unsigned char is more efficient than signed char - just clr 
;	r25). Arguments to functions with variable argument lists (printf etc.)
;	are all passed on stack, and char is extended to int. 
;
;	X:	r27:r26
;	Y:	r29:r28
;	Z:	r31:r30
;
;---------------------------------------------------------------------------;

.section stsBSS_SECTIONNAME
stsBSS_START = .
.global stsBSS_START

#define ticks_per_bit			stsMAKE_VARNAME(stsVAR_TIMER_PREFIX,_ticks_per_bit)
#define ticks_per_bit_and_half	stsMAKE_VARNAME(stsVAR_TIMER_PREFIX,_ticks_per_bit_and_half)
#define ticks_per_half_bit		stsMAKE_VARNAME(stsVAR_TIMER_PREFIX,_ticks_per_half_bit)
#define byte_delay				stsMAKE_VARNAME(stsVAR_TIMER_PREFIX,_byte_delay)
#define min_lead				stsMAKE_VARNAME(stsVAR_TIMER_PREFIX,_min_lead)
#define delay_per_bit			stsMAKE_VARNAME(stsVAR_TIMER_PREFIX,_delay_per_bit)
#define rx_stop_ticks			stsMAKE_VARNAME(stsVAR_TIMER_PREFIX,_rx_stop_ticks)

.global ticks_per_bit
.global ticks_per_bit_and_half
.global ticks_per_half_bit
.global byte_delay
.global min_lead
.global delay_per_bit
.global rx_stop_ticks

#if stsBITS == 8

; Timer is 8 bits. The maximum ticks it can wait is 255.
; The number of ticks needed to wait for a stop bit is 10.25 times
; the number of ticks per bit. Therefore the maximum ticks per bit
; is 255/10.25 = 24.878. which is rounded to 25.
#define MAX_TICKS_PER_BIT	25

#elif stsBITS == 16

; Timer is 16 bits. The maximum ticks it can wait is 65535.
; The number of ticks needed to wait for a stop bit is 10.25 times
; the number of ticks per bit. Therefore the maximum ticks per bit
; is 65535/10.25 = 6393.659 which is rounded to 6394.
#define MAX_TICKS_PER_BIT 6394

#endif

#if defined(stsBAUDRATE)

;-----------------------------------------------------------------------------
; Baudrate is hardcoded therefore use constants for some values
;-----------------------------------------------------------------------------

#define cycles_per_bit ((F_CPU + stsBAUDRATE/2) / stsBAUDRATE)

delay_per_bit = (cycles_per_bit / 4)			//!< Delay loop takes 4 cycles

	#if cycles_per_bit < MAX_TICKS_PER_BIT
		#define stsPRESCALE    stsNO_PRESCALE
		ticks_per_bit = ((F_CPU + stsBAUDRATE/2) / stsBAUDRATE)
	#elif cycles_per_bit / 8 < MAX_TICKS_PER_BIT
		#define stsPRESCALE    stsPRESCALE_8
		ticks_per_bit = ((F_CPU + stsBAUDRATE/2) / stsBAUDRATE / 8)
	#elif (cycles_per_bit / 32 < MAX_TICKS_PER_BIT) && defined(stsPRESCALE_32)
		#define stsPRESCALE    stsPRESCALE_32
		ticks_per_bit = ((F_CPU + stsBAUDRATE/2) / stsBAUDRATE / 32)
	#elif cycles_per_bit / 64 < MAX_TICKS_PER_BIT
		#define stsPRESCALE    stsPRESCALE_64
		ticks_per_bit = ((F_CPU + stsBAUDRATE/2) / stsBAUDRATE / 64)
	#elif (cycles_per_bit / 128 < MAX_TICKS_PER_BIT) && defined(stsPRESCALE_128)
		#define stsPRESCALE    stsPRESCALE_128
		ticks_per_bit = ((F_CPU + stsBAUDRATE/2) / stsBAUDRATE / 128)
	#elif cycles_per_bit / 256 < MAX_TICKS_PER_BIT
		#define stsPRESCALE    stsPRESCALE_256
		ticks_per_bit = ((F_CPU + stsBAUDRATE/2) / stsBAUDRATE / 256)
	#elif cycles_per_bit / 1024 < MAX_TICKS_PER_BIT
		#define stsPRESCALE    stsPRESCALE_1024
		ticks_per_bit = ((F_CPU + stsBAUDRATE/2) / stsBAUDRATE / 1024)
	#else // stsBAUDRATE
		#error "Unsupported baud rate"
	#endif // stsBAUDRATE

ticks_per_bit_and_half	= (ticks_per_bit * 3 / 2)		//!< 3/2  = 1.5
.if stsBITS == 8
ticks_per_half_bit		= (255 - ticks_per_bit/2 - 3)	//!< 8 bit one's complement (-3 good @ 14400 and lower for receive)
.else
ticks_per_half_bit		= (65535 - ticks_per_bit/2 - 3)	//!< 16 bit one's complement
.endif
;min_lead				= (ticks_per_bit * 4 / 25)		//!< 4/25 = 0.16  Minimum between timer counter and compare
min_lead				= (ticks_per_bit * 2 / 25)		//!< 4/25 = 0.16  Minimum between timer counter and compare
rx_stop_ticks			= (ticks_per_bit * 41 / 4)		//!< 41/4 = 10.25

#else // defined(stsBAUDRATE)

;-----------------------------------------------------------------------------
; Baudrate is unknown at compile therefore allocate space for variables
;-----------------------------------------------------------------------------

; These variables can be 8 or 16 bits depending on the timer
#if defined(stconfigTX) | defined(stconfigRX)
ticks_per_bit:			stsDS	1	; Used by TX and RX
#endif	// defined(stconfigTX) | defined(stconfigRX)
#ifdef stconfigTX
min_lead:				stsDS	1	; Used for TX frame error and write byte
delay_per_bit:			.ds.w	1	; Used to delay when TX buffer is full
#endif	// stconfigTX

#ifdef stconfigRX

ticks_per_bit_and_half:	stsDS	1	; Used by RX
ticks_per_half_bit:		stsDS	1	; Used by RX

#if stconfigRX & stconfigOCR_MASK
rx_stop_ticks:			stsDS	1	; Only used by RX Stop Bit IR
#endif

#endif	// stconfigRX

#endif // defined(stsBAUDRATE)

;-----------------------------------------------------------------------------
; int XSerialBegin(uint32_t baud)
;
; This function is renamed according to the timer and output compare register
; it is associated with.
;
; Initialize the timer and variables.
;
; Arguments:
;		r25:r22	baud
;
; Local Variables:
;		r20		Prescale config and temp
;		r25:r22	cycles_per_bit = (F_CPU + baud/2) / baud
;		r27:r26	max_ticks_per_bit
;
; Returns:
;		r25:r24	Success: 0; Error: -1 (baud rate too low)
;-----------------------------------------------------------------------------
#define stsSERIALBEGIN stsMAKE_FUNCNAME(stsFUNC_PREFIX,SerialBegin)
#define udiv2 stsMAKE_FUNCNAME(stsFUNC_PREFIX,udiv2)
#define error stsMAKE_FUNCNAME(stsFUNC_PREFIX,error)
#define exit stsMAKE_FUNCNAME(stsFUNC_PREFIX,exit)
.section stsMAKE_SECTIONNAME(stsMAKE_SECTIONNAME(stsTEXT_SECTIONNAME,.),stsSERIALBEGIN)

.global	stsSERIALBEGIN
.func	stsSERIALBEGIN

#if !defined(stsBAUDRATE)
udiv2:
	lsr		r25
	ror		r24
	ror		r23
	ror		r22
	lsr		r0
	brne	udiv2
	ret
#endif

stsSERIALBEGIN:
	; ------------------------------------------------------------------------
	; Zero BSS section for this timer
	ldi		r30, lo8(stsBSS_START)
	ldi		r31, hi8(stsBSS_START)
	ldi		r26, lo8(stsBSS_END)
	ldi		r27, hi8(stsBSS_END)
	subi	r26, 0xFF
	sbci	r27, 0xFF
00:	st		Z+, r1
	cp		r30, r26
	cpc		r31, r27
	brne	00b

#if !defined(stsBAUDRATE)

	; ------------------------------------------------------------------------
	; Calculate cycles_per_bit
	ldi		r18, lo8(300)			; Check that baud no lower than 300
	ldi		r19, hi8(300)
	ldi		r20, hlo8(300)
	ldi		r21, hhi8(300)
	cp		r22, r18
	cpc		r23, r19
	cpc		r24, r20
	cpc		r25, r21
	brsh	00f
	rjmp	error
00:	X_movw	r20, r24				; Move baud to divisor
	X_movw	r18, r22
	ldi		r22, lo8(F_CPU)			; F_CPU is dividend
	ldi		r23, hi8(F_CPU)
	ldi		r24, hlo8(F_CPU)
	ldi		r25, hhi8(F_CPU)
	call	urdiv32					; (F_CPU + baud/2) / baud
	X_movw	r26, r22
	#ifdef stconfigTX
	mov		r18, r24				; Needed for F_CPU >= 20M and baud ~300
	lsr		r18						; cycles_per_bit / 4
	ror		r27
	ror		r26						; Delay loop takes 4 cycles
	lsr		r18
	ror		r27
	ror		r26
	sts		delay_per_bit, r26		; delay_per_bit = ticks_per_bit / 4
	sts		delay_per_bit+1, r27
	#endif	// stconfigTX

	ldi		r26, lo8(MAX_TICKS_PER_BIT)
	ldi		r27, hi8(MAX_TICKS_PER_BIT)

	; ------------------------------------------------------------------------
	; Calculate ticks_per_bit (cycles_per_bit) and timer prescale
	cp		r22, r26				; Is ticks_per_bit < MAX_TICKS_PER_BIT?
	cpc		r23, r27
	cpc		r24, r1
;	cpc		r25, r1
	brsh	01f
	ldi		r20, stsNO_PRESCALE
	rjmp	8f

01:	set								; ticks_per_bit /= 8
	bld		r0, 2
	rcall	udiv2
	cp		r22, r26
	cpc		r23, r27
	cpc		r24, r1
;	cpc		r25, r1
	brsh	02f
	ldi		r20, stsPRESCALE_8
	rjmp	8f

02:
	#ifdef stsPRESCALE_32
	bld		r0, 1					; ticks_per_bit /= 4
	rcall	udiv2
	cp		r22, r26
	cpc		r23, r27
	cpc		r24, r1
;	cpc		r25, r1
	brsh	03f
	ldi		r20, stsPRESCALE_32
	rjmp	8f
	#endif

03:	
	#ifdef stsPRESCALE_32
	rcall	udiv2					; ticks_per_bit /= 2
	#else
	bld		r0, 2
	rcall	udiv2					; ticks_per_bit /= 8
	#endif
	cp		r22, r26
	cpc		r23, r27
	cpc		r24, r1
;	cpc		r25, r1
	brsh	04f
	ldi		r20, stsPRESCALE_64
	rjmp	8f

04:	
	#ifdef stsPRESCALE_128
	rcall	udiv2					; ticks_per_bit /= 2
	cp		r22, r26
	cpc		r23, r27
	cpc		r24, r1
;	cpc		r25, r1
	brsh	05f
	ldi		r20, stsPRESCALE_128
	rjmp	8f
	#endif

05:	
	#ifdef stsPRESCALE_128
	rcall	udiv2					; ticks_per_bit /= 2
	#else
	bld		r0, 1
	rcall	udiv2					; ticks_per_bit /= 4
	#endif
	cp		r22, r26
	cpc		r23, r27
	cpc		r24, r1
;	cpc		r25, r1
	brsh	06f
	ldi		r20, stsPRESCALE_256
	rjmp	8f

06:	bld		r0, 1
	rcall	udiv2					; ticks_per_bit /= 4
	cp		r22, r26
	cpc		r23, r27
	cpc		r24, r1
;	cpc		r25, r1
	brlo	07f
	rjmp	error					; Error: Baud rate too slow
07:	ldi		r20, stsPRESCALE_1024

#else // !defined(stsBAUDRATE)

	ldi		r20, stsPRESCALE

#endif // !defined(stsBAUDRATE)

08:
	#ifdef stconfigTX
;#warning "Is this needed?"
;	X_out	stsTXTCCRA, r1
	#endif
	X_out	stsTIMSK, r1
	X_out	stsTCCRB, r20

#if !defined(stsBAUDRATE)

	X_stsw	ticks_per_bit, r22, stsBITS

	; ------------------------------------------------------------------------
	; Calculate other constants

	#ifdef stconfigRX

	; ticks_per_bit_and_half = ticks_per_bit * 3 / 2
	X_movw	r18, r22, stsBITS
	X_addw	r18, r18, stsBITS
	X_addw	r18, r22, stsBITS
	X_movw	r26, r18, stsBITS		; Save (ticks_per_bit * 3)
	X_lsrw	r18, stsBITS
	X_stsw	ticks_per_bit_and_half, r18, stsBITS

	; 8/16 bit one's complement (-3 good @ 14400 and lower for receive)
	; ticks_per_half_bit = 255 - ticks_per_bit/2 - 3
	; ticks_per_half_bit = 65535 - ticks_per_bit/2 - 3
	X_movw	r18, r22, stsBITS
	X_lsrw	r18, stsBITS
	ldi		r24, 0xFC				; 252
	.if stsBITS == 16
	ldi		r25, 0xFF				; r5:r4 = 65532
	.endif
	X_subw	r24, r18, stsBITS
	X_stsw	ticks_per_half_bit, r24, stsBITS

	#endif	// stconfigRX

	#ifdef stconfigTX
	; min_lead = (ticks_per_bit*5/32) Minimum between timer counter and compare
	; 5/32 = 0.15625 (Previous value used was: 4/25 = 0.16)
	X_addw	r26, r22, stsBITS		; (ticks_per_bit*3) was saved in r26
	X_addw	r26, r22, stsBITS
	X_lsrw	r26, stsBITS
	X_lsrw	r26, stsBITS
	X_lsrw	r26, stsBITS
	X_lsrw	r26, stsBITS
	X_lsrw	r26, stsBITS
	X_cpzw	r26, stsBITS
	brne	9f
	ldi		r26, 1					; Use a minimum lead time of 1
09:	X_stsw	min_lead, r26, stsBITS
	#endif	// stconfigTX

	#if stconfigRX & stconfigOCR_MASK
	; rx_stop_ticks = ticks_per_bit * 41 / 4  (41/4 = 10.25)
	ldi		r26, 41
	.if stsBITS == 8
	mul		r26, r22
	lsr		r1
	ror		r0
	lsr		r1
	ror		r0
	sts		rx_stop_ticks, r0
	clr		r1
	.else
	mul		r22, r26
	X_movw	r18, r0
	mul		r23, r26
	mov		r20, r1
	add		r19, r0
	clr		r1
	adc		r20, r1
	lsr		r20
	ror		r19
	ror		r18
	lsr		r20
	ror		r19
	ror		r18
	sts		rx_stop_ticks+1, r19
	sts		rx_stop_ticks, r18
	.endif
	#endif	// stconfigRX & stconfigOCR_MASK

#endif // !defined(stsBAUDRATE)

	; ------------------------------------------------------------------------
	; Configure RX/TX pins

	; External interrupt pins
	#if (stconfigRX & stconfigINT_MASK)

	#if (stconfigRX & stconfigINT0)
	X_cbi	stsRXDDR_0, stsRXPIN_0, r20	; Configure as input
	X_sbi	stsRXPORT_0, stsRXPIN_0, r20; Enable pullup
	X_in	r20, stsRXEICRA
	andi	r20, ~(1<<stsRXISC0_0)
	ori		r20, 1<<stsRXISC1_0			; Capture falling edge
	X_out	stsRXEICRA, r20
	X_in	r20, stsRXEIFR
	ori		r20, 1<<stsRXINTF_0
	X_out	stsRXEIFR, r20				; Clear interrupt flag
	X_in	r20, stsRXEIMSK
	ori		r20, 1<<stsRXINT_0
	X_out	stsRXEIMSK, r20				; Enable interrupt
	#endif

	#if (stconfigRX & stconfigINT1)
	X_cbi	stsRXDDR_1, stsRXPIN_1, r20	; Configure as input
	X_sbi	stsRXPORT_1, stsRXPIN_1, r20; Enable pullup
	X_in	r20, stsRXEICRA
	andi	r20, ~(1<<stsRXISC0_1)
	ori		r20, 1<<stsRXISC1_1			; Capture falling edge
	X_out	stsRXEICRA, r20
	X_in	r20, stsRXEIFR
	ori		r20, 1<<stsRXINTF_1
	X_out	stsRXEIFR, r20				; Clear interrupt flag
	X_in	r20, stsRXEIMSK
	ori		r20, 1<<stsRXINT_1
	X_out	stsRXEIMSK, r20				; Enable interrupt
	#endif

	#if (stconfigRX & stconfigINT2)
	X_cbi	stsRXDDR_2, stsRXPIN_2, r20	; Configure as input
	X_sbi	stsRXPORT_2, stsRXPIN_2, r20; Enable pullup
	#endif

	#if (stconfigRX & stconfigINT3)
	X_cbi	stsRXDDR_3, stsRXPIN_3, r20	; Configure as input
	X_sbi	stsRXPORT_3, stsRXPIN_3, r20; Enable pullup
	#endif

	#if (stconfigRX & stconfigINT4)
	X_cbi	stsRXDDR_4, stsRXPIN_4, r20	; Configure as input
	X_sbi	stsRXPORT_4, stsRXPIN_4, r20; Enable pullup
	#endif

	#if (stconfigRX & stconfigINT5)
	X_cbi	stsRXDDR_5, stsRXPIN_5, r20	; Configure as input
	X_sbi	stsRXPORT_5, stsRXPIN_5, r20; Enable pullup
	#endif

	#if (stconfigRX & stconfigINT6)
	X_cbi	stsRXDDR_6, stsRXPIN_6, r20	; Configure as input
	X_sbi	stsRXPORT_6, stsRXPIN_6, r20; Enable pullup
	#endif

	#if (stconfigRX & stconfigINT7)
	X_cbi	stsRXDDR_7, stsRXPIN_7, r20	; Configure as input
	X_sbi	stsRXPORT_7, stsRXPIN_7, r20; Enable pullup
	#endif

	#endif // (stconfigRX & stconfigINT_MASK)

	; Input capture pins
	#if (stconfigRX & stconfigICP_MASK)

	#if (stconfigRX & stconfigICP0)
	X_cbi	stsRXDDRICP_0, stsRXPINICP_0, r20	; Configure as input
	X_sbi	stsRXPORTICP_0, stsRXPINICP_0, r20	; Enable pullup
	X_in	r20, stsRXEICRAICP
	andi	r20, ~(1<<stsRXICESICP_0)
	X_out	stsRXEICRAICP, r20					; Capture falling edge
	X_in	r20, stsRXEIFRICP
	ori		r20, 1<<stsRXINTFICP_0
	X_out	stsRXEIFRICP, r20					; Clear interrupt flag
	X_in	r20, stsRXEIMSKICP
	ori		r20, 1<<stsRXINTICP_0
	X_out	stsRXEIMSKICP, r20					; Enable interrupt
	#endif

	#if (stconfigRX & stconfigICP1)
	X_cbi	stsRXDDRICP_1, stsRXPINICP_1, r20	; Configure as input
	X_sbi	stsRXPORTICP_1, stsRXPINICP_1, r20	; Enable pullup
	X_in	r20, stsRXEICRAICP
	andi	r20, ~(1<<stsRXICESICP_1)
	X_out	stsRXEICRAICP, r20					; Capture falling edge
	X_in	r20, stsRXEIFRICP
	ori		r20, 1<<stsRXINTFICP_1
	X_out	stsRXEIFRICP, r20					; Clear interrupt flag
	X_in	r20, stsRXEIMSKICP
	ori		r20, 1<<stsRXINTICP_1
	X_out	stsRXEIMSKICP, r20					; Enable interrupt
	#endif

	#if (stconfigRX & stconfigICP2)
	#endif

	#if (stconfigRX & stconfigICP3)
	#endif

	#if (stconfigRX & stconfigICP4)
	#endif

	#if (stconfigRX & stconfigICP5)
	#endif

	#endif // (stconfigRX & stconfigICP_MASK)

	#if (stconfigRX & stconfigPCPORT_MASK)

	; Configure receive pins as input pullup
	X_in	r20, stsRXDDR			; Configure as input
	andi	r20, ~(stconfigRX_PINS)
	X_out	stsRXDDR, r20
	X_in	r20, stsRXPORT			; Enable pullup
	ori		r20, stconfigRX_PINS
	X_out	stsRXPORT, r20

;	ldi		r20, 0xFF				; Set previous pin states
;	sts		rx_prev, r20

	ldi		r20, stconfigRX_PINS	; Set mask for pins
	X_out	stsRXPCMSK, r20

	X_in	r20, stsRXEIFR			; Clear interrupt flag
	ori		r20, 1<<stsRXINTF
	X_out	stsRXEIFR, r20
	X_in	r20, stsRXEIMSK			; Enable interrupt
	ori		r20, 1<<stsRXINT
	X_out	stsRXEIMSK, r20

	#endif
	
	#ifdef stconfigTX

	; Configure transmit pin as high output
	#if stconfigTX & stconfigOCR_A
	X_sbi	stsTXPORTA, stsTXPINA, r20	; Set pin high
	X_sbi	stsTXDDRA, stsTXPINA, r20	; Configure as output
	#endif
	#if stconfigTX & stconfigOCR_B
	X_sbi	stsTXPORTB, stsTXPINB, r20	; Set pin high
	X_sbi	stsTXDDRB, stsTXPINB, r20	; Configure as output
	#endif
	#if stconfigTX & stconfigOCR_C
	X_sbi	stsTXPORTC, stsTXPINC, r20	; Set pin high
	X_sbi	stsTXDDRC, stsTXPINC, r20	; Configure as output
	#endif

	#endif	// stconfigTX

	clr		r24							; Return success
	clr		r25
#if !defined(stsBAUDRATE)
	rjmp	exit

error:
	ldi		r24, 0xFF					; Return error
	ldi		r25, 0xFF

#endif // !defined(stsBAUDRATE)

exit:
	ret

.endfunc	; stsSERIALBEGIN

;-----------------------------------------------------------------------------
; void XSerialEnd(void)
;
; This function is renamed according to the timer and output compare register
; it is associated with.
;
; Terminate the timer.
;
; Local Variables:
;		r20		Prescale config and temp
;		r25:r22	cycles_per_bit = (F_CPU + baud/2) / baud
;		r27:r26	max_ticks_per_bit
;-----------------------------------------------------------------------------
#define stsSERIALEND stsMAKE_FUNCNAME(stsFUNC_PREFIX,SerialEnd)
.section stsMAKE_SECTIONNAME(stsMAKE_SECTIONNAME(stsTEXT_SECTIONNAME,.),stsSERIALEND)

.global	stsSERIALEND
.func	stsSERIALEND

stsSERIALEND:
	; Input capture pins
	#if (stconfigRX & stconfigICP_MASK)

	X_in	r20, stsRXEIMSKICP

	#if (stconfigRX & stconfigICP0)
	andi	r20, ~(1<<stsRXINTICP_0)
	#endif

	#if (stconfigRX & stconfigICP1)
	andi	r20, ~(1<<stsRXINTICP_1)
	#endif

	#if (stconfigRX & stconfigICP2)
	#endif

	#if (stconfigRX & stconfigICP3)
	#endif

	#if (stconfigRX & stconfigICP4)
	#endif

	#if (stconfigRX & stconfigICP5)
	#endif

	X_out	stsRXEIMSKICP, r20					; Disable interrupt

	#endif // (stconfigRX & stconfigICP_MASK)

	#ifdef stconfigTX

	#if stconfigTX & stconfigOCR_A
;	#warning "ToDo Need a method to disable TX interrupt"
;	X_in	r18, stsTIMSK
;	ori		r18, 1<<stsTXOCIE
;	X_out	stsTIMSK, r18
	#endif
	#if stconfigTX & stconfigOCR_B
;	#warning "ToDo Need a method to disable TX interrupt"
;	X_in	r18, stsTIMSK
;	ori		r18, 1<<stsTXOCIE
;	X_out	stsTIMSK, r18
	#endif
	#if stconfigTX & stconfigOCR_C
;	#warning "ToDo Need a method to disable TX interrupt"
;	X_in	r18, stsTIMSK
;	ori		r18, 1<<stsTXOCIE
;	X_out	stsTIMSK, r18
	#endif

	#endif	// stconfigTX

	#if (stconfigRX & stconfigPCPORT_MASK)

;	ldi		r20, stconfigRX_PINS
;	X_out	stsRXPCMSK, r20

	#elif defined(stconfigRX)

	; ------------------------------------------------------------------------
	; Configure edge detect for the RX INTX or ICPX Interrupt
;	X_in	r20, stsRXEICRA			; Capture falling edge
;	#ifdef stsRXICES
;	andi	r20, ~(1<<stsRXICES)		; Input Capture Pin
;	#else
;	andi	r20, ~(1<<stsRXISC0)		; External Interrupt Pin
;	ori		r20, 1<<stsRXISC1
;	#endif
;	X_out	stsRXEICRA, r20

	#endif // (stconfigRX & stconfigPCPORT_MASK)

	#ifdef stconfigRX

	; ------------------------------------------------------------------------
	; Enable the RX INTX, ICPX, or Pin Change interrupt
;	X_in	r20, stsRXEIFR			; Clear interrupt flag
;	ori		r20, 1<<stsRXINTF
;	X_out	stsRXEIFR, r20
;	X_in	r20, stsRXEIMSK			; Enable interrupt
;	ori		r20, 1<<stsRXINT
;	X_out	stsRXEIMSK, r20

	#endif	// stconfigRX

	ret

;	#warning "ToDo: Finish SerialEnd routine"

.endfunc	; stsSERIALEND

#undef stconfigTX
#undef stconfigRX
#undef stsFUNC_PREFIX
