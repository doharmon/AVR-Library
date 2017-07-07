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

#define stsRXISC0				stsMAKE_REGNAME(stsRXISC0,stsREG_INT_SUFFIX)
#define stsRXISC1				stsMAKE_REGNAME(stsRXISC1,stsREG_INT_SUFFIX)

#if stconfigRX & stconfigICP_MASK
#define stsRXICES				stsMAKE_REGNAME(stsRXICESICP,stsREG_ICP_SUFFIX)
#define stsRXEICRA				stsRXEICRAICP
#endif

.global rx_state
.global rx_byte
.global	rx_bit
.global rx_target
.global rx_buffer_head
.global rx_buffer_tail
.global rx_buffer_overflow
.global rx_buffer
.global rx_command
.global rx_count

rx_state:				.ds.b	1
rx_byte:				.ds.b	1
rx_bit:					.ds.b	1
rx_target:				stsDS	1
rx_buffer_head:			.ds.b	1
rx_buffer_tail:			.ds.b	1
rx_buffer_overflow:		.ds.b	1
rx_buffer:				.ds.b	RX_BUFFER_SIZE*2
rx_command:				.ds.b	1

.section stsDATA_SECTIONNAME
rx_count:				.long	(0x00 - stconfigMIN_WAIT_TIME)
.section stsBSS_SECTIONNAME


;-----------------------------------------------------------------------------
; int XSerialRead(void)
;
; Return data byte from RX buffer or -1 if buffer is empty.
;
; Local Variables:
;		r24			Head of RX buffer
;		r25			Tail of RX buffer
;		r26			Command bit
;		r31:r30		Z-Index into RX buffer
;
; Return:
;		r25:r24		Byte or -1 if empty buffer
;-----------------------------------------------------------------------------
#define stsSERIALREAD stsMAKE_FUNCNAME(stsFUNC_PREFIX,SerialRead)
.section stsMAKE_SECTIONNAME(stsMAKE_SECTIONNAME(stsTEXT_SECTIONNAME,.),stsSERIALREAD)

.global	stsSERIALREAD
.func	stsSERIALREAD

stsSERIALREAD:
	lds		r24, rx_buffer_head
	lds		r25, rx_buffer_tail

	cp		r24, r25
	breq	01f

	inc		r25
	.if RX_BUFFER_MASK <> 255
	andi	r25, RX_BUFFER_MASK
	.endif

	; Read the buffer before updating rx_buffer_tail or RX interrupt
	; might over write the value.
	mov		ZL, r25
	clr		ZH
	lsl		ZL
	.if RX_BUFFER_SIZE > 127
	rol		ZH
	.endif
	subi	ZL, lo8(-(rx_buffer))	; Offset into buffer
	sbci	ZH, hi8(-(rx_buffer))
	ld		r24, Z+					; Get byte
	ld		r26, Z					; Get command bit
	sts		rx_buffer_tail, r25
	clr		r25
	cpse	r26, r1
	inc		r25
	ret

01:	ldi		r24, 0xFF				; Buffer is empty
	ldi		r25, 0xFF
	ret

.endfunc	; stsSERIALREAD

;-----------------------------------------------------------------------------
; int XSerialPeek(void)
;
; Return next byte in the receive buffer or -1 if empty.
;
; Local Variables:
;		r24			Head of RX buffer
;		r25			Tail of RX buffer
;		r26			Command bit
;		r31:r30		Z-Index into RX buffer
;
; Return:
;		r25:r24		Byte or -1 if empty buffer
;-----------------------------------------------------------------------------
#define stsSERIALPEEK stsMAKE_FUNCNAME(stsFUNC_PREFIX,SerialPeek)
.section stsMAKE_SECTIONNAME(stsMAKE_SECTIONNAME(stsTEXT_SECTIONNAME,.),stsSERIALPEEK)

.global	stsSERIALPEEK
.func	stsSERIALPEEK

stsSERIALPEEK:
	lds		r24, rx_buffer_head
	lds		r25, rx_buffer_tail

	cp		r24, r25
	breq	01f

	inc		r25
	.if RX_BUFFER_MASK <> 255
	andi	r25, RX_BUFFER_MASK
	.endif

	mov		ZL, r25
	clr		ZH
	lsl		ZL
	.if RX_BUFFER_SIZE > 127
	rol		ZH
	.endif
	subi	ZL, lo8(-(rx_buffer))	; Offset into buffer
	sbci	ZH, hi8(-(rx_buffer))
	ld		r24, Z+					; Get byte
	ld		r26, Z					; Get command bit
	clr		r25
	cpse	r26, r1
	inc		r25
	ret

01:	ldi		r24, 0xFF				; Buffer is empty
	ldi		r25, 0xFF
	ret

.endfunc	; stsSERIALPEEK

;-----------------------------------------------------------------------------
; int XSerialAvailable(void)
;
; Return number of bytes in the receive buffer.
;
; Local Variables:
;		r24			Head of RX buffer
;		r25			Tail of RX buffer
;		r31:r30		Z-Index into RX buffer
;
; Return:
;		r25:r24		Number of bytes in the receive buffer, 0xFF maximum
;-----------------------------------------------------------------------------
#define stsSERIALAVAILABLE stsMAKE_FUNCNAME(stsFUNC_PREFIX,SerialAvailable)
.section stsMAKE_SECTIONNAME(stsMAKE_SECTIONNAME(stsTEXT_SECTIONNAME,.),stsSERIALAVAILABLE)

.global	stsSERIALAVAILABLE
.func	stsSERIALAVAILABLE

stsSERIALAVAILABLE:
	lds		r24, rx_buffer_head
	lds		r25, rx_buffer_tail

	sub		r24, r25
	.if RX_BUFFER_MASK <> 255
	andi	r24, RX_BUFFER_MASK
	.endif
	clr		r25
	ret

.endfunc	; stsSERIALAVAILABLE

;-----------------------------------------------------------------------------
; void XSerialFlushInput(void)
;
; Set available bytes in the receive buffer to zero.
;
; Local Variables:
;		r25			Tail of RX buffer
;-----------------------------------------------------------------------------
#define stsSERIALFLUSHINPUT stsMAKE_FUNCNAME(stsFUNC_PREFIX,SerialFlushInput)
.section stsMAKE_SECTIONNAME(stsMAKE_SECTIONNAME(stsTEXT_SECTIONNAME,.),stsSERIALFLUSHINPUT)

.global	stsSERIALFLUSHINPUT
.func	stsSERIALFLUSHINPUT

stsSERIALFLUSHINPUT:
	lds		r25, rx_buffer_tail
	sts		rx_buffer_head, r25
	ret

.endfunc	; stsSERIALFLUSHINPUT

;-----------------------------------------------------------------------------
; int XSerialOverflow(void)
;
; Returns -1 if the receive buffer overflowed else 0.
;
; Local Variables:
;		r24			rx_buffer_overflow
;
; Return:
;		r25:r24		-1 if the receive buffer overflowed else 0
;-----------------------------------------------------------------------------
#define stsSERIALOVERFLOW stsMAKE_FUNCNAME(stsFUNC_PREFIX,SerialOverflow)
.section stsMAKE_SECTIONNAME(stsMAKE_SECTIONNAME(stsTEXT_SECTIONNAME,.),stsSERIALOVERFLOW)

.global	stsSERIALOVERFLOW
.func	stsSERIALOVERFLOW

stsSERIALOVERFLOW:
	lds		r24, rx_buffer_overflow
	tst		r24
	breq	00f
	ldi		r24, 0xFF
	ldi		r25, 0xFF
	ret
00:	clr		r25
	ret

.endfunc	; stsSERIALOVERFLOW

#undef stconfigRX
#undef stconfigPCBIT
#undef stsFUNC_PREFIX
