;---------------------------------------------------------------------------;
; Timer based software serial ports RX routines
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

#ifdef __AVR_ATmega328P__
#define DBGPORT		0x08
#endif

#ifdef __AVR_ATmega1284P__
#define DBGPORT		0x02
#endif

.if RX_BUFFER_SIZE > 256
	.error "RX_BUFFER_SIZE must be 256 or less"
.endif
.if RX_BUFFER_SIZE & RX_BUFFER_MASK
	.error "RTX_BUFFER_SIZE must be a power of 2"
.endif

; State machine states
#define tsRxSTART		0
#define tsRxBIT0		1
#define tsRxBIT1		2
#define tsRxBIT2		3
#define tsRxBIT3		4
#define tsRxBIT4		5
#define tsRxBIT5		6
#define tsRxBIT6		7
#define tsRxBIT7		8
#define tsRxSTOP		9

.section tsBSS_SECTIONNAME

#if !(defined(tsconfigRX_PINS) && (((tsconfigRX_PINS-1) & tsconfigRX_PINS) > 0))

;-----------------------------------------------------------------------------
; This section handles the RX interrupt routines for when the receive pin
; is an ICP, INT, or a single PCINT. The routines handle both binary and
; ASCII serial ports.
;-----------------------------------------------------------------------------

#define tsVAR_RX_PREFIX			tsMAKE_REGNAME(tsVAR_RX_PREFIX,tsREG_RX_SUFFIX)

#define rx_state				tsMAKE_VARNAME(tsVAR_RX_PREFIX,rx_state)
#define rx_byte					tsMAKE_VARNAME(tsVAR_RX_PREFIX,rx_byte)
#define	rx_bit					tsMAKE_VARNAME(tsVAR_RX_PREFIX,rx_bit)
#define rx_target				tsMAKE_VARNAME(tsVAR_RX_PREFIX,rx_target)
#define rx_buffer_head			tsMAKE_VARNAME(tsVAR_RX_PREFIX,rx_buffer_head)
#define rx_buffer_tail			tsMAKE_VARNAME(tsVAR_RX_PREFIX,rx_buffer_tail)
#define rx_buffer_overflow		tsMAKE_VARNAME(tsVAR_RX_PREFIX,rx_buffer_overflow)
#define rx_buffer				tsMAKE_VARNAME(tsVAR_RX_PREFIX,rx_buffer)

; Space for rx_stop_ticks is allocated in tserial_timer.inc
#define rx_stop_ticks			tsMAKE_VARNAME(tsVAR_TIMER_PREFIX,_rx_stop_ticks)

#define tsRXISC0				tsMAKE_REGNAME(tsRXISC0,tsREG_INT_SUFFIX)
#define tsRXISC1				tsMAKE_REGNAME(tsRXISC1,tsREG_INT_SUFFIX)

#if tsconfigRX & tsconfigICP_MASK
#define tsRXICES				tsMAKE_REGNAME(tsRXICESICP,tsREG_ICP_SUFFIX)
#define tsRXEICRA				tsRXEICRAICP
#endif

#ifdef tsconfigRX_PINS
#define rx_prev					tsMAKE_VARNAME(tsVAR_RX_PREFIX,rx_prev)
.global rx_prev

.section tsDATA_SECTIONNAME
rx_prev:					.byte	0xFF
.section tsBSS_SECTIONNAME
#endif

;-----------------------------------------------------------------------------
; RX Interrupt Routine
;
; This routine uses pin change interrupt ICPX, INTX, or a single PCINT pin
; to get level change.
;
; State		Name		Description
;	0		tsRxSTART	Waiting for line level to go low (0).
;  1-8		tsRxBIT0-7	Waiting for data bits.
;   9		tsRxSTOP	Waiting for stop bit. Set state to tsRxSTART.
;
; Data Byte: 0x9B 0b10011011
;
; Line Level:     1   0   1   1   0   1   1   0   0   1   1
; State:		    Strt B0  B1  B2  B3  B4  B5  B6  B7 Stop  
;				+---+   +---+---+   +---+---+       +---+---+
;   				|   |       |   |       |       |
;	    			|   |       |   |       |       |
;	    			+---+       +---+       +---+---+
;
;			   	    |   |   |   |   |   |   |   |   |   |
;			   	    |   |   |   |   |   |   |   |   |   +-- tsRxSTOP
;			   	    |   |   |   |   |   |   |   |   +-- tsRxBIT7
;			   	    |   |   |   |   |   |   |   +-- tsRxBIT6
;			   	    |   |   |   |   |   |   +-- tsRxBIT5
;			   	    |   |   |   |   |   +-- tsRxBIT4
;			   	    |   |   |   |   +-- tsRxBIT3
;			   	    |   |   |   +-- tsRxBIT2
;			   	    |   |   +-- tsRxBIT1
;			   	    |   +-- tsRxBIT0
;			   	    +-- tsRxSTART
;
;	NOTE:	Data bits are received LSB first.
;			The IR routine is called after the line level changes.
;
; Local Variables:
;		r15		HOLD_SREG
;		r16		CAPTURE	or	HEAD
;		r17		CAPTUREH
;		r18		STATE
;		r19		BYTE
;		r20		BIT
;		r22		TARGET
;		r23		TARGETH
;		r24		TEMP
;		r25		TEMPH
;		r26		XL		or	TEMP2
;		r27		XH		or	TEMP2H
;		r28		TEMP3
;		r29		DIFF
;-----------------------------------------------------------------------------
.section tsTEXT_SECTIONNAME

; These are undefined at the bottom of the function
#define	HOLD_SREG	r15
#define	HEAD		r16
#define	CAPTURE		r16
#define	CAPTUREH	r17
#define	STATE		r18
#define	BYTE		r19
#define	BIT			r20
#define	TARGET		r22
#define	TARGETH		r23
#define	TEMP		r24
#define	TEMPH		r25
#define	TEMP2		r26
#define	TEMP2H		r27
#define	TEMP3		r28
#define	DIFF		r29

.global tsRX_IR
.func tsRX_IR
tsRX_IR:
;#warning "Fix RX IR"
;sbi	DBGPORT, 1
	push	HOLD_SREG
	push	CAPTURE
	push	STATE
	push	BYTE
	push 	BIT
	push	TARGET
	push	TEMP
	push	XL
	push	XH
	.if tsBITS == 8
	push	TEMP3
	.else
	push	CAPTUREH
	push	TARGETH
	push	TEMPH
	.endif
	#if tsconfigRX & tsconfigPCPORT_MASK
	push	DIFF
	#endif
	X_in	HOLD_SREG, SREG

	; ------------------------------------------------------------------------
	; Counter when RX level changed
	X_inw	CAPTURE, tsRXTCNT, tsBITS

	#if tsconfigRX & tsconfigPCPORT_MASK

	; ------------------------------------------------------------------------
	; Find out which pins changed
	lds		DIFF, rx_prev			; Get previous pin states
	X_in	TEMP2, tsRXINPUT		; Get current pin states
	sts		rx_prev, TEMP2			; Save current states to previous
	eor		DIFF, TEMP2				; Changed pins = prev xor current
	ldi		TEMP, tsconfigRX_PINS	; Get pins we are watching
	and		DIFF, TEMP				; Did one of our pins change?
	brne	10f
	rjmp	53f						; Exit

10:	ldi		TEMP, 0x80
	lds		BIT, rx_bit
	eor		BIT, TEMP				; Next bit we are expecting

	#else // tsconfigRX & tsconfigPCPORT_MASK

	; ------------------------------------------------------------------------
	; Set the next rising/falling edge to capture depending on current bit
	X_in	TEMP, tsRXEICRA
	lds		BIT, rx_bit
	sbrs	BIT, 7
	rjmp	20f
									; Capture falling edge
	#if tsconfigRX & tsconfigICP_MASK
	andi	TEMP, ~(1<<tsRXICES)	; Input Capture Pin
	#else
	andi	TEMP, ~(1<<tsRXISC0)	; External Interrupt Pin
	ori		TEMP, 1<<tsRXISC1
	#endif
	ldi		BIT, 0x00				; Next bit will be zero
	rjmp	21f
20:									; Capture rising edge
	#ifdef tsRXICES
	ori		TEMP, (1<<tsRXICES)		; Input Capture Pin
	#else
									; External Interrupt Pin
	ori		TEMP, (1<<tsRXISC1) | (1<<tsRXISC0)
	#endif
	ldi		BIT, 0x80				; Next bit will be one
21:	X_out	tsRXEICRA, TEMP

	#endif // tsconfigRX & tsconfigPCPORT_MASK

	lds		STATE, rx_state

	; ------------------------------------------------------------------------
	; Handle start bit (tsRxSTART)
	tst		STATE
	brne	30f

	#if tsconfigRX & tsconfigPCPORT_MASK

	andi	TEMP2, tsconfigRX_PINS	; Verify that RX level is zero
	brne	53f						; Exit

	#else // tsconfigRX & tsconfigPCPORT_MASK

	sbrs	BIT, 7
	rjmp	52f						; Exit, start bit should be zero

	#if !(tsconfigRX & tsconfigASCII)
	X_ldsiw	TEMP, rx_stop_ticks, tsBITS, tsIMMEDIATE
	X_addw	TEMP, CAPTURE, tsBITS	; Timer value when stop bit should arrive
	X_outw	tsRXOCR, TEMP, tsBITS
	
	ldi		TEMP, 1<<tsRXOCF		; Enable stop bit interrupt that handles
	X_out	tsRXTIFR, TEMP			; when the MSB of the data byte is one.
	X_in	TEMP, tsTIMSK			; Since the stop bit is also one, there
	ori		TEMP, 1<<tsRXOCIE		; will be no rising edge to trigger this
	X_out	tsTIMSK, TEMP			; interrupt.
	#endif // !(tsconfigRX & tsconfigASCII)

	#endif // tsconfigRX & tsconfigPCPORT_MASK

									; Counter value half way into next bit
	X_ldsiw	TARGET, ticks_per_bit_and_half, tsBITS, tsIMMEDIATE
	X_addw	TARGET, CAPTURE, tsBITS
	inc		STATE					; Change state for first data bit (LSB)
	rjmp	51f						; Exit

	; ------------------------------------------------------------------------
	; Handle data and stop bits
	; Should be half way into bit
30:	X_ldsw	TARGET, rx_target, tsBITS
	lds		BYTE, rx_byte
	X_ldsiw	TEMP2, ticks_per_bit, tsBITS, tsIMMEDIATE
	.if tsBITS == 8
	X_ldsiw	TEMP3, ticks_per_half_bit, tsBITS, tsIMMEDIATE
	.endif

31:									; Loop to count number of bits received
	X_movw	TEMP, CAPTURE, tsBITS	; ticks_per_half_bit < (CAPTURE - TARGET)
	X_subw	TEMP, TARGET, tsBITS
	.if tsBITS == 8
	cp		TEMP3, TEMP
	brlo	50f						; Not enough time for a bit, exit
	.else
	brmi	50f						; Not enough time for a bit, exit
	.endif
	lsr		BYTE
	or		BYTE, BIT				; Bits received LSB first

	X_addw	TARGET, TEMP2, tsBITS	; Add time for bit
	inc		STATE					; State is current bit waiting for

	cpi		STATE, tsRxSTOP			; Stop at stop bit
	brlo	31b						; Loop to see if next bit has arrived

	#if !(tsconfigRX & tsconfigASCII)
	X_in	TEMP, tsTIMSK			; Have a stop bit so disable stop bit
	andi	TEMP, ~(1<<tsRXOCIE)	; interrupt.
	X_out	tsTIMSK, TEMP
	#endif // !(tsconfigRX & tsconfigASCII)

	; ------------------------------------------------------------------------
	; Place byte into RX buffer
	lds		HEAD, rx_buffer_head
	inc		HEAD
	.if RX_BUFFER_MASK <> 255
	andi	HEAD, RX_BUFFER_MASK
	.endif

	lds		TEMP, rx_buffer_tail
	cp		HEAD, TEMP
	breq	40f
	mov		XL, HEAD
	clr		XH
	subi	XL, lo8(-(rx_buffer))	; Offset into buffer
	sbci	XH, hi8(-(rx_buffer))
	st		X, BYTE
	sts		rx_buffer_head, HEAD
	rjmp	41f
40: ldi		TEMP, 1					; Set overflow flag
	sts		rx_buffer_overflow, TEMP

41:
	#if !(tsconfigRX & tsconfigPCPORT_MASK)

	X_in	TEMP, tsRXEICRA			; Prepare for next start bit
									; Capture falling edge
	#ifdef tsRXICES
	andi	TEMP, ~(1<<tsRXICES)	; Input Capture Pin
	#else
	andi	TEMP, ~(1<<tsRXISC0)	; External Interrupt Pin
	ori		TEMP, 1<<tsRXISC1
	#endif
	X_out	tsRXEICRA, TEMP

	#endif // !(tsconfigRX & tsconfigPCPORT_MASK)

	ldi		BIT, 0x00				; Next bit will be zero
	ldi		STATE, tsRxSTART

	; ------------------------------------------------------------------------
	; Exit
50:	sts		rx_byte, BYTE

51:	X_stsw	rx_target, TARGET, tsBITS
	sts		rx_state, STATE

52:	sts		rx_bit, BIT				; Save next expected bit

53:	X_out	SREG, HOLD_SREG
	#if tsconfigRX & tsconfigPCPORT_MASK
	pop		DIFF
	#endif
	.if tsBITS == 16
	pop		TEMPH
	pop		TARGETH
	pop		CAPTUREH
	.else
	pop		TEMP3
	.endif
	pop		XH
	pop		XL
	pop		TEMP
	pop		TARGET
	pop		BIT
	pop		BYTE
	pop		STATE
	pop		CAPTURE
	pop		HOLD_SREG
;#warning "Fix RX IR"
;cbi	DBGPORT, 1
	reti

.endfunc							; tsRX_IR

#undef	HOLD_SREG
#undef	HEAD
#undef	CAPTURE
#undef	CAPTUREH
#undef	STATE
#undef	BYTE
#undef	BIT
#undef	TARGET
#undef	TARGETH
#undef	TEMP
#undef	TEMPH
#undef	TEMP2
#undef	TEMP3
#undef	TEMP2H
#undef	TEMP3
#undef	DIFF

#if !(tsconfigRX & tsconfigASCII)
;-----------------------------------------------------------------------------
; RX Stop Bit Interrupt Routine
;
; This routine is called when the stop bit should arrive. It is used to
; handle data values from 0x80 to 0xFF. These values have the MSB set. The
; stop bit is also high. The RX interrupt routine needs a level change to
; detect the stop bit. It can only do that when the MSB of the data byte is
; low (0x00 - 0x7F). This routine catches the stop bit for data bytes > 0x7F.
;
; Local Variables:
;		r15		HOLD_SREG
;		r19		BYTE
;		r26		XL		or	HEAD
;		r27		XH		or	TEMP
;-----------------------------------------------------------------------------
.section tsTEXT_SECTIONNAME

; These are undefined at the bottom of the function
#define	HOLD_SREG	r15
#define	BYTE		r19
#define	HEAD		r26
#define	TEMP		r27

.global tsRX_SB_IR
.func tsRX_SB_IR
tsRX_SB_IR:
;#warning "Fix RX Stop Bit IR"
;sbi	DBGPORT, 2
	push	HOLD_SREG
	push	BYTE
	push	XL
	push	XH
	X_in	HOLD_SREG, SREG

	X_in	TEMP, tsTIMSK			; Disable this interrupt
	andi	TEMP, ~(1<<tsRXOCIE)
	X_out	tsTIMSK, TEMP

	#if !(tsconfigRX & tsconfigPCPORT_MASK)

	X_in	TEMP, tsRXEICRA			; Prepare for next start bit (0)
									; Capture falling edge
	#ifdef tsRXICES
	andi	TEMP, ~(1<<tsRXICES)	; Input Capture Pin
	#else
	andi	TEMP, ~(1<<tsRXISC0)	; External Interrupt Pin
	ori		TEMP, 1<<tsRXISC1
	#endif

	X_out	tsRXEICRA, TEMP

	#endif // !(tsconfigRX & tsconfigPCPORT_MASK)

	; ------------------------------------------------------------------------
	; Count remaining bits
	lds		BYTE, rx_byte			; Set MSB to 1
	sec
	ror		BYTE
	lds		HEAD, rx_state			; Calc bits left to shift minus bit
	ldi		TEMP, 8					; from above
	sub		TEMP, HEAD
	breq	02f

01:	asr		BYTE
	dec		TEMP
	brne	01b

	; ------------------------------------------------------------------------
	; Place byte into RX buffer
02:	lds		HEAD, rx_buffer_head	; Get offset into buffer
	inc		HEAD
	.if RX_BUFFER_MASK <> 255
	andi	HEAD, RX_BUFFER_MASK
	.endif

	lds		TEMP, rx_buffer_tail
	cp		HEAD, TEMP
	breq	03f
	sts		rx_buffer_head, HEAD
	clr		XH
	subi	XL, lo8(-(rx_buffer))	; Offset into buffer
	sbci	XH, hi8(-(rx_buffer))
	st		X, BYTE
	rjmp	04f
03: ldi		TEMP, 1					; Set overflow flag
	sts		rx_buffer_overflow, TEMP

04:	clr		TEMP
	sts		rx_state, TEMP			; State tsRxSTART for start bit (0)
	sts		rx_bit, TEMP			; Start bit is zero

	X_out	SREG, HOLD_SREG
	pop		XH
	pop		XL
	pop		BYTE
	pop		HOLD_SREG
;#warning "Fix RX Stop Bit IR"
;cbi	DBGPORT, 2
	reti

.endfunc							; tsRX_SB_IR

#undef	HOLD_SREG
#undef	STATE
#undef	HEAD
#undef	BYTE
#undef	TEMP

#endif // !(tsconfigRX & tsconfigASCII)

#else // !(defined(tsconfigRX_PINS) && (((tsconfigRX_PINS-1) & tsconfigRX_PINS) > 0))

;-----------------------------------------------------------------------------
; This section handles the RX interrupt routines for when the receive pins
; are multiple PCINTs. The routines handle only ASCII serial ports.
;-----------------------------------------------------------------------------

#if tsconfigRX & tsconfigPCPORTA
	#define tsVAR_RX_PCPORT			tsMAKE_VARNAME(tsVAR_TIMER_PREFIX,pa)
#elif tsconfigRX & tsconfigPCPORTB
	#define tsVAR_RX_PCPORT			tsMAKE_VARNAME(tsVAR_TIMER_PREFIX,pb)
#elif tsconfigRX & tsconfigPCPORTC
	#define tsVAR_RX_PCPORT			tsMAKE_VARNAME(tsVAR_TIMER_PREFIX,pc)
#elif tsconfigRX & tsconfigPCPORTD
	#define tsVAR_RX_PCPORT			tsMAKE_VARNAME(tsVAR_TIMER_PREFIX,pd)
#endif

#define rx_prev	tsMAKE_VARNAME(tsVAR_RX_PCPORT,_rx_prev)
.global rx_prev

.section tsDATA_SECTIONNAME
rx_prev:					.byte	0xFF
.section tsBSS_SECTIONNAME

;-----------------------------------------------------------------------------
; RX Interrupt Routine
;
; This routine handles multiple PCINTX pins as the input levels.
;
; State		Name		Description
;	0		tsRxSTART	Waiting for line level to go low (0).
;  1-8		tsRxBIT0-7	Waiting for data bits.
;   9		tsRxSTOP	Waiting for stop bit. Set state to tsRxSTART.
;
; Data Byte: 0x9B 0b10011011
;
; Line Level:     1   0   1   1   0   1   1   0   0   1   1
; State:		    Strt B0  B1  B2  B3  B4  B5  B6  B7 Stop  
;				+---+   +---+---+   +---+---+       +---+---+
;   				|   |       |   |       |       |
;	    			|   |       |   |       |       |
;	    			+---+       +---+       +---+---+
;
;			   	    |   |   |   |   |   |   |   |   |   |
;			   	    |   |   |   |   |   |   |   |   |   +-- tsRxSTOP
;			   	    |   |   |   |   |   |   |   |   +-- tsRxBIT7
;			   	    |   |   |   |   |   |   |   +-- tsRxBIT6
;			   	    |   |   |   |   |   |   +-- tsRxBIT5
;			   	    |   |   |   |   |   +-- tsRxBIT4
;			   	    |   |   |   |   +-- tsRxBIT3
;			   	    |   |   |   +-- tsRxBIT2
;			   	    |   |   +-- tsRxBIT1
;			   	    |   +-- tsRxBIT0
;			   	    +-- tsRxSTART
;
;	NOTE:	Data bits are received LSB first.
;			The IR routine is called after the line level changes.
;
; Local Variables:
;		r15		HOLD_SREG
;		r16		CAPTURE	or	HEAD
;		r17		CAPTUREH
;		r18		STATE
;		r19		BYTE
;		r20		BIT
;		r21		LEVELS
;		r22		TARGET
;		r23		TARGETH
;		r24		TEMP
;		r25		TEMPH
;		r26		XL		or	TEMP2
;		r27		XH		or	TEMP2H
;		r28		TEMP3
;		r29		DIFF
;-----------------------------------------------------------------------------
.section tsTEXT_SECTIONNAME

; These are undefined at the bottom of the function
#define	HOLD_SREG	r15
#define	HEAD		r16
#define	CAPTURE		r16
#define	CAPTUREH	r17
#define	STATE		r18
#define	BYTE		r19
#define	BIT			r20
#define LEVELS		r21
#define	TARGET		r22
#define	TARGETH		r23
#define	TEMP		r24
#define	TEMPH		r25
#define	TEMP2		r26
#define	TEMP2H		r27
#define	TEMP3		r28
#define	DIFF		r29

.global tsRX_IR
.func tsRX_IR
tsRX_IR:
;#warning "Fix ASCII RX IR"
;sbi	0x08, 0
	push	HOLD_SREG
	push	CAPTURE
	push	STATE
	push	BYTE
	push 	BIT
	push	LEVELS
	push	TARGET
	push	TEMP
	push	XL
	push	XH
	.if tsBITS == 8
	push	TEMP3
	.else
	push	CAPTUREH
	push	TARGETH
	push	TEMPH
	.endif
	push	DIFF
	X_in	HOLD_SREG, SREG

	; ------------------------------------------------------------------------
	; Counter when RX level changed
	X_inw	CAPTURE, tsRXTCNT, tsBITS

	; ------------------------------------------------------------------------
	; Find out which pins changed
	lds		DIFF, rx_prev			; Get previous pin states
	X_in	LEVELS, tsRXINPUT		; Get current pin states
	sts		rx_prev, LEVELS			; Save current states to previous
	eor		DIFF, LEVELS			; Changed pins = prev xor current

#if tsconfigRX_PINS & tsconfigPCBIT0

#define rx_state				tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,0_),rx_state)
#define rx_byte					tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,0_),rx_byte)
#define	rx_bit					tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,0_),rx_bit)
#define rx_target				tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,0_),rx_target)
#define rx_buffer_head			tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,0_),rx_buffer_head)
#define rx_buffer_tail			tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,0_),rx_buffer_tail)
#define rx_buffer_overflow		tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,0_),rx_buffer_overflow)
#define rx_buffer				tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,0_),rx_buffer)

	sbrs	DIFF, 0					; Did pin change?
	rjmp	11f

	ldi		TEMP, 0x80
	lds		BIT, rx_bit
	eor		BIT, TEMP				; Next bit we are expecting

	lds		STATE, rx_state

	; ------------------------------------------------------------------------
	; Handle start bit (state == 0)
	tst		STATE
	brne	03f

	sbrc	LEVELS, 0				; Verify that RX level is zero
	rjmp	10f

									; Counter value half way into next bit
	X_ldsiw	TARGET, ticks_per_bit_and_half, tsBITS, tsIMMEDIATE
	X_addw	TARGET, CAPTURE, tsBITS
	inc		STATE					; Change state for first data bit (LSB)
	rjmp	8f

	; ------------------------------------------------------------------------
	; Handle data and stop bits
	; Should be half way into bit
03:	X_ldsw	TARGET, rx_target, tsBITS
	lds		BYTE, rx_byte
	X_ldsiw	TEMP2, ticks_per_bit, tsBITS, tsIMMEDIATE
	.if tsBITS == 8
	X_ldsiw	TEMP3, ticks_per_half_bit, tsBITS, tsIMMEDIATE
	.endif

04:									; Loop to count number of bits received
	X_movw	TEMP, CAPTURE, tsBITS	; ticks_per_half_bit < (CAPTURE - TARGET)
	X_subw	TEMP, TARGET, tsBITS
	.if tsBITS == 8
	cp		TEMP3, TEMP
	brlo	07f						; Not enough time for a bit
	.else
	brmi	07f						; Not enough time for a bit
	.endif
	lsr		BYTE
	or		BYTE, BIT				; Bits received LSB first
									; Add time for bit
	X_addw	TARGET, TEMP2, tsBITS
	inc		STATE					; State is current bit waiting for

	cpi		STATE, 9				; Stop bit is state 9 (10th bit)
	brlo	04b						; Loop to see if next bit has arrived

	; ------------------------------------------------------------------------
	; Place byte into RX buffer
	lds		HEAD, rx_buffer_head
	inc		HEAD
	.if RX_BUFFER_MASK <> 255
	andi	HEAD, RX_BUFFER_MASK
	.endif

	lds		TEMP, rx_buffer_tail
	cp		HEAD, TEMP
	breq	05f
	mov		XL, HEAD
	clr		XH
	subi	XL, lo8(-(rx_buffer))	; Offset into buffer
	sbci	XH, hi8(-(rx_buffer))
	st		X, BYTE
	sts		rx_buffer_head, HEAD
	rjmp	06f
05: ldi		TEMP, 1					; Set overflow flag
	sts		rx_buffer_overflow, TEMP

06:	ldi		BIT, 0x00				; Next bit will be zero
	ldi		STATE, 0				; State zero is for start bit (0)

	; ------------------------------------------------------------------------
	; Save current state for next interrupt
07:	sts		rx_byte, BYTE

08:	X_stsw	rx_target, TARGET, tsBITS
	sts		rx_state, STATE

09:	sts		rx_bit, BIT				; Save next expected bit

#undef rx_state
#undef rx_byte
#undef rx_bit
#undef rx_target
#undef rx_buffer_head
#undef rx_buffer_tail
#undef rx_buffer_overflow
#undef rx_buffer

#endif  // tsconfigRX_PINS & tsconfigPCBIT0

11:

#if tsconfigRX_PINS & tsconfigPCBIT1

#define rx_state				tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,1_),rx_state)
#define rx_byte					tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,1_),rx_byte)
#define	rx_bit					tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,1_),rx_bit)
#define rx_target				tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,1_),rx_target)
#define rx_buffer_head			tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,1_),rx_buffer_head)
#define rx_buffer_tail			tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,1_),rx_buffer_tail)
#define rx_buffer_overflow		tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,1_),rx_buffer_overflow)
#define rx_buffer				tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,1_),rx_buffer)

	sbrs	DIFF, 1					; Did pin change?
	rjmp	22f

	ldi		TEMP, 0x80
	lds		BIT, rx_bit
	eor		BIT, TEMP				; Next bit we are expecting

	lds		STATE, rx_state

	; ------------------------------------------------------------------------
	; Handle start bit (state == 0)
	tst		STATE
	brne	03f

	sbrc	LEVELS, 1				; Verify that RX level is zero
	rjmp	10f

									; Counter value half way into next bit
	X_ldsiw	TARGET, ticks_per_bit_and_half, tsBITS, tsIMMEDIATE
	X_addw	TARGET, CAPTURE, tsBITS
	inc		STATE					; Change state for first data bit (LSB)
	rjmp	8f

	; ------------------------------------------------------------------------
	; Handle data and stop bits
	; Should be half way into bit
03:	X_ldsw	TARGET, rx_target, tsBITS
	lds		BYTE, rx_byte
	X_ldsiw	TEMP2, ticks_per_bit, tsBITS, tsIMMEDIATE
	.if tsBITS == 8
	X_ldsiw	TEMP3, ticks_per_half_bit, tsBITS, tsIMMEDIATE
	.endif

04:									; Loop to count number of bits received
	X_movw	TEMP, CAPTURE, tsBITS	; ticks_per_half_bit < (CAPTURE - TARGET)
	X_subw	TEMP, TARGET, tsBITS
	.if tsBITS == 8
	cp		TEMP3, TEMP
	brlo	07f						; Not enough time for a bit
	.else
	brmi	07f						; Not enough time for a bit
	.endif
	lsr		BYTE
	or		BYTE, BIT				; Bits received LSB first
									; Add time for bit
	X_addw	TARGET, TEMP2, tsBITS
	inc		STATE					; State is current bit waiting for

	cpi		STATE, 9				; Stop bit is state 9 (10th bit)
	brlo	04b						; Loop to see if next bit has arrived

	; ------------------------------------------------------------------------
	; Place byte into RX buffer
	lds		HEAD, rx_buffer_head
	inc		HEAD
	.if RX_BUFFER_MASK <> 255
	andi	HEAD, RX_BUFFER_MASK
	.endif

	lds		TEMP, rx_buffer_tail
	cp		HEAD, TEMP
	breq	05f
	mov		XL, HEAD
	clr		XH
	subi	XL, lo8(-(rx_buffer))	; Offset into buffer
	sbci	XH, hi8(-(rx_buffer))
	st		X, BYTE
	sts		rx_buffer_head, HEAD
	rjmp	06f
05: ldi		TEMP, 1					; Set overflow flag
	sts		rx_buffer_overflow, TEMP

06:	ldi		BIT, 0x00				; Next bit will be zero
	ldi		STATE, 0				; State zero is for start bit (0)

	; ------------------------------------------------------------------------
	; Save current state for next interrupt
07:	sts		rx_byte, BYTE

08:	X_stsw	rx_target, TARGET, tsBITS
	sts		rx_state, STATE

09:	sts		rx_bit, BIT				; Save next expected bit

#undef rx_state
#undef rx_byte
#undef rx_bit
#undef rx_target
#undef rx_buffer_head
#undef rx_buffer_tail
#undef rx_buffer_overflow
#undef rx_buffer

#endif  // tsconfigRX_PINS & tsconfigPCBIT1

22:

#if tsconfigRX_PINS & tsconfigPCBIT2

#define rx_state				tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,2_),rx_state)
#define rx_byte					tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,2_),rx_byte)
#define	rx_bit					tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,2_),rx_bit)
#define rx_target				tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,2_),rx_target)
#define rx_buffer_head			tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,2_),rx_buffer_head)
#define rx_buffer_tail			tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,2_),rx_buffer_tail)
#define rx_buffer_overflow		tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,2_),rx_buffer_overflow)
#define rx_buffer				tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,2_),rx_buffer)

	sbrs	DIFF, 2					; Did pin change?
	rjmp	33f

	ldi		TEMP, 0x80
	lds		BIT, rx_bit
	eor		BIT, TEMP				; Next bit we are expecting

	lds		STATE, rx_state

	; ------------------------------------------------------------------------
	; Handle start bit (state == 0)
	tst		STATE
	brne	03f

	sbrc	LEVELS, 2				; Verify that RX level is zero
	rjmp	10f

									; Counter value half way into next bit
	X_ldsiw	TARGET, ticks_per_bit_and_half, tsBITS, tsIMMEDIATE
	X_addw	TARGET, CAPTURE, tsBITS
	inc		STATE					; Change state for first data bit (LSB)
	rjmp	8f

	; ------------------------------------------------------------------------
	; Handle data and stop bits
	; Should be half way into bit
03:	X_ldsw	TARGET, rx_target, tsBITS
	lds		BYTE, rx_byte
	X_ldsiw	TEMP2, ticks_per_bit, tsBITS, tsIMMEDIATE
	.if tsBITS == 8
	X_ldsiw	TEMP3, ticks_per_half_bit, tsBITS, tsIMMEDIATE
	.endif

04:									; Loop to count number of bits received
	X_movw	TEMP, CAPTURE, tsBITS	; ticks_per_half_bit < (CAPTURE - TARGET)
	X_subw	TEMP, TARGET, tsBITS
	.if tsBITS == 8
	cp		TEMP3, TEMP
	brlo	07f						; Not enough time for a bit
	.else
	brmi	07f						; Not enough time for a bit
	.endif
	lsr		BYTE
	or		BYTE, BIT				; Bits received LSB first
									; Add time for bit
	X_addw	TARGET, TEMP2, tsBITS
	inc		STATE					; State is current bit waiting for

	cpi		STATE, 9				; Stop bit is state 9 (10th bit)
	brlo	04b						; Loop to see if next bit has arrived

	; ------------------------------------------------------------------------
	; Place byte into RX buffer
	lds		HEAD, rx_buffer_head
	inc		HEAD
	.if RX_BUFFER_MASK <> 255
	andi	HEAD, RX_BUFFER_MASK
	.endif

	lds		TEMP, rx_buffer_tail
	cp		HEAD, TEMP
	breq	05f
	mov		XL, HEAD
	clr		XH
	subi	XL, lo8(-(rx_buffer))	; Offset into buffer
	sbci	XH, hi8(-(rx_buffer))
	st		X, BYTE
	sts		rx_buffer_head, HEAD
	rjmp	06f
05: ldi		TEMP, 1					; Set overflow flag
	sts		rx_buffer_overflow, TEMP

06:	ldi		BIT, 0x00				; Next bit will be zero
	ldi		STATE, 0				; State zero is for start bit (0)

	; ------------------------------------------------------------------------
	; Save current state for next interrupt
07:	sts		rx_byte, BYTE

08:	X_stsw	rx_target, TARGET, tsBITS
	sts		rx_state, STATE

09:	sts		rx_bit, BIT				; Save next expected bit

#undef rx_state
#undef rx_byte
#undef rx_bit
#undef rx_target
#undef rx_buffer_head
#undef rx_buffer_tail
#undef rx_buffer_overflow
#undef rx_buffer

#endif  // tsconfigRX_PINS & tsconfigPCBIT2

33:

#if tsconfigRX_PINS & tsconfigPCBIT3

#define rx_state				tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,3_),rx_state)
#define rx_byte					tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,3_),rx_byte)
#define	rx_bit					tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,3_),rx_bit)
#define rx_target				tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,3_),rx_target)
#define rx_buffer_head			tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,3_),rx_buffer_head)
#define rx_buffer_tail			tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,3_),rx_buffer_tail)
#define rx_buffer_overflow		tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,3_),rx_buffer_overflow)
#define rx_buffer				tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,3_),rx_buffer)

	sbrs	DIFF, 3					; Did pin change?
	rjmp	44f

	ldi		TEMP, 0x80
	lds		BIT, rx_bit
	eor		BIT, TEMP				; Next bit we are expecting

	lds		STATE, rx_state

	; ------------------------------------------------------------------------
	; Handle start bit (state == 0)
	tst		STATE
	brne	03f

	sbrc	LEVELS,3 				; Verify that RX level is zero
	rjmp	10f

									; Counter value half way into next bit
	X_ldsiw	TARGET, ticks_per_bit_and_half, tsBITS, tsIMMEDIATE
	X_addw	TARGET, CAPTURE, tsBITS
	inc		STATE					; Change state for first data bit (LSB)
	rjmp	8f

	; ------------------------------------------------------------------------
	; Handle data and stop bits
	; Should be half way into bit
03:	X_ldsw	TARGET, rx_target, tsBITS
	lds		BYTE, rx_byte
	X_ldsiw	TEMP2, ticks_per_bit, tsBITS, tsIMMEDIATE
	.if tsBITS == 8
	X_ldsiw	TEMP3, ticks_per_half_bit, tsBITS, tsIMMEDIATE
	.endif

04:									; Loop to count number of bits received
	X_movw	TEMP, CAPTURE, tsBITS	; ticks_per_half_bit < (CAPTURE - TARGET)
	X_subw	TEMP, TARGET, tsBITS
	.if tsBITS == 8
	cp		TEMP3, TEMP
	brlo	07f						; Not enough time for a bit
	.else
	brmi	07f						; Not enough time for a bit
	.endif
	lsr		BYTE
	or		BYTE, BIT				; Bits received LSB first
									; Add time for bit
	X_addw	TARGET, TEMP2, tsBITS
	inc		STATE					; State is current bit waiting for

	cpi		STATE, 9				; Stop bit is state 9 (10th bit)
	brlo	04b						; Loop to see if next bit has arrived

	; ------------------------------------------------------------------------
	; Place byte into RX buffer
	lds		HEAD, rx_buffer_head
	inc		HEAD
	.if RX_BUFFER_MASK <> 255
	andi	HEAD, RX_BUFFER_MASK
	.endif

	lds		TEMP, rx_buffer_tail
	cp		HEAD, TEMP
	breq	05f
	mov		XL, HEAD
	clr		XH
	subi	XL, lo8(-(rx_buffer))	; Offset into buffer
	sbci	XH, hi8(-(rx_buffer))
	st		X, BYTE
	sts		rx_buffer_head, HEAD
	rjmp	06f
05: ldi		TEMP, 1					; Set overflow flag
	sts		rx_buffer_overflow, TEMP

06:	ldi		BIT, 0x00				; Next bit will be zero
	ldi		STATE, 0				; State zero is for start bit (0)

	; ------------------------------------------------------------------------
	; Save current state for next interrupt
07:	sts		rx_byte, BYTE

08:	X_stsw	rx_target, TARGET, tsBITS
	sts		rx_state, STATE

09:	sts		rx_bit, BIT				; Save next expected bit

#undef rx_state
#undef rx_byte
#undef rx_bit
#undef rx_target
#undef rx_buffer_head
#undef rx_buffer_tail
#undef rx_buffer_overflow
#undef rx_buffer

#endif  // tsconfigRX_PINS & tsconfigPCBIT3

44:

#if tsconfigRX_PINS & tsconfigPCBIT4

#define rx_state				tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,4_),rx_state)
#define rx_byte					tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,4_),rx_byte)
#define	rx_bit					tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,4_),rx_bit)
#define rx_target				tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,4_),rx_target)
#define rx_buffer_head			tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,4_),rx_buffer_head)
#define rx_buffer_tail			tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,4_),rx_buffer_tail)
#define rx_buffer_overflow		tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,4_),rx_buffer_overflow)
#define rx_buffer				tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,4_),rx_buffer)

	sbrs	DIFF, 4					; Did pin change?
	rjmp	55f

	ldi		TEMP, 0x80
	lds		BIT, rx_bit
	eor		BIT, TEMP				; Next bit we are expecting

	lds		STATE, rx_state

	; ------------------------------------------------------------------------
	; Handle start bit (state == 0)
	tst		STATE
	brne	03f

	sbrc	LEVELS, 4				; Verify that RX level is zero
	rjmp	10f

									; Counter value half way into next bit
	X_ldsiw	TARGET, ticks_per_bit_and_half, tsBITS, tsIMMEDIATE
	X_addw	TARGET, CAPTURE, tsBITS
	inc		STATE					; Change state for first data bit (LSB)
	rjmp	8f

	; ------------------------------------------------------------------------
	; Handle data and stop bits
	; Should be half way into bit
03:	X_ldsw	TARGET, rx_target, tsBITS
	lds		BYTE, rx_byte
	X_ldsiw	TEMP2, ticks_per_bit, tsBITS, tsIMMEDIATE
	.if tsBITS == 8
	X_ldsiw	TEMP3, ticks_per_half_bit, tsBITS, tsIMMEDIATE
	.endif

04:									; Loop to count number of bits received
	X_movw	TEMP, CAPTURE, tsBITS	; ticks_per_half_bit < (CAPTURE - TARGET)
	X_subw	TEMP, TARGET, tsBITS
	.if tsBITS == 8
	cp		TEMP3, TEMP
	brlo	07f						; Not enough time for a bit
	.else
	brmi	07f						; Not enough time for a bit
	.endif
	lsr		BYTE
	or		BYTE, BIT				; Bits received LSB first
									; Add time for bit
	X_addw	TARGET, TEMP2, tsBITS
	inc		STATE					; State is current bit waiting for

	cpi		STATE, 9				; Stop bit is state 9 (10th bit)
	brlo	04b						; Loop to see if next bit has arrived

	; ------------------------------------------------------------------------
	; Place byte into RX buffer
	lds		HEAD, rx_buffer_head
	inc		HEAD
	.if RX_BUFFER_MASK <> 255
	andi	HEAD, RX_BUFFER_MASK
	.endif

	lds		TEMP, rx_buffer_tail
	cp		HEAD, TEMP
	breq	05f
	mov		XL, HEAD
	clr		XH
	subi	XL, lo8(-(rx_buffer))	; Offset into buffer
	sbci	XH, hi8(-(rx_buffer))
	st		X, BYTE
	sts		rx_buffer_head, HEAD
	rjmp	06f
05: ldi		TEMP, 1					; Set overflow flag
	sts		rx_buffer_overflow, TEMP

06:	ldi		BIT, 0x00				; Next bit will be zero
	ldi		STATE, 0				; State zero is for start bit (0)

	; ------------------------------------------------------------------------
	; Save current state for next interrupt
07:	sts		rx_byte, BYTE

08:	X_stsw	rx_target, TARGET, tsBITS
	sts		rx_state, STATE

09:	sts		rx_bit, BIT				; Save next expected bit

#undef rx_state
#undef rx_byte
#undef rx_bit
#undef rx_target
#undef rx_buffer_head
#undef rx_buffer_tail
#undef rx_buffer_overflow
#undef rx_buffer

#endif  // tsconfigRX_PINS & tsconfigPCBIT4

55:

#if tsconfigRX_PINS & tsconfigPCBIT6

#define rx_state				tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,6_),rx_state)
#define rx_byte					tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,6_),rx_byte)
#define	rx_bit					tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,6_),rx_bit)
#define rx_target				tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,6_),rx_target)
#define rx_buffer_head			tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,6_),rx_buffer_head)
#define rx_buffer_tail			tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,6_),rx_buffer_tail)
#define rx_buffer_overflow		tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,6_),rx_buffer_overflow)
#define rx_buffer				tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,6_),rx_buffer)

	sbrs	DIFF, 6					; Did pin change?
	rjmp	77f

	ldi		TEMP, 0x80
	lds		BIT, rx_bit
	eor		BIT, TEMP				; Next bit we are expecting

	lds		STATE, rx_state

	; ------------------------------------------------------------------------
	; Handle start bit (state == 0)
	tst		STATE
	brne	03f

	sbrc	LEVELS, 6				; Verify that RX level is zero
	rjmp	10f

									; Counter value half way into next bit
	X_ldsiw	TARGET, ticks_per_bit_and_half, tsBITS, tsIMMEDIATE
	X_addw	TARGET, CAPTURE, tsBITS
	inc		STATE					; Change state for first data bit (LSB)
	rjmp	8f

	; ------------------------------------------------------------------------
	; Handle data and stop bits
	; Should be half way into bit
03:	X_ldsw	TARGET, rx_target, tsBITS
	lds		BYTE, rx_byte
	X_ldsiw	TEMP2, ticks_per_bit, tsBITS, tsIMMEDIATE
	.if tsBITS == 8
	X_ldsiw	TEMP3, ticks_per_half_bit, tsBITS, tsIMMEDIATE
	.endif

04:									; Loop to count number of bits received
	X_movw	TEMP, CAPTURE, tsBITS	; ticks_per_half_bit < (CAPTURE - TARGET)
	X_subw	TEMP, TARGET, tsBITS
	.if tsBITS == 8
	cp		TEMP3, TEMP
	brlo	07f						; Not enough time for a bit
	.else
	brmi	07f						; Not enough time for a bit
	.endif
	lsr		BYTE
	or		BYTE, BIT				; Bits received LSB first
									; Add time for bit
	X_addw	TARGET, TEMP2, tsBITS
	inc		STATE					; State is current bit waiting for

	cpi		STATE, 9				; Stop bit is state 9 (10th bit)
	brlo	04b						; Loop to see if next bit has arrived

	; ------------------------------------------------------------------------
	; Place byte into RX buffer
	lds		HEAD, rx_buffer_head
	inc		HEAD
	.if RX_BUFFER_MASK <> 255
	andi	HEAD, RX_BUFFER_MASK
	.endif

	lds		TEMP, rx_buffer_tail
	cp		HEAD, TEMP
	breq	05f
	mov		XL, HEAD
	clr		XH
	subi	XL, lo8(-(rx_buffer))	; Offset into buffer
	sbci	XH, hi8(-(rx_buffer))
	st		X, BYTE
	sts		rx_buffer_head, HEAD
	rjmp	06f
05: ldi		TEMP, 1					; Set overflow flag
	sts		rx_buffer_overflow, TEMP

06:	ldi		BIT, 0x00				; Next bit will be zero
	ldi		STATE, 0				; State zero is for start bit (0)

	; ------------------------------------------------------------------------
	; Save current state for next interrupt
07:	sts		rx_byte, BYTE

08:	X_stsw	rx_target, TARGET, tsBITS
	sts		rx_state, STATE

09:	sts		rx_bit, BIT				; Save next expected bit

#undef rx_state
#undef rx_byte
#undef rx_bit
#undef rx_target
#undef rx_buffer_head
#undef rx_buffer_tail
#undef rx_buffer_overflow
#undef rx_buffer

#endif  // tsconfigRX_PINS & tsconfigPCBIT6

77:

#if tsconfigRX_PINS & tsconfigPCBIT7

#define rx_state				tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,7_),rx_state)
#define rx_byte					tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,7_),rx_byte)
#define	rx_bit					tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,7_),rx_bit)
#define rx_target				tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,7_),rx_target)
#define rx_buffer_head			tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,7_),rx_buffer_head)
#define rx_buffer_tail			tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,7_),rx_buffer_tail)
#define rx_buffer_overflow		tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,7_),rx_buffer_overflow)
#define rx_buffer				tsMAKE_VARNAME(tsMAKE_VARNAME(tsVAR_RX_PCPORT,7_),rx_buffer)

	sbrs	DIFF, 7					; Did pin change?
	rjmp	10f

	ldi		TEMP, 0x80
	lds		BIT, rx_bit
	eor		BIT, TEMP				; Next bit we are expecting

	lds		STATE, rx_state

	; ------------------------------------------------------------------------
	; Handle start bit (state == 0)
	tst		STATE
	brne	03f

	sbrc	LEVELS, 7				; Verify that RX level is zero
	rjmp	10f

									; Counter value half way into next bit
	X_ldsiw	TARGET, ticks_per_bit_and_half, tsBITS, tsIMMEDIATE
	X_addw	TARGET, CAPTURE, tsBITS
	inc		STATE					; Change state for first data bit (LSB)
	rjmp	8f

	; ------------------------------------------------------------------------
	; Handle data and stop bits
	; Should be half way into bit
03:	X_ldsw	TARGET, rx_target, tsBITS
	lds		BYTE, rx_byte
	X_ldsiw	TEMP2, ticks_per_bit, tsBITS, tsIMMEDIATE
	.if tsBITS == 8
	X_ldsiw	TEMP3, ticks_per_half_bit, tsBITS, tsIMMEDIATE
	.endif

04:									; Loop to count number of bits received
	X_movw	TEMP, CAPTURE, tsBITS	; ticks_per_half_bit < (CAPTURE - TARGET)
	X_subw	TEMP, TARGET, tsBITS
	.if tsBITS == 8
	cp		TEMP3, TEMP
	brlo	07f						; Not enough time for a bit
	.else
	brmi	07f						; Not enough time for a bit
	.endif
	lsr		BYTE
	or		BYTE, BIT				; Bits received LSB first
									; Add time for bit
	X_addw	TARGET, TEMP2, tsBITS
	inc		STATE					; State is current bit waiting for

	cpi		STATE, 9				; Stop bit is state 9 (10th bit)
	brlo	04b						; Loop to see if next bit has arrived

	; ------------------------------------------------------------------------
	; Place byte into RX buffer
	lds		HEAD, rx_buffer_head
	inc		HEAD
	.if RX_BUFFER_MASK <> 255
	andi	HEAD, RX_BUFFER_MASK
	.endif

	lds		TEMP, rx_buffer_tail
	cp		HEAD, TEMP
	breq	05f
	mov		XL, HEAD
	clr		XH
	subi	XL, lo8(-(rx_buffer))	; Offset into buffer
	sbci	XH, hi8(-(rx_buffer))
	st		X, BYTE
	sts		rx_buffer_head, HEAD
	rjmp	06f
05: ldi		TEMP, 1					; Set overflow flag
	sts		rx_buffer_overflow, TEMP

06:	ldi		BIT, 0x00				; Next bit will be zero
	ldi		STATE, 0				; State zero is for start bit (0)

	; ------------------------------------------------------------------------
	; Save current state for next interrupt
07:	sts		rx_byte, BYTE

08:	X_stsw	rx_target, TARGET, tsBITS
	sts		rx_state, STATE

09:	sts		rx_bit, BIT				; Save next expected bit

#undef rx_state
#undef rx_byte
#undef rx_bit
#undef rx_target
#undef rx_buffer_head
#undef rx_buffer_tail
#undef rx_buffer_overflow
#undef rx_buffer

#endif  // tsconfigRX_PINS & tsconfigPCBIT7

10:	X_out	SREG, HOLD_SREG
	pop		DIFF
	.if tsBITS == 16
	pop		TEMPH
	pop		TARGETH
	pop		CAPTUREH
	.else
	pop		TEMP3
	.endif
	pop		XH
	pop		XL
	pop		TEMP
	pop		TARGET
	pop		LEVELS
	pop		BIT
	pop		BYTE
	pop		STATE
	pop		CAPTURE
	pop		HOLD_SREG
;#warning "Fix ASCII RX IR"
;cbi	0x08, 0
	reti

.endfunc							; tsRX_IR

#undef	HOLD_SREG
#undef	HEAD
#undef	CAPTURE
#undef	CAPTUREH
#undef	STATE
#undef	BYTE
#undef	BIT
#undef	TARGET
#undef	TARGETH
#undef	TEMP
#undef	TEMPH
#undef	TEMP2
#undef	TEMP3
#undef	TEMP2H
#undef	TEMP3
#undef	DIFF

#endif // !(defined(tsconfigRX_PINS) && (((tsconfigRX_PINS-1) & tsconfigRX_PINS) > 0))
