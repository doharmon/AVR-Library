;---------------------------------------------------------------------------;
; Timer based SeaTalk software serial ports TX routines
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

.if TX_BUFFER_SIZE > 256
	.error "TX_BUFFER_SIZE must be 256 or less"
.endif
.if TX_BUFFER_SIZE & TX_BUFFER_MASK
	.error "TX_BUFFER_SIZE must be a power of 2"
.endif

; State machine states
#define stsTxIDLE		0
#define stsTxSTART		1
#define stsTxBIT0		2
#define stsTxBIT1		3
#define stsTxBIT2		4
#define stsTxBIT3		5
#define stsTxBIT4		6
#define stsTxBIT5		7
#define stsTxBIT6		8
#define stsTxBIT7		9
#define stsTxCMND		10
#define stsTxSTOP		11
#define stsTxCHKTX		12
#define stsTxCHKBUF		13

.section stsBSS_SECTIONNAME

#define stsTX_IR				stsMAKE_REGNAME(stsTX_IR,stsREG_TX_SUFFIX)
#define stsTXOCR				stsMAKE_REGNAME(stsTXOCR,stsREG_TX_SUFFIX)
#define stsTXOCF				stsMAKE_REGNAME(stsTXOCF,stsREG_TX_SUFFIX)
#define stsTXCOM0				stsMAKE_REGNAME(stsTXCOM0,stsREG_TX_SUFFIX)
#define stsTXCOM1				stsMAKE_REGNAME(stsTXCOM1,stsREG_TX_SUFFIX)
#define stsTXFOC				stsMAKE_REGNAME(stsTXFOC,stsREG_TX_SUFFIX)
#define stsTXOCIE				stsMAKE_REGNAME(stsTXOCIE,stsREG_TX_SUFFIX)
#define stsVAR_TX_PREFIX		stsMAKE_REGNAME(stsVAR_TX_PREFIX,stsREG_TX_SUFFIX)

#define tx_state				stsMAKE_VARNAME(stsVAR_TX_PREFIX,tx_state)
#define tx_byte					stsMAKE_VARNAME(stsVAR_TX_PREFIX,tx_byte)
#define tx_bit					stsMAKE_VARNAME(stsVAR_TX_PREFIX,tx_bit)
#define tx_target				stsMAKE_VARNAME(stsVAR_TX_PREFIX,tx_target)
#define tx_buffer_head			stsMAKE_VARNAME(stsVAR_TX_PREFIX,tx_buffer_head)
#define tx_buffer_tail			stsMAKE_VARNAME(stsVAR_TX_PREFIX,tx_buffer_tail)
#define tx_buffer				stsMAKE_VARNAME(stsVAR_TX_PREFIX,tx_buffer)

; SeaTalk specific
#define tx_command				stsMAKE_VARNAME(stsVAR_TX_PREFIX,tx_command)
#define tx_app_command			stsMAKE_VARNAME(stsVAR_TX_PREFIX,tx_app_command)
#define tx_datagram_byte		stsMAKE_VARNAME(stsVAR_TX_PREFIX,tx_datagram_byte)
#define tx_byte_sent			stsMAKE_VARNAME(stsVAR_TX_PREFIX,tx_byte_sent)

; The following globals are for debugging. Should be removed.
.global tx_state
.global tx_byte
.global tx_bit
.global tx_target
.global tx_buffer_head
.global tx_buffer_tail
.global tx_buffer
.global tx_command
.global tx_app_command
.global tx_datagram_byte
.global tx_byte_sent

tx_state:				.ds.b	1
tx_byte:				.ds.b	1
tx_bit:					.ds.b	1
tx_target:				stsDS	1
tx_buffer_head:			.ds.b	1
tx_buffer_tail:			.ds.b	1
tx_buffer:				.ds.b	TX_BUFFER_SIZE*2
tx_command:				.ds.b	1
tx_app_command:			.ds.b	1
tx_datagram_byte:		.ds.b	1	// Position in TX buffer of current byte being sent
tx_byte_sent:			.ds.b	1	// Used to compare to byte received

;-----------------------------------------------------------------------------
; TX Interrupt Routine
;
; State		Name		Description
;	0		stsTxIDLE	Idle: IR routine disabled
;	1		stsTxSTART	Add delay for start bit.
;  2-8		stsTxBIT0-6	Add delay for data bit.
;	9		stsTxBIT7	Add delay for data bit.
;  10		stsTxCMND	Add delay for command bit.
;  11		stsTxSTOP	Add delay for stop bit.
;  12		stsTxCHKTX	Check TX byte against RX byte.
;  13		stsTxCHKBUF	Check if more bytes in TX buffer.
;
; Data Byte: 0x9B 0b10011011
; Cmnd Bit:  0x00
;
; Line Level:     1   0   1   1   0   1   1   0   0   1   0   1
; State:		Idl Strt B0  B1  B2  B3  B4  B5  B6  B7 Cmd Stop  
;				+---+   +---+---+   +---+---+       +---+   +---+
;   				|   |       |   |       |       |	|	|
;	    			|   |       |   |       |       |	|	|
;	    			+---+       +---+       +---+---+	+---+
;
;			   	|   |   |   |   |   |   |   |   |   |   |	|   |
;			   	|   |   |   |   |   |   |   |   |   |   |	|   +-- stsTxCHKTX/stsTxCHKBUF
;			   	|   |   |   |   |   |   |   |   |   |   |	+-- stsTxSTOP
;			   	|   |   |   |   |   |   |   |   |   |   *-- stsTxCMND
;			   	|   |   |   |   |   |   |   |   |   +-- stsTxBIT7
;			   	|   |   |   |   |   |   |   |   +-- stsTxBIT6
;			   	|   |   |   |   |   |   |   +-- stsTxBIT5
;			   	|   |   |   |   |   |   +-- stsTxBIT4
;			   	|   |   |   |   |   +-- stsTxBIT3
;			   	|   |   |   |   +-- stsTxBIT2
;			   	|   |   |   +-- stsTxBIT1
;			   	|   |   +-- stsTxBIT0
;			   	|   +-- stsTxSTART
;			   	+-- stsTxIDLE
;
;	NOTE:	Data bits are sent LSB first.
;			Any line level changes happen before IR routine is called.
;
; Local Variables:
;		r15		HOLD_SREG
;		r16		STATE
;		r17		BYTE
;		r22		TARGET(L)
;		r23		TARGETH
;		r24		TEMP2	or	TAIL
;		r25		TEMP2H
;		r26		XL		or	TEMP(L)
;		r27		XH		or	TEMPH
;-----------------------------------------------------------------------------
.section stsTEXT_SECTIONNAME

; These are undefined at the bottom of the function
#define HOLD_SREG	r15
#define STATE		r16
#define BYTE		r17
#define TARGET		r22
#define TARGETH		r23
#define TAIL		r24
#define TEMP2		r24
#define TEMP2H		r25
#define TEMP		r26				// XL
#define	TEMPH		r27				// XH

.global stsTX_IR
.func stsTX_IR

stsTX_IR:
;#warning "Fix TX IR"
;sbi	0x02, 0
	push	HOLD_SREG
	push	STATE
	push	BYTE
	push	TARGET
	push	TEMP2
	push	XL
	push	XH
	.if stsBITS == 16
	push	TARGETH
	push	TEMP2H
	.endif // stsBITS == 16
	X_in	HOLD_SREG, SREG

	lds		STATE, tx_state
	lds		BYTE, tx_byte								; Not needed for stsTxCHKBUF
	X_ldsiw	TEMP2, ticks_per_bit, stsBITS, stsIMMEDIATE	; Not needed for stsTxCHKBUF
	X_ldsw	TARGET, tx_target, stsBITS

	; ------------------------------------------------------------------------
	; Jump to state handler
	cpi		STATE, stsTxBIT0
	brlo	10f						; stsTxSTART
	breq	20f						; stsTxBIT0
	cpi		STATE, stsTxSTOP
	brlo	30f						; stsTxBIT1-7,stsTXCMND
	breq	01f
	cpi		STATE, stsTxCHKTX
	.if stsBITS == 8
	breq	40f						; stsTxCHKTX
	.else
	breq	02f
	.endif
	rjmp	50f						; stsTxCHKBUF
01:	rjmp	60f						; stsTxSTOP
	.if stsBITS == 16
02:	rjmp	40f						; stsTxCHKTX
	.endif
/*
	cpi		STATE, stsTxSTART
	brne	01f
	rjmp	10f						; stsTxSTART
01:	cpi		STATE, stsTxBIT0
	brne	02f
	rjmp	20f						; stsTxBIT0
02:	cpi		STATE, stsTxSTOP
	brsh	03f
	rjmp	30f						; stsTxBIT1-7,stsTxCMND
03:	cpi		STATE, stsTxSTOP
	brne	04f
	rjmp	60f						; stsTxSTOP
04:	cpi		STATE, stsTxCHKTX
	brne	05f
	rjmp	40f						; stsTxCHKTX
05:	rjmp	50f						; stsTxCHKBUF
*/
	; ------------------------------------------------------------------------
	; Handle stsTxSTART
	; Timing is based off of counter at start of start bit.
10:	X_inw	TARGET, stsTXOCR, stsBITS
	X_addw	TARGET, TEMP2, stsBITS	; Add delay for start bit
	inc		STATE
	lds		TEMP, tx_command		; Load command bit into MSB of TX byte
	lsr		TEMP					; TEMP = 0 = tx_bit
	ror		BYTE
	brcs	33f
	inc		STATE					; LSB = 0
	X_addw	TARGET, TEMP2, stsBITS	; Add delay for LSB
	lsr		BYTE
	ori		BYTE, 0x80				; Load stop bit into MSB of TX byte
	brcc	31f						; Bit 1 = 0
	rjmp	32f						; Bit 1 = 1

	; ------------------------------------------------------------------------
	; Handle stsTxBIT0
20:	inc		STATE					; LSB is 1
	X_addw	TARGET, TEMP2, stsBITS	; Add delay for LSB
	lsr		BYTE
	ori		BYTE, 0x80				; Load stop bit into MSB of TX byte
	brcs	34f						; Bit 1 = 1
	rjmp	35f						; Bit 1 = 0

	; ------------------------------------------------------------------------
	; Handle data bits 1-7 and command bit
30:
	lds		TEMP, tx_bit
	sbrc	TEMP, 0
	rjmp	34f

									; Current bit is 0
31:	X_addw	TARGET, TEMP2, stsBITS	; Loop checks if next bits are same as the
	inc		STATE					; current bit being sent.
	lsr		BYTE
	brcc	31b						; Bottom of loop
32:	ldi		TEMP, 1
	sts		tx_bit, TEMP			; Next bit is a 1
33:	X_in	TEMP, stsTXTCCRA		; Raise level on next timeout
	ori		TEMP, (1<<stsTXCOM1) | (1<<stsTXCOM0)
	X_out	stsTXTCCRA, TEMP
	sts		tx_byte, BYTE
	rjmp	61f						; Exit

									; Current bit is 1
34:	X_addw	TARGET, TEMP2, stsBITS	; Loop checks if next bits are same as the
	inc		STATE					; current bit being sent.
	lsr		BYTE
	brcs	34b						; Bottom of loop
	cpi		STATE, stsTxSTOP
	brlo	35f
	rjmp	61f						; Exit
35:	ldi		TEMP, 0
	sts		tx_bit, TEMP			; Next bit is a 0
	X_in	TEMP, stsTXTCCRA		; Lower level on next timeout
	ori		TEMP, (1<<stsTXCOM1)
	andi	TEMP, ~(1<<stsTXCOM0)
	X_out	stsTXTCCRA, TEMP
	sts		tx_byte, BYTE
	rjmp	61f						; Exit

	; ------------------------------------------------------------------------
	; Handle stsTxCHKTX
40:	clr		STATE					; Set state to stsTxIDLE
	lds		TEMP, rx_byte			; Compare sent byte with received byte
	lds		TEMP2, tx_byte_sent
	cp		TEMP, TEMP2
	breq	51f
									; Collision
	lds		TAIL, tx_buffer_tail	; Reset to beginning of datagram
	sts		tx_datagram_byte, TAIL
41:	X_in	TEMP, stsTIMSK
	andi	TEMP, ~(1<<stsTXOCIE)
	X_out	stsTIMSK, TEMP			; Disable INT compare TX
	rjmp	62f						; Exit

	; ------------------------------------------------------------------------
	; Handle stsTxCHKBUF - This state is used by XSerialCheckTxBuffer
50:	clr		STATE					; Set state to stsTxIDLE
51:	lds		TEMP, tx_buffer_head
	lds		TAIL, tx_datagram_byte
	cp		TAIL, TEMP
	breq	41b
									; Ready next byte to transmit
	mov		XL, TAIL				; X = Index into tx buffer
	clr		XH
	lsl		XL
	.if TX_BUFFER_SIZE > 127
	rol		XH
	.endif
	subi	XL, lo8(-(tx_buffer))	; Offset into buffer
	sbci	XH, hi8(-(tx_buffer))
	ld		BYTE, X+				; Get next byte
	sts		tx_byte, BYTE
	sts		tx_byte_sent, BYTE		; Used to compare byte received
	ld		BYTE, X					; Get next command bit
	sts		tx_command, BYTE
	tst		BYTE
	breq	52f
	sts		tx_buffer_tail, TAIL	; Start of a new command so update tail
52:	inc		TAIL					; Move tail to next byte in array
	.if TX_BUFFER_MASK <> 255
	andi	TAIL, TX_BUFFER_MASK
	.endif
	sts		tx_datagram_byte, TAIL
	X_in	TEMP, stsTXTCCRA
	ori		TEMP, (1<<stsTXCOM1)
	andi	TEMP, ~(1<<stsTXCOM0)
	X_out	stsTXTCCRA, TEMP		; Lower level after waiting
	X_ldsiw	TEMP2, min_lead, stsBITS, stsIMMEDIATE
									; State set to stsTxSTART below

	; ------------------------------------------------------------------------
	; Exit
60:	inc		STATE					; Handle stsTxSTOP
	X_addw	TARGET, TEMP2, stsBITS
61:	X_stsw	tx_target, TARGET, stsBITS
	X_outw	stsTXOCR, TARGET, stsBITS
62:	sts		tx_state, STATE

	X_out	SREG, HOLD_SREG
	.if stsBITS == 16
	pop		TEMP2H
	pop		TARGETH
	.endif // stsBITS == 16
	pop		XH
	pop		XL
	pop		TEMP2
	pop		TARGET
	pop		BYTE
	pop		STATE
	pop		HOLD_SREG
;#warning "Fix TX IR"
;cbi	0x02, 0
	reti

.endfunc							; stsTX_IR

#undef HOLD_SREG
#undef STATE
#undef BYTE
#undef TAIL
#undef TARGET
#undef TARGETH
#undef TEMP2
#undef TEMP2H
#undef TEMP
#undef TEMPH

;-----------------------------------------------------------------------------
; void XSerialCheckTxBuffer(void)
;
; This function is renamed according to the timer and output compare register
; it is associated with.
;
; Checks if there are bytes in the TX buffer. If not currently transmitting
; then check that a minimum time since last received byte has passed and
; enable transmit interrupt routine.
;
; NOTE: This function should be called periodically from main program loop.
;
; Local Variables:
;		r18			tx_state
;		r21:r18		minimum time since last received byte
;		r25:r22		milliseconds from call to millis()
;		r26			Tail of TX buffer
;		r27			Head of TX buffer
;-----------------------------------------------------------------------------
#define stsSERIALCHECKTXBUFFER stsMAKE_FUNCNAME(stsFUNC_PREFIX,SerialCheckTxBuffer)
.section stsMAKE_SECTIONNAME(stsMAKE_SECTIONNAME(stsTEXT_SECTIONNAME,.),stsSERIALCHECKTXBUFFER)

.global	stsSERIALCHECKTXBUFFER
.func	stsSERIALCHECKTXBUFFER

stsSERIALCHECKTXBUFFER:
	; Check if anything to transmit
	lds		r26, tx_buffer_tail
	lds		r27, tx_buffer_head
	cp		r26, r27
	brne	01f
	ret

	; Verify not transmitting and minimum time since last received byte
	; if ((0 == tx_state) && (rx_count + SEATALK_MIN_WAIT_TIME < millis()))
01:	lds		r18, tx_state
	tst		r18
	brne	02f
	call	millis
	lds		r18, rx_count
	lds		r19, rx_count+1
	lds		r20, rx_count+2
	lds		r21, rx_count+3
	subi	r18, lo8(-stconfigMIN_WAIT_TIME)
	sbci	r19, hi8(-stconfigMIN_WAIT_TIME)
	sbci	r20, hlo8(-stconfigMIN_WAIT_TIME)
	sbci	r21, hhi8(-stconfigMIN_WAIT_TIME)
	cp		r18, r22
	cpc		r19, r23
	cpc		r20, r24
	cpc		r21, r25
	brsh	02f	

	; In idle, set up TX interrupt routine to start transmitting byte
	ldi		r18, stsTxCHKBUF
	sts		tx_state, r18			; Set state to check TX buffer
	ldi		r18, 1<<stsTXOCF		; Enable TX interrupt routine
	X_out	stsTXTIFR, r18
	X_in	r18, stsTIMSK
	ori		r18, 1<<stsTXOCIE
	X_out	stsTIMSK, r18
	X_in	r18, stsTXTCCRA			; Configure timer/counter set level high
	ori		r18, 1<<stsTXCOM0 | 1<<stsTXCOM1
	X_out	stsTXTCCRA, r18
	X_ldsiw	r21, min_lead, stsBITS, stsIMMEDIATE	; Set compare count with a little lead time
	.if stsBITS != 8
	X_in	r20, SREG				; TEMP1 is a shared register
	cli
	.endif
	X_inw	r18, stsTXTCNT, stsBITS
	X_addw	r18, r21, stsBITS
	X_outw	stsTXOCR, r18, stsBITS
	.if stsBITS != 8
	X_out	SREG, r20
	.endif
	X_stsw	tx_target, r18, stsBITS

02:	ret

.endfunc	; stsSERIALCHECKTXBUFFER

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
;		r20			Holder for SREG
;		r21			Temp
;		r22			Temp high byte
;		r24			Byte to transmit
;		r26			Tail of TX buffer
;		r27			Head of TX buffer
;		r31:r30		Delay count and Z-Index into TX buffer
;-----------------------------------------------------------------------------
#define stsSERIALWRITE stsMAKE_FUNCNAME(stsFUNC_PREFIX,SerialWrite)
.section stsMAKE_SECTIONNAME(stsMAKE_SECTIONNAME(stsTEXT_SECTIONNAME,.),stsSERIALWRITE)

.global	stsSERIALWRITE
.func	stsSERIALWRITE

stsSERIALWRITE:
	lds		r27, tx_buffer_head

	inc		r27
	.if TX_BUFFER_MASK <> 255
	andi	r27, TX_BUFFER_MASK
	.endif

	; Verify that TX buffer is not full.
	; Delay between checks.
	ldi		r18, 23					; Max delay is 23 data bits
01:									; Delay for one data bit
	#ifdef stsBAUDRATE
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
	rjmp	05f						; Timed out waiting for space
									; Should add some type of error return

03:
	X_in	r26, SREG
	cli
	lds		r18, tx_state
	tst		r18
	brne	04f
	X_out	SREG, r26				; Not transmitting
04:	lds		ZL, tx_buffer_head
	ldi		ZH, 0x00
	lsl		ZL
	.if TX_BUFFER_SIZE > 127
	rol		ZH
	.endif
	subi	ZL, lo8(-(tx_buffer))
	sbci	ZH, hi8(-(tx_buffer))
	st		Z+, r24					; Add byte to TX buffer
	lds		r24, tx_app_command
	st		Z, r24					; Add command bit to TX buffer
	sts		tx_buffer_head, r27
	sts		tx_app_command, r1
	X_out	SREG, r26

05:	rcall	stsSERIALCHECKTXBUFFER

	ret

.endfunc	; stsSERIALWRITE

;-----------------------------------------------------------------------------
; void XSerialSetCommand(void)
;
; This function is renamed according to the timer and output compare register
; it is associated with.
;
; Makes next byte written a command (Bit 9 is set).
;
; Local Variables:
;		r24			temp
;-----------------------------------------------------------------------------
#define stsSERIALSETCOMMAND stsMAKE_FUNCNAME(stsFUNC_PREFIX,SerialSetCommand)
.section stsMAKE_SECTIONNAME(stsMAKE_SECTIONNAME(stsTEXT_SECTIONNAME,.),stsSERIALSETCOMMAND)

.global	stsSERIALSETCOMMAND
.func	stsSERIALSETCOMMAND

stsSERIALSETCOMMAND:
	ldi		r24, 0x01
	sts		tx_app_command, r24
	ret

.endfunc	; stsSERIALSETCOMMAND

;-----------------------------------------------------------------------------
; void XSerialClearCommand(void)
;
; This function is renamed according to the timer and output compare register
; it is associated with.
;
; Mark next byte written as not a command (Bit 9 is clear).
;
; Local Variables:
;		r24			temp
;-----------------------------------------------------------------------------
#define stsSERIALCLEARCOMMAND stsMAKE_FUNCNAME(stsFUNC_PREFIX,SerialClearCommand)
.section stsMAKE_SECTIONNAME(stsMAKE_SECTIONNAME(stsTEXT_SECTIONNAME,.),stsSERIALCLEARCOMMAND)

.global	stsSERIALCLEARCOMMAND
.func	stsSERIALCLEARCOMMAND

stsSERIALCLEARCOMMAND:
	clr		r24
	sts		tx_app_command, r24
	ret

.endfunc	; stsSERIALCLEARCOMMAND

;-----------------------------------------------------------------------------
; void XSerialFlushOutput(void)
;
; This function is renamed according to the timer and output compare register
; it is associated with.
;
; Wait until all bytes in the transmit buffer have been sent.
;
; Local Variables:
;		r24			tx_state
;-----------------------------------------------------------------------------
#define stsSERIALFLUSHOUTPUT stsMAKE_FUNCNAME(stsFUNC_PREFIX,SerialFlushOutput)
.section stsMAKE_SECTIONNAME(stsMAKE_SECTIONNAME(stsTEXT_SECTIONNAME,.),stsSERIALFLUSHOUTPUT)

.global	stsSERIALFLUSHOUTPUT
.func	stsSERIALFLUSHOUTPUT

stsSERIALFLUSHOUTPUT:
	rcall	stsSERIALCHECKTXBUFFER
00:	lds		r24, tx_state
	cpse	r24, r1
	rjmp	00b
	ret

#warning "SerialFlushOutput needs to be cleaned up"

.endfunc	; stsSERIALFLUSHOUTPUT

#undef DELAY_1MS

#undef stconfigTX
#undef stsFUNC_PREFIX
#undef stsVAR_TX_PREFIX
#undef stsREG_TX_SUFFIX
#undef stsTX_IR
#undef stsTXOCR
#undef stsTXOCF
#undef stsTXCOM0
#undef stsTXCOM1
#undef stsTXFOC
#undef stsTXOCIE

#undef stconfigTX
#undef stsFUNC_PREFIX
