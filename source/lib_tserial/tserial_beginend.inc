;---------------------------------------------------------------------------;
; Timer based software serial ports
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

.section tsBSS_SECTIONNAME
tsBSS_START = .
.global tsBSS_START

#define ticks_per_bit			tsMAKE_VARNAME(tsVAR_TIMER_PREFIX,_ticks_per_bit)
#define ticks_per_bit_and_half	tsMAKE_VARNAME(tsVAR_TIMER_PREFIX,_ticks_per_bit_and_half)
#define ticks_per_half_bit		tsMAKE_VARNAME(tsVAR_TIMER_PREFIX,_ticks_per_half_bit)
#define byte_delay				tsMAKE_VARNAME(tsVAR_TIMER_PREFIX,_byte_delay)
#define min_lead				tsMAKE_VARNAME(tsVAR_TIMER_PREFIX,_min_lead)
#define delay_per_bit			tsMAKE_VARNAME(tsVAR_TIMER_PREFIX,_delay_per_bit)
#define rx_stop_ticks			tsMAKE_VARNAME(tsVAR_TIMER_PREFIX,_rx_stop_ticks)

.global ticks_per_bit
.global ticks_per_bit_and_half
.global ticks_per_half_bit
.global byte_delay
.global min_lead
.global delay_per_bit
.global rx_stop_ticks

#if tsBITS == 8

; Timer is 8 bits. The maximum ticks it can wait is 255.
; The number of ticks needed to wait for a stop bit is 9.25 times
; the number of ticks per bit. Therefore the maximum ticks per bit
; is 255/9.25 = 27.567. Find a timer prescale that can count at the
; most this number of ticks per bit.
#define MAX_TICKS_PER_BIT	28

#elif tsBITS == 16

; Timer is 16 bits. The maximum ticks it can wait is 65535.
; The number of ticks needed to wait for a stop bit is 9.25 times
; the number of ticks per bit. Therefore the maximum ticks per bit
; is 65535/9.25 = 7084.864. Find a timer prescale that can count at the
; most this number of ticks per bit.
#define MAX_TICKS_PER_BIT 7085

#endif

#if defined(tsBAUDRATE)

;-----------------------------------------------------------------------------
; Baudrate is hardcoded therefore use constants for some values
;-----------------------------------------------------------------------------

#define cycles_per_bit ((F_CPU + tsBAUDRATE/2) / tsBAUDRATE)

delay_per_bit = (cycles_per_bit / 4)			//!< Delay loop takes 4 cycles

	#if cycles_per_bit < MAX_TICKS_PER_BIT
		#define tsPRESCALE    tsNO_PRESCALE
		ticks_per_bit = ((F_CPU + tsBAUDRATE/2) / tsBAUDRATE)
	#elif cycles_per_bit / 8 < MAX_TICKS_PER_BIT
		#define tsPRESCALE    tsPRESCALE_8
		ticks_per_bit = ((F_CPU + tsBAUDRATE/2) / tsBAUDRATE / 8)
	#elif (cycles_per_bit / 32 < MAX_TICKS_PER_BIT) && defined(tsPRESCALE_32)
		#define tsPRESCALE    tsPRESCALE_32
		ticks_per_bit = ((F_CPU + tsBAUDRATE/2) / tsBAUDRATE / 32)
	#elif cycles_per_bit / 64 < MAX_TICKS_PER_BIT
		#define tsPRESCALE    tsPRESCALE_64
		ticks_per_bit = ((F_CPU + tsBAUDRATE/2) / tsBAUDRATE / 64)
	#elif (cycles_per_bit / 128 < MAX_TICKS_PER_BIT) && defined(tsPRESCALE_128)
		#define tsPRESCALE    tsPRESCALE_128
		ticks_per_bit = ((F_CPU + tsBAUDRATE/2) / tsBAUDRATE / 128)
	#elif cycles_per_bit / 256 < MAX_TICKS_PER_BIT
		#define tsPRESCALE    tsPRESCALE_256
		ticks_per_bit = ((F_CPU + tsBAUDRATE/2) / tsBAUDRATE / 256)
	#elif cycles_per_bit / 1024 < MAX_TICKS_PER_BIT
		#define tsPRESCALE    tsPRESCALE_1024
		ticks_per_bit = ((F_CPU + tsBAUDRATE/2) / tsBAUDRATE / 1024)
	#else // tsBAUDRATE
		#error "Unsupported baud rate"
	#endif // tsBAUDRATE

ticks_per_bit_and_half	= (ticks_per_bit * 3 / 2)		//!< 3/2  = 1.5
ticks_per_half_bit		= (255 - ticks_per_bit/2 - 3)	//!< 8 bit one's complement (-3 good @ 14400 and lower for receive)
min_lead				= (ticks_per_bit * 13 / 64)		//!< 13/64 = 0.203125  Minimum between timer counter and compare
rx_stop_ticks			= (ticks_per_bit * 37 / 4)		//!< 37/4  = 9.25

#else // defined(tsBAUDRATE)

;-----------------------------------------------------------------------------
; Baudrate is unknown at compile therefore allocate space for variables
;-----------------------------------------------------------------------------

; These variables can be 8 or 16 bits depending on the timer
#if defined(tsconfigTX) | defined(tsconfigRX)
ticks_per_bit:			tsDS	1	; Used by TX and RX
#endif	// defined(tsconfigTX) | defined(tsconfigRX)
#ifdef tsconfigTX
min_lead:				tsDS	1	; Used for TX frame error and write byte
delay_per_bit:			.ds.w	1	; Used to delay when TX buffer is full
#endif	// tsconfigTX

#ifdef tsconfigRX

ticks_per_bit_and_half:	tsDS	1	; Used by RX

.if tsBITS == 8
ticks_per_half_bit:		.ds.b	1	; Only used by 8 bit RX
.endif

#if tsconfigRX & tsconfigOCR_MASK
rx_stop_ticks:			tsDS	1	; Only used by RX Stop Bit IR
#endif

#endif	// tsconfigRX

#endif // defined(tsBAUDRATE)

;-----------------------------------------------------------------------------
; uint16_t TXSerialBegin(uint32_t baud)
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
#define tsSERIALBEGIN tsMAKE_FUNCNAME(tsFUNC_PREFIX,SerialBegin)
#define udiv2 tsMAKE_FUNCNAME(tsFUNC_PREFIX,udiv2)
#define error tsMAKE_FUNCNAME(tsFUNC_PREFIX,error)
#define exit tsMAKE_FUNCNAME(tsFUNC_PREFIX,exit)
.section tsMAKE_SECTIONNAME(tsMAKE_SECTIONNAME(tsTEXT_SECTIONNAME,.),tsSERIALBEGIN)

.global	tsSERIALBEGIN
.func	tsSERIALBEGIN

#if !defined(tsBAUDRATE)
udiv2:
	lsr		r25
	ror		r24
	ror		r23
	ror		r22
	lsr		r0
	brne	udiv2
	ret
#endif

tsSERIALBEGIN:
	; ------------------------------------------------------------------------
	; Zero BSS section for this timer
	ldi		r30, lo8(tsBSS_START)
	ldi		r31, hi8(tsBSS_START)
	ldi		r26, lo8(tsBSS_END)
	ldi		r27, hi8(tsBSS_END)
	subi	r26, 0xFF
	sbci	r27, 0xFF
00:	st		Z+, r1
	cp		r30, r26
	cpc		r31, r27
	brne	00b

#if !defined(tsBAUDRATE)

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
	#ifdef tsconfigTX
	mov		r18, r24				; Needed for F_CPU >= 20M and baud ~300
	lsr		r18						; cycles_per_bit / 4
	ror		r27
	ror		r26						; Delay loop takes 4 cycles
	lsr		r18
	ror		r27
	ror		r26
	sts		delay_per_bit, r26		; delay_per_bit = ticks_per_bit / 4
	sts		delay_per_bit+1, r27
	#endif	// tsconfigTX

	ldi		r26, lo8(MAX_TICKS_PER_BIT)
	ldi		r27, hi8(MAX_TICKS_PER_BIT)

	; ------------------------------------------------------------------------
	; Calculate ticks_per_bit (cycles_per_bit) and timer prescale
	cp		r22, r26				; Is ticks_per_bit < MAX_TICKS_PER_BIT?
	cpc		r23, r27
	cpc		r24, r1
;	cpc		r25, r1
	brsh	01f
	ldi		r20, tsNO_PRESCALE
	rjmp	8f

01:	set								; ticks_per_bit /= 8
	bld		r0, 2
	rcall	udiv2
	cp		r22, r26
	cpc		r23, r27
	cpc		r24, r1
;	cpc		r25, r1
	brsh	02f
	ldi		r20, tsPRESCALE_8
	rjmp	8f

02:
	#ifdef tsPRESCALE_32
	bld		r0, 1					; ticks_per_bit /= 4
	rcall	udiv2
	cp		r22, r26
	cpc		r23, r27
	cpc		r24, r1
;	cpc		r25, r1
	brsh	03f
	ldi		r20, tsPRESCALE_32
	rjmp	8f
	#endif

03:	
	#ifdef tsPRESCALE_32
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
	ldi		r20, tsPRESCALE_64
	rjmp	8f

04:	
	#ifdef tsPRESCALE_128
	rcall	udiv2					; ticks_per_bit /= 2
	cp		r22, r26
	cpc		r23, r27
	cpc		r24, r1
;	cpc		r25, r1
	brsh	05f
	ldi		r20, tsPRESCALE_128
	rjmp	8f
	#endif

05:	
	#ifdef tsPRESCALE_128
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
	ldi		r20, tsPRESCALE_256
	rjmp	8f

06:	bld		r0, 1
	rcall	udiv2					; ticks_per_bit /= 4
	cp		r22, r26
	cpc		r23, r27
	cpc		r24, r1
;	cpc		r25, r1
	brlo	07f
	rjmp	error					; Error: Baud rate too slow
07:	ldi		r20, tsPRESCALE_1024

#else // !defined(tsBAUDRATE)

	ldi		r20, tsPRESCALE

#endif // !defined(tsBAUDRATE)

08:
	#ifdef tsconfigTX
	X_out	tsTXTCCRA, r1
	#endif
	X_out	tsTIMSK, r1
	X_out	tsTCCRB, r20

#if !defined(tsBAUDRATE)

	X_stsw	ticks_per_bit, r22, tsBITS

	; ------------------------------------------------------------------------
	; Calculate other constants

	#ifdef tsconfigRX

	; ticks_per_bit_and_half = ticks_per_bit * 3 / 2
	X_movw	r18, r22, tsBITS
	X_addw	r18, r18, tsBITS
	X_addw	r18, r22, tsBITS
	X_movw	r26, r18, tsBITS		; Save (ticks_per_bit * 3)
	X_lsrw	r18, tsBITS
	X_stsw	ticks_per_bit_and_half, r18, tsBITS

	.if tsBITS == 8
	; ticks_per_half_bit = 255 - ticks_per_bit/2 - 3
	; 8/16 bit one's complement (-3 good @ 14400 and lower for receive)
	mov		r18, r22
	lsr		r18
	ldi		r24, 0xFC				; 252
	sub		r24, r18
	sts		ticks_per_half_bit, r24
	.endif

	#endif	// tsconfigRX

	#ifdef tsconfigTX
	; min_lead = (ticks_per_bit/64*13) Minimum between timer counter and compare
	; 13/64 = 0.203125 (Previous value used was: 4/25 = 0.16)
	X_movw	r26, r22
	X_lsrw	r26, tsBITS				; /2
	X_lsrw	r26, tsBITS				; /4
	X_lsrw	r26, tsBITS				; /8
	X_lsrw	r26, tsBITS				; /16
	X_lsrw	r26, tsBITS				; /32
	X_lsrw	r26, tsBITS				; /64
	X_movw	r24, r26, tsBITS
	X_lsrw	r26, tsBITS				; *2
	X_addw	r26, r24, tsBITS		; *3
	X_lsrw	r26, tsBITS				; *6
	X_lsrw	r26, tsBITS				; *12
	X_addw	r26, r24, tsBITS		; *13
	X_cpzw	r26, tsBITS
	brne	9f
	ldi		r26, 1					; Use a minimum lead time of 1
09:	X_stsw	min_lead, r26, tsBITS
	#endif	// tsconfigTX

	#if tsconfigRX & tsconfigOCR_MASK
	; rx_stop_ticks = ticks_per_bit * 37 / 4  (37/4 = 9.25)
	ldi		r26, 37
	.if tsBITS == 8
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
	#endif	// tsconfigRX & tsconfigOCR_MASK

#endif // !defined(tsBAUDRATE)

	; ------------------------------------------------------------------------
	; Configure RX/TX pins

	; External interrupt pins
	#if (tsconfigRX & tsconfigINT_MASK)

	#if (tsconfigRX & tsconfigINT0)
	X_cbi	tsRXDDR_0, tsRXPIN_0, r20	; Configure as input
	X_sbi	tsRXPORT_0, tsRXPIN_0, r20	; Enable pullup
	X_in	r20, tsRXEICRA
	andi	r20, ~(1<<tsRXISC0_0)
	ori		r20, 1<<tsRXISC1_0			; Capture falling edge
	X_out	tsRXEICRA, r20
	X_in	r20, tsRXEIFR
	ori		r20, 1<<tsRXINTF_0
	X_out	tsRXEIFR, r20				; Clear interrupt flag
	X_in	r20, tsRXEIMSK
	ori		r20, 1<<tsRXINT_0
	X_out	tsRXEIMSK, r20				; Enable interrupt
	#endif

	#if (tsconfigRX & tsconfigINT1)
	X_cbi	tsRXDDR_1, tsRXPIN_1, r20	; Configure as input
	X_sbi	tsRXPORT_1, tsRXPIN_1, r20	; Enable pullup
	X_in	r20, tsRXEICRA
	andi	r20, ~(1<<tsRXISC0_1)
	ori		r20, 1<<tsRXISC1_1			; Capture falling edge
	X_out	tsRXEICRA, r20
	X_in	r20, tsRXEIFR
	ori		r20, 1<<tsRXINTF_1
	X_out	tsRXEIFR, r20				; Clear interrupt flag
	X_in	r20, tsRXEIMSK
	ori		r20, 1<<tsRXINT_1
	X_out	tsRXEIMSK, r20				; Enable interrupt
	#endif

	#if (tsconfigRX & tsconfigINT2)
	X_cbi	tsRXDDR_2, tsRXPIN_2, r20	; Configure as input
	X_sbi	tsRXPORT_2, tsRXPIN_2, r20	; Enable pullup
	X_in	r20, tsRXEICRA
	andi	r20, ~(1<<tsRXISC0_2)
	ori		r20, 1<<tsRXISC1_2			; Capture falling edge
	X_out	tsRXEICRA, r20
	X_in	r20, tsRXEIFR
	ori		r20, 1<<tsRXINTF_2
	X_out	tsRXEIFR, r20				; Clear interrupt flag
	X_in	r20, tsRXEIMSK
	ori		r20, 1<<tsRXINT_2
	X_out	tsRXEIMSK, r20				; Enable interrupt
	#endif

	#if (tsconfigRX & tsconfigINT3)
	X_cbi	tsRXDDR_3, tsRXPIN_3, r20	; Configure as input
	X_sbi	tsRXPORT_3, tsRXPIN_3, r20	; Enable pullup
	#endif

	#if (tsconfigRX & tsconfigINT4)
	X_cbi	tsRXDDR_4, tsRXPIN_4, r20	; Configure as input
	X_sbi	tsRXPORT_4, tsRXPIN_4, r20	; Enable pullup
	#endif

	#if (tsconfigRX & tsconfigINT5)
	X_cbi	tsRXDDR_5, tsRXPIN_5, r20	; Configure as input
	X_sbi	tsRXPORT_5, tsRXPIN_5, r20	; Enable pullup
	#endif

	#if (tsconfigRX & tsconfigINT6)
	X_cbi	tsRXDDR_6, tsRXPIN_6, r20	; Configure as input
	X_sbi	tsRXPORT_6, tsRXPIN_6, r20	; Enable pullup
	#endif

	#if (tsconfigRX & tsconfigINT7)
	X_cbi	tsRXDDR_7, tsRXPIN_7, r20	; Configure as input
	X_sbi	tsRXPORT_7, tsRXPIN_7, r20	; Enable pullup
	#endif

	#endif // (tsconfigRX & tsconfigINT_MASK)

	; Input capture pins
	#if (tsconfigRX & tsconfigICP_MASK)

	#if (tsconfigRX & tsconfigICP0)
	X_cbi	tsRXDDRICP_0, tsRXPINICP_0, r20		; Configure as input
	X_sbi	tsRXPORTICP_0, tsRXPINICP_0, r20	; Enable pullup
	X_in	r20, tsRXEICRAICP
	andi	r20, ~(1<<tsRXICESICP_0)
	X_out	tsRXEICRAICP, r20					; Capture falling edge
	X_in	r20, tsRXEIFRICP
	ori		r20, 1<<tsRXINTFICP_0
	X_out	tsRXEIFRICP, r20					; Clear interrupt flag
	X_in	r20, tsRXEIMSKICP
	ori		r20, 1<<tsRXINTICP_0
	X_out	tsRXEIMSKICP, r20					; Enable interrupt
	#endif

	#if (tsconfigRX & tsconfigICP1)
	X_cbi	tsRXDDRICP_1, tsRXPINICP_1, r20		; Configure as input
	X_sbi	tsRXPORTICP_1, tsRXPINICP_1, r20	; Enable pullup
	X_in	r20, tsRXEICRAICP
	andi	r20, ~(1<<tsRXICESICP_1)
	X_out	tsRXEICRAICP, r20					; Capture falling edge
	X_in	r20, tsRXEIFRICP
	ori		r20, 1<<tsRXINTFICP_1
	X_out	tsRXEIFRICP, r20					; Clear interrupt flag
	X_in	r20, tsRXEIMSKICP
	ori		r20, 1<<tsRXINTICP_1
	X_out	tsRXEIMSKICP, r20					; Enable interrupt
	#endif

	#if (tsconfigRX & tsconfigICP2)
	#endif

	#if (tsconfigRX & tsconfigICP3)
	#endif

	#if (tsconfigRX & tsconfigICP4)
	#endif

	#if (tsconfigRX & tsconfigICP5)
	#endif

	#endif // (tsconfigRX & tsconfigICP_MASK)

	#if (tsconfigRX & tsconfigPCPORT_MASK)

	; Configure receive pins as input pullup
	X_in	r20, tsRXDDR			; Configure as input
	andi	r20, ~(tsconfigRX_PINS)
	X_out	tsRXDDR, r20
	X_in	r20, tsRXPORT			; Enable pullup
	ori		r20, tsconfigRX_PINS
	X_out	tsRXPORT, r20

;	ldi		r20, 0xFF				; Set previous pin states
;	sts		rx_prev, r20

	ldi		r20, tsconfigRX_PINS	; Set mask for pins
	X_out	tsRXPCMSK, r20

	X_in	r20, tsRXEIFR			; Clear interrupt flag
	ori		r20, 1<<tsRXINTF
	X_out	tsRXEIFR, r20
	X_in	r20, tsRXEIMSK			; Enable interrupt
	ori		r20, 1<<tsRXINT
	X_out	tsRXEIMSK, r20

	#endif
	
	#ifdef tsconfigTX

	; Configure transmit pin as high output
	#if tsconfigTX & tsconfigOCR_A
	X_sbi	tsTXPORTA, tsTXPINA, r20; Set pin high
	X_sbi	tsTXDDRA, tsTXPINA, r20	; Configure as output
	#endif
	#if tsconfigTX & tsconfigOCR_B
	X_sbi	tsTXPORTB, tsTXPINB, r20; Set pin high
	X_sbi	tsTXDDRB, tsTXPINB, r20	; Configure as output
	#endif
	#if tsconfigTX & tsconfigOCR_C
	X_sbi	tsTXPORTC, tsTXPINC, r20; Set pin high
	X_sbi	tsTXDDRC, tsTXPINC, r20	; Configure as output
	#endif

	#endif	// tsconfigTX

	clr		r24						; Return success
	clr		r25
#if !defined(tsBAUDRATE)
	rjmp	exit

error:
	ldi		r24, 0xFF				; Return error
	ldi		r25, 0xFF

#endif // !defined(tsBAUDRATE)

exit:
	ret

.endfunc	; tsSERIALBEGIN

;-----------------------------------------------------------------------------
; void TXSerialEnd(void)
;
; Terminate the timer.
;
; Local Variables:
;		r20		Prescale config and temp
;		r25:r22	cycles_per_bit = (F_CPU + baud/2) / baud
;		r27:r26	max_ticks_per_bit
;-----------------------------------------------------------------------------
#define tsSERIALEND tsMAKE_FUNCNAME(tsFUNC_PREFIX,SerialEnd)
.section tsMAKE_SECTIONNAME(tsMAKE_SECTIONNAME(tsTEXT_SECTIONNAME,.),tsSERIALEND)

.global	tsSERIALEND
.func	tsSERIALEND

tsSERIALEND:
	; Input capture pins
	#if (tsconfigRX & tsconfigICP_MASK)

	X_in	r20, tsRXEIMSKICP

	#if (tsconfigRX & tsconfigICP0)
	andi	r20, ~(1<<tsRXINTICP_0)
	#endif

	#if (tsconfigRX & tsconfigICP1)
	andi	r20, ~(1<<tsRXINTICP_1)
	#endif

	#if (tsconfigRX & tsconfigICP2)
	#endif

	#if (tsconfigRX & tsconfigICP3)
	#endif

	#if (tsconfigRX & tsconfigICP4)
	#endif

	#if (tsconfigRX & tsconfigICP5)
	#endif

	X_out	tsRXEIMSKICP, r20					; Disable interrupt

	#endif // (tsconfigRX & tsconfigICP_MASK)

	#ifdef tsconfigTX

	#if tsconfigTX & tsconfigOCR_A
;	#warning "ToDo Need a method to disable TX interrupt"
;	X_in	r18, tsTIMSK
;	ori		r18, 1<<tsTXOCIE
;	X_out	tsTIMSK, r18
	#endif
	#if tsconfigTX & tsconfigOCR_B
;	#warning "ToDo Need a method to disable TX interrupt"
;	X_in	r18, tsTIMSK
;	ori		r18, 1<<tsTXOCIE
;	X_out	tsTIMSK, r18
	#endif
	#if tsconfigTX & tsconfigOCR_C
;	#warning "ToDo Need a method to disable TX interrupt"
;	X_in	r18, tsTIMSK
;	ori		r18, 1<<tsTXOCIE
;	X_out	tsTIMSK, r18
	#endif

	#endif	// tsconfigTX

	#if (tsconfigRX & tsconfigPCPORT_MASK)

;	ldi		r20, tsconfigRX_PINS
;	X_out	tsRXPCMSK, r20

	#elif defined(tsconfigRX)

	; ------------------------------------------------------------------------
	; Configure edge detect for the RX INTX or ICPX Interrupt
;	X_in	r20, tsRXEICRA			; Capture falling edge
;	#ifdef tsRXICES
;	andi	r20, ~(1<<tsRXICES)		; Input Capture Pin
;	#else
;	andi	r20, ~(1<<tsRXISC0)		; External Interrupt Pin
;	ori		r20, 1<<tsRXISC1
;	#endif
;	X_out	tsRXEICRA, r20

	#endif // (tsconfigRX & tsconfigPCPORT_MASK)

	#ifdef tsconfigRX

	; ------------------------------------------------------------------------
	; Enable the RX INTX, ICPX, or Pin Change interrupt
;	X_in	r20, tsRXEIFR			; Clear interrupt flag
;	ori		r20, 1<<tsRXINTF
;	X_out	tsRXEIFR, r20
;	X_in	r20, tsRXEIMSK			; Enable interrupt
;	ori		r20, 1<<tsRXINT
;	X_out	tsRXEIMSK, r20

	#endif	// tsconfigRX

	ret

;	#warning "ToDo: Finish SerialEnd routine"

.endfunc	; tsSERIALEND

#undef tsconfigTX
#undef tsconfigRX
#undef tsFUNC_PREFIX
