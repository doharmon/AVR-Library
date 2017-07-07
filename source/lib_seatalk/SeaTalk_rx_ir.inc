;---------------------------------------------------------------------------;
; Timer based SeaTalk software serial ports RX routines
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

.if RX_BUFFER_SIZE > 256
	.error "RX_BUFFER_SIZE must be 256 or less"
.endif
.if RX_BUFFER_SIZE & RX_BUFFER_MASK
	.error "RTX_BUFFER_SIZE must be a power of 2"
.endif

.section stsBSS_SECTIONNAME

#ifdef stconfigRX_PINS
	#if stconfigRX & stconfigPCPORTA
		#define stsVAR_RX_SUFFIX			stsMAKE_VARNAME(stsMAKE_VARNAME(pa,stconfigPCBIT),_)
	#elif stconfigRX & stconfigPCPORTB
		#define stsVAR_RX_SUFFIX			stsMAKE_VARNAME(stsMAKE_VARNAME(pb,stconfigPCBIT),_)
	#elif stconfigRX & stconfigPCPORTC
		#define stsVAR_RX_SUFFIX			stsMAKE_VARNAME(stsMAKE_VARNAME(pc,stconfigPCBIT),_)
	#elif stconfigRX & stconfigPCPORTD
		#define stsVAR_RX_SUFFIX			stsMAKE_VARNAME(stsMAKE_VARNAME(pd,stconfigPCBIT),_)
	#endif
	#define stsVAR_RX_PREFIX	stsMAKE_VARNAME(stsVAR_TIMER_PREFIX,stsVAR_RX_SUFFIX)
#else
	#define stsVAR_RX_PREFIX	stsMAKE_REGNAME(stsVAR_RX_PREFIX,stsREG_RX_SUFFIX)
#endif


#define rx_state				stsMAKE_VARNAME(stsVAR_RX_PREFIX,rx_state)
#define rx_byte					stsMAKE_VARNAME(stsVAR_RX_PREFIX,rx_byte)
#define	rx_bit					stsMAKE_VARNAME(stsVAR_RX_PREFIX,rx_bit)
#define rx_target				stsMAKE_VARNAME(stsVAR_RX_PREFIX,rx_target)
#define rx_buffer_head			stsMAKE_VARNAME(stsVAR_RX_PREFIX,rx_buffer_head)
#define rx_buffer_tail			stsMAKE_VARNAME(stsVAR_RX_PREFIX,rx_buffer_tail)
#define rx_buffer_overflow		stsMAKE_VARNAME(stsVAR_RX_PREFIX,rx_buffer_overflow)
#define rx_buffer				stsMAKE_VARNAME(stsVAR_RX_PREFIX,rx_buffer)

; SeaTalk specific
#define rx_command				stsMAKE_VARNAME(stsVAR_RX_PREFIX,rx_command)
#define rx_count				stsMAKE_VARNAME(stsVAR_RX_PREFIX,rx_count)

; Space for rx_stop_ticks is allocated in SeaTalk_timer.inc
#define rx_stop_ticks			stsMAKE_VARNAME(stsVAR_TIMER_PREFIX,_rx_stop_ticks)

#define stsRXISC0				stsMAKE_REGNAME(stsRXISC0,stsREG_INT_SUFFIX)
#define stsRXISC1				stsMAKE_REGNAME(stsRXISC1,stsREG_INT_SUFFIX)

#if stconfigRX & stconfigICP_MASK
#define stsRXICES				stsMAKE_REGNAME(stsRXICESICP,stsREG_ICP_SUFFIX)
#define stsRXEICRA				stsRXEICRAICP
#endif

#if stconfigRX & stconfigPCPORT_MASK
#define rx_prev					stsMAKE_VARNAME(stsVAR_RX_PREFIX,rx_prev)
.global rx_prev

.section stsDATA_SECTIONNAME
rx_prev:	.byte	0xFF
.section stsBSS_SECTIONNAME
#endif

;-----------------------------------------------------------------------------
; RX Interrupt Routine
;
; This routine uses pin change interrupt ICPX, INTX, or a single PCINT pin
; to get level change.
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
.section stsTEXT_SECTIONNAME

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
#define	TEMP3H		r29
#define	DIFF		r29

.global stsRX_IR
.func stsRX_IR
stsRX_IR:
;#warning "Fix RX IR"
;sbi	0x02, 1
	push	HOLD_SREG
	push	CAPTURE
	push	STATE
	push	BYTE
	push 	BIT
	push	TARGET
	push	TEMP
	push	XL
	push	XH
	push	TEMP3
	.if stsBITS == 16
	push	CAPTUREH
	push	TARGETH
	push	TEMPH
	.endif
	#if (stconfigRX & stconfigPCPORT_MASK) || (stsBITS == 16)
	push	DIFF
	#endif
	X_in	HOLD_SREG, SREG

	; ------------------------------------------------------------------------
	; Counter when RX level changed
	X_inw	CAPTURE, stsRXTCNT, stsBITS

	#if stconfigRX & stconfigPCPORT_MASK
	; ------------------------------------------------------------------------
	; Find out which pins changed
	lds		DIFF, rx_prev			; Get previous pin states
	X_in	TEMP2, stsRXINPUT		; Get current pin states
	sts		rx_prev, TEMP2			; Save current states to previous
	eor		DIFF, TEMP2				; Changed pins = prev xor current

	ldi		TEMP, stconfigRX_PINS	; Get pins we are watching
	and		DIFF, TEMP				; Did one of our pins change?
	brne	10f
	rjmp	63f

10:	ldi		TEMP, 0x80
	lds		BIT, rx_bit
	eor		BIT, TEMP				; Next bit we are expecting

	#else // stconfigRX & stconfigPCPORT_MASK
	; ------------------------------------------------------------------------
	; Set the next rising/falling edge to capture depending on current bit
	X_in	TEMP, stsRXEICRA
	lds		BIT, rx_bit
	sbrs	BIT, 7
	rjmp	20f
									; Capture falling edge
	#if stconfigRX & stconfigICP_MASK
	andi	TEMP, ~(1<<stsRXICES)	; Input Capture Pin
	#else
	andi	TEMP, ~(1<<stsRXISC0)	; External Interrupt Pin
	ori		TEMP, 1<<stsRXISC1
	#endif
	ldi		BIT, 0x00				; Next bit will be zero
	rjmp	21f
20:									; Capture rising edge
	#ifdef stsRXICES
	ori		TEMP, (1<<stsRXICES)	; Input Capture Pin
	#else
									; External Interrupt Pin
	ori		TEMP, (1<<stsRXISC1) | (1<<stsRXISC0)
	#endif
	ldi		BIT, 0x80				; Next bit will be one
21:	X_out	stsRXEICRA, TEMP

	#endif // stconfigRX & stconfigPCPORT_MASK

	lds		STATE, rx_state

	; ------------------------------------------------------------------------
	; Handle start bit (state == 0)
	tst		STATE
	brne	40f

	#if stconfigRX & stconfigPCPORT_MASK

	andi	TEMP2, stconfigRX_PINS	; Verify that RX level is zero
	breq	30f
	rjmp	63f

	#else // stconfigRX & stconfigPCPORT_MASK

	sbrs	BIT, 7
	rjmp	62f						; Current bit should be zero for start bit

	#endif // stconfigRX & stconfigPCPORT_MASK

30:
	X_ldsiw	TEMP, rx_stop_ticks, stsBITS, stsIMMEDIATE
	X_addw	TEMP, CAPTURE, stsBITS	; Timer value when stop bit should arrive
	X_outw	stsRXOCR, TEMP, stsBITS

	ldi		TEMP, 1<<stsRXOCF		; Enable stop bit interrupt that handles
	X_out	stsRXTIFR, TEMP			; when the MSB of the data byte is one.
	X_in	TEMP, stsTIMSK			; Since the stop bit is also one, there
	ori		TEMP, 1<<stsRXOCIE		; will be no rising edge to trigger this
	X_out	stsTIMSK, TEMP			; interrupt.

									; Counter value half way into next bit
	X_ldsiw	TARGET, ticks_per_bit_and_half, stsBITS, stsIMMEDIATE
	X_addw	TARGET, CAPTURE, stsBITS
	inc		STATE					; Change state for first data bit (LSB)
	rjmp	61f

	; ------------------------------------------------------------------------
	; Handle data and stop bits
	; Should be half way into bit
40:
	X_ldsw	TARGET, rx_target, stsBITS
	lds		BYTE, rx_byte
	X_ldsiw	TEMP2, ticks_per_bit, stsBITS, stsIMMEDIATE
	X_ldsiw	TEMP3, ticks_per_half_bit, stsBITS, stsIMMEDIATE

41:									; Loop to count number of bits received
	X_movw	TEMP, CAPTURE, stsBITS	; Check if enough time for a bit
	X_subw	TEMP, TARGET, stsBITS
	X_cpw	TEMP3, TEMP, stsBITS
	brsh	42f						; ticks_per_half_bit >= (CAPTURE - TARGET)
	rjmp	60f						; Not enough time for a bit

42:	cpi		STATE, 9				; Ninth bit is command bit
	brne	43f
	sts		rx_command, BIT
	rjmp	44f
43:	lsr		BYTE
	or		BYTE, BIT				; Bits received LSB first

44:	X_addw	TARGET, TEMP2, stsBITS	; Add time for bit
	inc		STATE					; State is current bit waiting for

	cpi		STATE, 10				; Stop bit is state 10 (11th bit)
	brlo	41b						; Loop to see if next bit has arrived

	X_in	TEMP, stsTIMSK			; Have a stop bit so disable stop bit
	andi	TEMP, ~(1<<stsRXOCIE)	; interrupt.
	X_out	stsTIMSK, TEMP

	; ------------------------------------------------------------------------
	; Place byte into RX buffer
	lds		HEAD, rx_buffer_head
	inc		HEAD
	.if RX_BUFFER_MASK <> 255
	andi	HEAD, RX_BUFFER_MASK
	.endif

	lds		TEMP, rx_buffer_tail
	cp		HEAD, TEMP
	breq	50f
	mov		XL, HEAD
	clr		XH
	lsl		XL
	.if		RX_BUFFER_SIZE > 127
	rol		XH
	.endif
	subi	XL, lo8(-(rx_buffer))	; Offset into buffer
	sbci	XH, hi8(-(rx_buffer))
	st		X+, BYTE
	lds		TEMP, rx_command
	st		X, TEMP
	sts		rx_buffer_head, HEAD
	rjmp	51f
50: ldi		TEMP, 1					; Set overflow flag
	sts		rx_buffer_overflow, TEMP

51:
	#if !(stconfigRX & stconfigPCPORT_MASK)

	X_in	TEMP, stsRXEICRA		; Prepare for next start bit
									; Capture falling edge
	#ifdef stsRXICES
	andi	TEMP, ~(1<<stsRXICES)	; Input Capture Pin
	#else
	andi	TEMP, ~(1<<stsRXISC0)	; External Interrupt Pin
	ori		TEMP, 1<<stsRXISC1
	#endif
	X_out	stsRXEICRA, TEMP

	#endif // !(stconfigRX & stconfigPCPORT_MASK)

	ldi		BIT, 0x00				; Next bit will be zero
	ldi		STATE, 0				; State zero is for start bit (0)
	lds		TEMP, timer_millis
	sts		rx_count, TEMP
	lds		TEMP, timer_millis+1
	sts		rx_count+1, TEMP
	lds		TEMP, timer_millis+2
	sts		rx_count+2, TEMP
	lds		TEMP, timer_millis+3
	sts		rx_count+3, TEMP

	; ------------------------------------------------------------------------
	; Save current state for next interrupt
60:	sts		rx_byte, BYTE

61:	X_stsw	rx_target, TARGET, stsBITS
	sts		rx_state, STATE

62:	sts		rx_bit, BIT				; Save next expected bit

63:
	X_out	SREG, HOLD_SREG

	#if (stconfigRX & stconfigPCPORT_MASK) || (stsBITS == 16)
	pop		DIFF
	#endif
	.if stsBITS == 16
	pop		TEMPH
	pop		TARGETH
	pop		CAPTUREH
	.endif
	pop		TEMP3
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
;cbi	0x02, 1
	reti

.endfunc							; stsRX_IR

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
#undef	TEMP2H
#undef	TEMP3
#undef	TEMP3H
#undef	DIFF

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
;		r16		HEAD
;		r19		BYTE
;		r26		XL
;		r27		XH		or	TEMP
;-----------------------------------------------------------------------------
.section stsTEXT_SECTIONNAME

; These are undefined at the bottom of the function
#define	HOLD_SREG	r15
#define	HEAD		r16
#define	BYTE		r19
#define	TEMP		r27

.global stsRX_SB_IR
.func stsRX_SB_IR
stsRX_SB_IR:
;#warning "Fix RX Stop Bit IR"
;sbi	0x02, 2
	push	HOLD_SREG
	push	HEAD
	push	BYTE
	push	XL
	push	XH
	X_in	HOLD_SREG, SREG

	X_in	TEMP, stsTIMSK			; Disable this interrupt
	andi	TEMP, ~(1<<stsRXOCIE)
	X_out	stsTIMSK, TEMP

	#if !(stconfigRX & stconfigPCPORT_MASK)

	X_in	TEMP, stsRXEICRA		; Prepare for next start bit
									; Capture falling edge
	#ifdef stsRXICES
	andi	TEMP, ~(1<<stsRXICES)	; Input Capture Pin
	#else
	andi	TEMP, ~(1<<stsRXISC0)	; External Interrupt Pin
	ori		TEMP, 1<<stsRXISC1
	#endif

	X_out	stsRXEICRA, TEMP

	#endif // !(stconfigRX & stconfigPCPORT_MASK)

	; ------------------------------------------------------------------------
	; Count remaining bits
	lds		BYTE, rx_byte
	lds		HEAD, rx_state			; Calc bits left to shift
	ldi		TEMP, 9
	sub		TEMP, HEAD
	breq	03f

	sec								; Set MSB to 1
	ror		BYTE
	rjmp	02f
01:	asr		BYTE
02:	dec		TEMP
	brne	01b

	; ------------------------------------------------------------------------
	; Place byte into RX buffer
03:
	lds		HEAD, rx_buffer_head	; Get offset into buffer
	inc		HEAD
	.if RX_BUFFER_MASK <> 255
	andi	HEAD, RX_BUFFER_MASK
	.endif

	lds		TEMP, rx_buffer_tail
	cp		HEAD, TEMP
	breq	04f
	mov		XL, HEAD
	clr		XH
	lsl		XL
	.if RX_BUFFER_SIZE > 127
	rol		XH
	.endif
	subi	XL, lo8(-(rx_buffer))	; Offset into buffer
	sbci	XH, hi8(-(rx_buffer))
	st		X+, BYTE
	sts		rx_byte, BYTE			; Used by TX IR to compare with byte sent
	ldi		BYTE, 1					; Command bit will always be a 1
	st		X, BYTE
	sts		rx_buffer_head, HEAD
	rjmp	05f
04: ldi		TEMP, 1					; Set overflow flag
	sts		rx_buffer_overflow, TEMP

05:	clr		TEMP
	sts		rx_state, TEMP			; State 0 for start bit (0)
	sts		rx_bit, TEMP			; Start bit is zero
	lds		TEMP, timer_millis
	sts		rx_count, TEMP
	lds		TEMP, timer_millis+1
	sts		rx_count+1, TEMP
	lds		TEMP, timer_millis+2
	sts		rx_count+2, TEMP
	lds		TEMP, timer_millis+3
	sts		rx_count+3, TEMP

	X_out	SREG, HOLD_SREG
	pop		XH
	pop		XL
	pop		BYTE
	pop		HEAD
	pop		HOLD_SREG
;#warning "Fix RX Stop Bit IR"
;cbi	0x02, 2
	reti

.endfunc							; stsRX_SB_IR

#undef	HOLD_SREG
#undef	HEAD
#undef	BYTE
#undef	TEMP
#undef	stsVAR_RX_PREFIX
