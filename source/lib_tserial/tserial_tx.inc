;---------------------------------------------------------------------------;
; Timer based software serial ports TX routines
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

.if TX_BUFFER_SIZE > 256
	.error "TX_BUFFER_SIZE must be 256 or less"
.endif
.if TX_BUFFER_SIZE & TX_BUFFER_MASK
	.error "TX_BUFFER_SIZE must be a power of 2"
.endif

; State machine states
#define tsTxIDLE		0
#define tsTxSTART		1
#define tsTxBIT0		2
#define tsTxBIT1		3
#define tsTxBIT2		4
#define tsTxBIT3		5
#define tsTxBIT4		6
#define tsTxBIT5		7
#define tsTxBIT6		8
#define tsTxBIT7		9
#define tsTxSTOP		10
#define tsTxCHKBUF		11

.section tsBSS_SECTIONNAME

#define tsTX_IR					tsMAKE_REGNAME(tsTX_IR,tsREG_TX_SUFFIX)
#define tsTXOCR					tsMAKE_REGNAME(tsTXOCR,tsREG_TX_SUFFIX)
#define tsTXOCF					tsMAKE_REGNAME(tsTXOCF,tsREG_TX_SUFFIX)
#define tsTXCOM0				tsMAKE_REGNAME(tsTXCOM0,tsREG_TX_SUFFIX)
#define tsTXCOM1				tsMAKE_REGNAME(tsTXCOM1,tsREG_TX_SUFFIX)
#define tsTXFOC					tsMAKE_REGNAME(tsTXFOC,tsREG_TX_SUFFIX)
#define tsTXOCIE				tsMAKE_REGNAME(tsTXOCIE,tsREG_TX_SUFFIX)
#define tsVAR_TX_PREFIX			tsMAKE_REGNAME(tsVAR_TX_PREFIX,tsREG_TX_SUFFIX)

#define tx_state				tsMAKE_VARNAME(tsVAR_TX_PREFIX,tx_state)
#define tx_byte					tsMAKE_VARNAME(tsVAR_TX_PREFIX,tx_byte)
#define tx_bit					tsMAKE_VARNAME(tsVAR_TX_PREFIX,tx_bit)
#define tx_target				tsMAKE_VARNAME(tsVAR_TX_PREFIX,tx_target)
#define tx_buffer_head			tsMAKE_VARNAME(tsVAR_TX_PREFIX,tx_buffer_head)
#define tx_buffer_tail			tsMAKE_VARNAME(tsVAR_TX_PREFIX,tx_buffer_tail)
#define tx_buffer				tsMAKE_VARNAME(tsVAR_TX_PREFIX,tx_buffer)

;The following globals are for debugging. Should be removed.
.global tx_state
.global tx_byte
.global tx_bit
.global tx_target
.global tx_buffer_head
.global tx_buffer_tail
.global tx_buffer

tx_state:				.ds.b	1
tx_byte:				.ds.b	1
tx_bit:					.ds.b	1
tx_target:				tsDS	1
tx_buffer_head:			.ds.b	1
tx_buffer_tail:			.ds.b	1
tx_buffer:				.ds.b	TX_BUFFER_SIZE

;-----------------------------------------------------------------------------
; TX Interrupt Routine
;
; State		Name		Description
;	0		tsTxIDLE	Idle: IR routine disabled
;	1		tsTxSTART	Add delay for start bit.
;  2-8		tsTxBIT0-6	Add delay for data bit.
;	9		tsTxBIT7	Add delay for data bit. OCR output high for stop bit.
;  10		tsTxSTOP	Add delay for stop bit.
;  11		tsTxCHKBUF	Check buffer for next byte.
;
; Data Byte: 0x9B 0b10011011
;
; Line Level:     1   0   1   1   0   1   1   0   0   1   1
; State:		Idl Strt B0  B1  B2  B3  B4  B5  B6  B7 Stop  
;				+---+   +---+---+   +---+---+       +---+---+
;   				|   |       |   |       |       |
;	    			|   |       |   |       |       |
;	    			+---+       +---+       +---+---+
;
;			   	|   |   |   |   |   |   |   |   |   |   |   |
;			   	|   |   |   |   |   |   |   |   |   |   |   +-- tsTxCHKBUF
;			   	|   |   |   |   |   |   |   |   |   |   +-- tsTxSTOP
;			   	|   |   |   |   |   |   |   |   |   +-- tsTxBIT7
;			   	|   |   |   |   |   |   |   |   +-- tsTxBIT6
;			   	|   |   |   |   |   |   |   +-- tsTxBIT5
;			   	|   |   |   |   |   |   +-- tsTxBIT4
;			   	|   |   |   |   |   +-- tsTxBIT3
;			   	|   |   |   |   +-- tsTxBIT2
;			   	|   |   |   +-- tsTxBIT1
;			   	|   |   +-- tsTxBIT0
;			   	|   +-- tsTxSTART
;			   	+-- tsTxIDLE
;
;	NOTE:	Data bits are sent LSB first.
;			Any line level changes happen before IR routine is called.
;
; Local Variables:
;		r15		HOLD_SREG
;		r16		STATE
;		r17		BYTE	or	TAIL
;		r20		TARGET(L)
;		r21		TARGETH
;		r24		DELAY
;		r25		DELAYH
;		r26		XL		or	TEMP(L)
;		r27		XH		or	TEMPH
;-----------------------------------------------------------------------------
.section tsTEXT_SECTIONNAME

; These are undefined at the bottom of the function
#define HOLD_SREG	r15
#define STATE		r16
#define BYTE		r17
#define TAIL		r17
#define TARGET		r20
#define TARGETH		r21
#define DELAY		r24
#define DELAYH		r25
#define TEMP		r26				// XL
#define	TEMPH		r27				// XH

.global tsTX_IR
.func tsTX_IR

tsTX_IR:
#warning "Fix TX IR"
sbi	DBGPORT, 0
	push	HOLD_SREG
	push	STATE
	push	BYTE
	push	TARGET
	push	DELAY
	push	XL
	push	XH
	.if tsBITS == 16
	push	TARGETH
	push	DELAYH
	.endif // tsBITS == 16
	X_in	HOLD_SREG, SREG

	lds		STATE, tx_state
	lds		BYTE, tx_byte
	X_ldsiw	DELAY, ticks_per_bit, tsBITS, tsIMMEDIATE

	; ------------------------------------------------------------------------
	; Jump to state handler
	cpi		STATE, tsTxSTART
	breq	10f						; tsTxSTART
	X_ldsw	TARGET, tx_target,tsBITS; Not used by tsTxCHKBUF
	cpi		STATE, tsTxSTOP
	brlo	20f						; tsTxBIT0-7
	brne	30f						; tsTxCHKBUF
	rjmp	40f						; tsTxSTOP

	; ------------------------------------------------------------------------
	; Handle tsTxSTART
	; Timing is based off of counter at start of start bit.
10:	X_inw	TARGET, tsTXOCR, tsBITS
	X_addw	TARGET, DELAY, tsBITS	; Add delay for start bit
	inc		STATE
	lsr		BYTE
	ori		BYTE, 0x80				; Load stop bit into MSB of TX byte
	brcc	21f						; LSB is 0
	rjmp	22f						; LSB is 1

	; ------------------------------------------------------------------------
	; Handle data bits 0-7
20:	lds		TEMP, tx_bit
	sbrc	TEMP, 0
	rjmp	23f

									; Current bit is 0
21:	X_addw	TARGET, DELAY, tsBITS	; Loop checks if next bits are same as the
	inc		STATE					; current bit being sent.
	lsr		BYTE
	brcc	21b						; Bottom of loop
22:	ldi		TEMP, 1
	sts		tx_bit, TEMP			; Next bit is a 1
	X_in	TEMP, tsTXTCCRA			; Raise level on next timeout
	ori		TEMP, (1<<tsTXCOM1) | (1<<tsTXCOM0)
	X_out	tsTXTCCRA, TEMP
	sts		tx_byte, BYTE
	rjmp	41f						; Exit

									; Current bit is 1
23:	X_addw	TARGET, DELAY, tsBITS	; Loop checks if next bits are same as the
	inc		STATE					; current bit being sent.
	lsr		BYTE
	brcs	23b						; Bottom of loop
	#if !(tsTX & tsconfigASCII)
	cpi		STATE, tsTxSTOP
	brsh	41f
	#endif
	ldi		TEMP, 0
	sts		tx_bit, TEMP			; Next bit is a 0
	X_in	TEMP, tsTXTCCRA			; Lower level on next timeout
	ori		TEMP, (1<<tsTXCOM1)
	andi	TEMP, ~(1<<tsTXCOM0)
	X_out	tsTXTCCRA, TEMP
	sts		tx_byte, BYTE
	rjmp	41f						; Exit

	; ------------------------------------------------------------------------
	; Handle tsTxCHKBUF, check if more bytes in TX buffer
30:	clr		STATE					; Set state to tsTxIDLE
	lds		TEMP, tx_buffer_head
	lds		TAIL, tx_buffer_tail
	cp		TEMP, TAIL
	brne	31f

	X_in	TEMP, tsTIMSK			; TX buffer empty
	andi	TEMP, ~(1<<tsTXOCIE)
	X_out	tsTIMSK, TEMP			; Disable INT compare TX
	rjmp	42f						; Exit

									; Ready next byte to transmit
31:	mov		XL, TAIL				; X = Index into tx buffer
	clr		XH
	inc		TAIL					; Move tail to next byte in array
	.if TX_BUFFER_MASK <> 255
	andi	TAIL, TX_BUFFER_MASK
	.endif
	sts		tx_buffer_tail, TAIL
	subi	XL, lo8(-(tx_buffer))	; Offset into buffer
	sbci	XH, hi8(-(tx_buffer))
	ld		BYTE, X					; Get next byte
	sts		tx_byte, BYTE
	X_in	TEMP, tsTXTCCRA
	ori		TEMP, (1<<tsTXCOM1)
	andi	TEMP, ~(1<<tsTXCOM0)
	X_out	tsTXTCCRA, TEMP			; Lower level after waiting
#warning "Look for better way to implement delay before start bit"
	X_inw	TARGET, tsTXTCNT, tsBITS
	X_ldsiw	DELAY, min_lead, tsBITS, tsIMMEDIATE
									; State set to tsTxSTART below

	; ------------------------------------------------------------------------
	; Exit
40:	inc		STATE					; Handle tsTxSTOP
	X_addw	TARGET, DELAY, tsBITS
41:	X_outw	tsTXOCR, TARGET, tsBITS
	X_stsw	tx_target, TARGET, tsBITS
42:	sts		tx_state, STATE

	X_out	SREG, HOLD_SREG
	.if tsBITS == 16
	pop		DELAYH
	pop		TARGETH
	.endif // tsBITS == 16
	pop		XH
	pop		XL
	pop		DELAY
	pop		TARGET
	pop		BYTE
	pop		STATE
	pop		HOLD_SREG
#warning "Fix TX IR"
cbi	DBGPORT, 0
	reti

.endfunc							; tsTX_IR

#undef HOLD_SREG
#undef STATE
#undef BYTE
#undef TAIL
#undef TARGET
#undef TARGETH
#undef DELAY
#undef DELAYH
#undef TEMP
#undef TEMPH

;-----------------------------------------------------------------------------
; void XSerialWrite(uint8_t b)
;
; This function is renamed according to the timer and output compare register
; it is associated with.
;
; If the TX interrupt routine is disabled, enable it and configure byte to
; be sent. If enabled, place byte in TX buffer if space. If no space, delay
; a short period to wait for space. If still no space, return with no action.
;
; Local Variables:
;		r18			Retries to wait for buffer space, tx_state, and temp
;		r19			High byte if timer is 16 bits
;		r20			Temp
;		r21			Temp high byte
;		r22			Holder for SREG
;		r24			Byte to transmit
;		r26			Tail of TX buffer
;		r27			Head of TX buffer
;		r31:r30		Delay count and Z-Index into TX buffer
;-----------------------------------------------------------------------------
#define tsSERIALWRITE tsMAKE_FUNCNAME(tsFUNC_PREFIX,SerialWrite)
.section tsMAKE_SECTIONNAME(tsMAKE_SECTIONNAME(tsTEXT_SECTIONNAME,.),tsSERIALWRITE)

.global	tsSERIALWRITE
.func	tsSERIALWRITE

tsSERIALWRITE:
	lds		r27, tx_buffer_head

	inc		r27
	.if TX_BUFFER_MASK <> 255
	andi	r27, TX_BUFFER_MASK
	.endif

	; Verify that TX buffer is not full.
	; Delay between checks.
	ldi		r18, 12					; Max delay is 12 data bits
01:									; Delay for one data bit
	#ifdef tsBAUDRATE
	ldi		r30, lo8(delay_per_bit)
	ldi		r31, hi8(delay_per_bit)
	#else
	lds		r30, delay_per_bit
	lds		r31, delay_per_bit+1
	#endif
	lds		r26, tx_buffer_tail
	cp		r26, r27				; Buffer full if tail == head
	brne	03f
02:									; Delay loop
	subi	r30, 0x01				; Cycles: 1
	sbci	r31, 0x00				; Cycles: 1
	brne	02b						; Cycles: 1-false; 2-true
	subi	r18, 1
	brne	01b
	ret								; Timed out waiting for space
									; Should add some type of error return

03:
	#if (tsconfigTX & tsconfigASCII)
	; Sending ASCII only so clear high bit
	andi	r24, 0x7F
	#endif

	; Check what state TX interrupt routine is in
	X_in	r22, SREG
	cli
	lds		r18, tx_state
	tst		r18
	breq	04f

	; Currently transmitting bytes so add byte to TX buffer
	lds		r30, tx_buffer_head
	ldi		r31, 0x00
	subi	r30, lo8(-(tx_buffer))
	sbci	r31, hi8(-(tx_buffer))
	st		Z, r24
	sts		tx_buffer_head, r27
	X_out	SREG, r22
	rjmp	05f

	; In idle, set up TX interrupt routine to start transmitting byte
04:	X_out	SREG, r22
	ldi		r18, 0x01
	sts		tx_state, r18			; Set state to transmit start bit
	sts		tx_byte, r24
	ldi		r18, 1<<tsTXOCF			; Enable TX interrupt routine
	X_out	tsTXTIFR, r18
	X_in	r18, tsTIMSK
	ori		r18, 1<<tsTXOCIE
	X_out	tsTIMSK, r18
	X_in	r18, tsTXTCCRA			; Configure timer/counter match clear
	ori		r18, 1<<tsTXCOM1
	andi	r18, ~(1<<tsTXCOM0)
	X_out	tsTXTCCRA, r18
	X_ldsiw	r20, min_lead, tsBITS, tsIMMEDIATE	; Set compare count with a little lead time
	.if tsBITS != 8
	; TEMP1 is used, therefore need to disable interrupts
	cli
	.endif
	X_inw	r18, tsTXTCNT, tsBITS
	X_addw	r18, r20, tsBITS
	X_outw	tsTXOCR, r18, tsBITS
	.if tsBITS != 8
	X_out	SREG, r22
	.endif

05:	ret

.endfunc	; tsSERIALWRITE

;-----------------------------------------------------------------------------
; void XSerialFlushOutput(void)
;
; Wait until all bytes in the transmit buffer have been sent.
;
; Local Variables:
;		r24			tx_state
;-----------------------------------------------------------------------------
#define tsSERIALFLUSHOUTPUT tsMAKE_FUNCNAME(tsFUNC_PREFIX,SerialFlushOutput)
.section tsMAKE_SECTIONNAME(tsMAKE_SECTIONNAME(tsTEXT_SECTIONNAME,.),tsSERIALFLUSHOUTPUT)

.global	tsSERIALFLUSHOUTPUT
.func	tsSERIALFLUSHOUTPUT

tsSERIALFLUSHOUTPUT:
00:	lds		r24, tx_state
	cpse	r24, r1
	rjmp	00b
	ret

#warning "SerialFlushOutput can hang if the TX interrupt is disabled"

.endfunc	; tsSERIALFLUSHOUTPUT

;-----------------------------------------------------------------------------
; uint16_t XSerialPending(void)
;
; Return size of transmit buffer.
;
; Return:
;		r25:r24		Number of bytes waiting to be transmitted
;-----------------------------------------------------------------------------
#define tsSERIALPENDING tsMAKE_FUNCNAME(tsFUNC_PREFIX,SerialPending)
.section tsMAKE_SECTIONNAME(tsMAKE_SECTIONNAME(tsTEXT_SECTIONNAME,.),tsSERIALPENDING)

.global	tsSERIALPENDING
.func	tsSERIALPENDING

tsSERIALPENDING:
	lds		r24, tx_buffer_head
	lds		r25, tx_buffer_tail
	sub		r24, r25
	.if TX_BUFFER_MASK <> 255
	andi	r24, TX_BUFFER_MASK
	.endif
	clr		r25
	ret

.endfunc	; tsSERIALPENDING

#undef DELAY_1MS

#undef tsconfigTX
#undef tsFUNC_PREFIX
#undef tsVAR_TX_PREFIX
#undef tsREG_TX_SUFFIX
#undef tsTX_IR
#undef tsTXOCR
#undef tsTXOCF
#undef tsTXCOM0
#undef tsTXCOM1
#undef tsTXFOC
#undef tsTXOCIE

#undef tsconfigTX
#undef tsFUNC_PREFIX
