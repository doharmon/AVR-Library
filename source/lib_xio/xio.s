;----------------------------------------------------------------------------
; Extended itoa, puts, printf and atoi                     (C)ChaN, 2011
;
; doharmon 2015
; Added fixed point functionality and xfunc_out as passed parameter
;----------------------------------------------------------------------------
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
; Source for above:
; http://www.atmel.com/webdoc/AVRLibcReferenceManual/FAQ_1faq_reg_usage.html
;----------------------------------------------------------------------------

#define xioASSEMBLER	1
#include "xio.h"

#if FLASHEND > 0x1FFFF
#error xio module does not support 256K devices
#endif

.nolist
#include <avr/io.h>					// Device specific definitions.
#include "../common/macros.inc"		// Assembler macros
.list

;----------------------------------------------------------------------------
; Stub function to forward to user output function
;
; Prototype: void xputc(void (*xfunc_out)(uint8_t),	// output function
;						 char chr					// character to output
;			 		   );
;
; Arguments:
;	r25:r24		pointer to xfunc_out
;	    r22		character to output
;
; Returns:
;	r24:r25		int
;	If COUNT_CHARS is nonzero then the number of chars output is returned
;	or a negative value if there was an error.
;----------------------------------------------------------------------------
.section .data.lib_xio
.global xfunc_out							; xfunc_out must be initialized before using xprintf_P.
xfunc_out:	.byte	0, 0

.section .text.lib_xio.xputc
.func xputc
.global xputc
xputc:
	cp		r24, r1							; Verify pointer is not null
	cpc		r25, r1							;
	breq	10f								; /

#if LF_CRLF
#if COUNT_CHARS
	cpi		r22, 10							; LF --> CRLF
	breq	1f								; Send two chars
	rjmp	9f								;
1:	ldi		r22, 13							;
	push	r25								;
	push	r24								;
	rcall	9f								;
	cpi		r24, 0xFF						;
	breq	2f								; Error
	ldi		r22, 10							;
	pop		r24								;
	pop		r25								;
	rcall	9f								;
	inc		r24								; Count is 2
	ret										;
2:	pop		r0								; Trash saved r25:r24
	pop		r0								; /
	ret										;
#else	// COUNT_CHARS
	cpi		r22, 10							; LF --> CRLF
	brne	9f								;
	ldi		r22, 13							;
	rcall	9f								;
	ldi		r22, 10							; /
#endif	// COUNT_CHARS
#endif	// LF_CRLF

9:	push	ZH								;
	push	ZL								;
#if COUNT_CHARS == 0
	push	r25								; Save pointer to output func
	push	r24								; /
#endif
	X_movw	ZL, r24							; Pointer to output function
	mov		r24, r22						; r24 = Byte to output
	icall									;
#if COUNT_CHARS == 0
	pop		r24								;
	pop		r25								;
#endif
	pop		ZL								;
	pop		ZH								;
#if COUNT_CHARS
	ldi		r24, 1							; 1 char output
	clr		r25								;
	ret										;
#endif
10:
#if COUNT_CHARS
	ser		r24								; Error
	ser		r25								; /
#endif
	ret										;
.endfunc	// xputc


;----------------------------------------------------------------------------
; Direct ROM string output
;
; Prototype: void xputs_P (void (*xfunc_out)(uint8_t), // output function
;						   const prog_char *str);	   // ROM string
;
; Arguments:
;	r25:r24		pointer to output func
;	r23:r22		pointer to ROM string
;
; Local Variables:
;	r17:r16		count of chars if COUNT_CHARS is nonzero
;
; Returns:
;	r24:r25		int
;	If COUNT_CHARS is nonzero then the number of chars output is returned
;	or a negative value if there was an error.
;----------------------------------------------------------------------------
.section .text.lib_xio.xputs_P
.func xputs_P
.global xputs_P
xputs_P:
#if COUNT_CHARS
	push	r17								;
	push	r16								;
	push	r15								;
	push	r14								;
	clr		r17								;
	clr		r16								;
	X_movw	r14, r24						;
#endif
	X_movw	ZL, r22							; Z = pointer to ROM string
1:	X_lpm	r22, Z+							;
	cpi		r22, 0							;
	breq	2f								;
	rcall	xputc							;
#if COUNT_CHARS
	cpi		r24, 0xFF						;
	breq	3f								;
	add		r16, r24						;
	adc		r17, r25						;
	X_movw	r24, r14						;
#endif
	rjmp	1b								;
2:
#if COUNT_CHARS
	X_movw	r24, r16						;
	pop		r14								;
	pop		r15								;
	pop		r16								;
	pop		r17								;
#endif
3:	ret										;
.endfunc	// xputs_P


;----------------------------------------------------------------------------
; Direct RAM string output
;
; Prototype: void xputs (void (*xfunc_out)(uint8_t), // output function
;	  					 const *str);				 // RAM string to output
;
; Arguments:
;	r25:r24		pointer to output func
;	r23:r22		pointer to RAM string
;
; Local Variables:
;	r17:r16		count of chars if COUNT_CHARS is nonzero
;
; Returns:
;	r24:r25		int
;	If COUNT_CHARS is nonzero then the number of chars output is returned
;	or a negative value if there was an error.
;----------------------------------------------------------------------------
.section .text.lib_xio.xputs
.func xputs
.global xputs
xputs:
#if COUNT_CHARS
	push	r17								;
	push	r16								;
	push	r15								;
	push	r14								;
	clr		r17								; Set count to zero
	clr		r16								; /
	X_movw	r14, r24						;
#endif
	X_movw	ZL, r22							; Z = pointer to RAM string
1:	ld		r22, Z+							;
	cpi		r22, 0							;
	breq	2f								;
	rcall	xputc							;
#if COUNT_CHARS
	cpi		r24, 0xFF						;
	breq	3f								;
	add		r16, r24						;
	adc		r17, r25						;
	X_movw	r24, r14						;
#endif
	rjmp	1b								; /
2:
#if COUNT_CHARS
	X_movw	r24, r16						;
	pop		r14								;
	pop		r15								;
	pop		r16								;
	pop		r17								;
#endif
3:	ret										;
.endfunc	// xputs


;----------------------------------------------------------------------------
; Formatted string output (16/32/64 bit version)
;
; Prototype:
;   void xprintf_P(const prog_char* format, ...);
;   void xvprintf_P(const prog_char* format, va_list);
;   void xvsprintf_P(char* str, const prog_char* format, ...);
;   void xsprintf_P(char* str, const prog_char* format, ...);
;   void xvsnprintf_P(char* str, int size, const prog_char* format, ...);
;   void xsnprintf_P(char* str, int size, const prog_char* format, ...);
;   void xfprintf_P(void(*func)(char), const prog_char* format, ...);
;
; If COUNT_CHARS is nonzero:
;   uint8_t xprintf_P(const prog_char* format, ...);
;   uint8_t xvprintf_P(const prog_char* format, va_list);
;   uint8_t xvsprintf_P(char* str, const prog_char* format, ...);
;   uint8_t xsprintf_P(char* str, const prog_char* format, ...);
;   uint8_t xvsnprintf_P(char* str, int size, const prog_char* format, ...);
;   uint8_t xsnprintf_P(char* str, int size, const prog_char* format, ...);
;   uint8_t xfprintf_P(void(*func)(char), const prog_char* format, ...);
;----------------------------------------------------------------------------

#if USE_XPRINTF_P

;----------------------------------------------------------------------------
; xioprintf_P
;
; Main printf_P routine. Called by the other printf_P routines.
;
; Arguments
;       T		Set:   pointer to argument frame after format string
;				Clear: arguments follow format string
;	r25:r24		Pointer to output func
;	r29:r28		Y points to argument frame
;
; Local variables
;	r7: r6		Count of chars output if COUNT_CHARS is nonzero
;		r16		Width
;		r18		Number of bits in fixed point fraction; Radix for int
;		r19		Current conversion specification char being parsed
;		r22		Current char being parsed, param to xputc
;		r20		Temp
;		r26		Flags
;		r27		Precision
;	r23:r22		8/16 bit value
;	r23:r20		32 bit value
;	r23:r16		64 bit value
;	r31:r30		Z points to format string in ROM
;
; Returns
;	r24:r25		int
;	If COUNT_CHARS is nonzero then the number of chars output is returned
;	or a negative value if there was an error.
;----------------------------------------------------------------------------
.section .text.lib_xio.xioprintf_P
.func xioprintf_P
.global xioprintf_P
#if COUNT_CHARS
xvcountchars:
	cpi		r25, 0xFF						; Assume char count never over 0xFEFF
	breq	01f								;
	add		r6, r24							; Add to running total
	adc		r7, r25							; /
	ret										;
01:	pop		r0								; Trash return address
	pop		r0								; /
	pop		r0								; r25:r24
	pop		r0								; /
	rjmp	91f								;
#endif // COUNT_CHARS

xioprintf_P:
											; Push registers
	#if USE_XLQTOA
	push	r17								; Used to pass part of 64 bit fp
	#endif
	push	r16								; Width int/32 bit
	#if USE_XQTOA
	push	r14								; Flags		32 bit; Frac bits	64 bit
	push	r12								; Precision 32 bit; Width		64 bit
	#endif
	#if USE_XLQTOA
	push	r10								;					Flags		64 bit
	push	r8								;					Precision	64 bit
	#endif
	#if COUNT_CHARS
	push	r7								; r7:r6 Number of chars output or negative
	push	r6								;		value if error
	#endif
	
	#if COUNT_CHARS
	clr		r6								; Set counter to zero
	clr		r7								; /
	#endif

	ld		ZL, Y+							; Z = pointer to format string in ROM
	ld		ZH, Y+							; /

	brtc	0f								; Test if need to reset Y to va_list
;	ld		XL, Y+							;
;	ld		XH,	Y+							;
;	X_movw	YL, XL							; /
	ld		r0, Y+							;
	ld		YH,	Y							;
	mov		YL, r0							; /

											; ----------------------------------------------
											; Head of parse loop
											; ----------------------------------------------
0:	X_lpm	r22, Z+							; Get a format char
	cpi		r22, 0							; End of format string?
	breq	90f								; /
	cpi		r22, '%'						; Is format?
	breq	2f								; /
1:
	#if COUNT_CHARS
	push	r25								; Save output func pointer
	push	r24								; /
	#endif
	rcall	xputc							; Put a normal character
	#if COUNT_CHARS
	rcall	xvcountchars					;
	pop		r24								; Restore output func pointer
	pop		r25								; /
	#endif
	rjmp	0b								; /
2:	X_lpm	r19, Z+							; Get conversion character
	cpi		r19, '%'						; Is a %? (r22 equals ASCII '%')
	breq	1b								; /

											; ----------------------------------------------
											; Start parsing conversion specification
											; ----------------------------------------------
	clr		r16								; Reset width
	clr		r26								; Reset flags
	#if USE_XQTOA
	ldi		r27, 0xFF						; Mark precision undefined
	#endif

											; ----------------------------------------------
											; Parse flags
											; ----------------------------------------------
3:	cpi		r19, '0'						; Left pad with zeros
	#if USE_XQTOA
	brne	4f								;
	ori		r26, xioFLAG_ZERO_FILL			;
	rjmp	7f								;
4:	cpi		r19, '-'						; Left justify
	brne	5f								;
	ori		r26, xioFLAG_LEFT_JUSTIFIED		;
	rjmp	7f								;
5:	cpi		r19, '+'						; Output plus sign on positive number
	brne	6f								;
	ori		r26, xioFLAG_LEADING_PLUS		;
	rjmp	7f								;
6:	cpi		r19, ' '						; Output leading space on positive number
	brne	11f								;
	ori		r26, xioFLAG_LEADING_SPACE		;
7:	X_lpm	r19, Z+							; Get conversion character
	rjmp	3b								;
	#else	// USE_XQTOA
	brne	11f								;
	ori		r26, xioFLAG_ZERO_FILL			;
	#endif	// USE_XQTOA

											; ----------------------------------------------
											; Parse width
											; ----------------------------------------------
10:	X_lpm	r19, Z+							; Get next digit
11:	cpi		r19, '9'+1						;
	brcc	12f								; Not a digit or '.'
	#if USE_XQTOA
	cpi		r19, '.'						; End of width, start of precision?
	breq	20f								; /
	#endif
	subi	r19, '0'						; Convert ASCII digit to numeric value
	brcs	90f								; Non ASCII digit
	lsl		r16								; r16 = r16*10 + r19
	mov		r0, r16							;
	lsl		r16								;
	lsl		r16								;
	add		r16, r0							;
	add		r16, r19						; /
	rjmp	10b								;
12:	cpi		r16, 0x80						; Max width is 127
	brlo	30f								;
	ldi		r16, 0x7F						;
	rjmp	30f								;

											; ----------------------------------------------
											; Exit
											; ----------------------------------------------
;20:
90:											; Pop registers
	#if COUNT_CHARS
	X_movw	r24, r6							; Return char count

91:	pop		r6								;
	pop		r7								;
	#endif
	#if USE_XLQTOA
	pop		r8								;
	pop		r10								;
	#endif
	#if USE_XQTOA
	pop		r12								;
	pop		r14								;
	#endif
	pop		r16								;
	#if USE_XLQTOA
	pop		r17								;
	#endif
	ret										;
	
#if USE_XQTOA
											; ----------------------------------------------
											; Parse precision
											; ----------------------------------------------
;30:
20:	X_lpm	r19, Z+							; Get next digit
	cpi		r19, '9'+1						;
	brcc	90b								;
	subi	r19, '0'						;
	brcs	90b								;
	mov		r27, r19						;
	X_lpm	r19, Z+							; Get optional second digit
	cpi		r19, '9'+1						;
	brcc	12b								; Check max width
	subi	r19, '0'						;
	brcs	90b								;
	lsl		r27								; r27 = r27*10 + r19
	mov		r0, r27							;
	lsl		r27								;
	lsl		r27								;
	add		r27, r0							;
	add		r27, r19						; /
	X_lpm	r19, Z+							; Get next format char
	brcc	12b								; Check max width

#endif	// USE_XQTOA

											; ----------------------------------------------
											; Parse size
											; ----------------------------------------------
;40:
30:
;	#if USE_XQTOA
;	cpi		r19, 'U'						; Unsigned
;	brne	31f								;
;	ori		r26, xioFLAG_UNSIGNED			;
;	rjmp	33f								;
;	#endif
31:	cpi		r19, 'h'						; Short
	brne	32f								;
	ori		r26, xioFLAG_SHORT				;
	rjmp	33f								;
32:	cpi		r19, 'l'						; Long
	brne	40f								;
	ori		r26, xioFLAG_LONG				;
33:	X_lpm	r19, Z+							;
;	#if USE_XQTOA
;	mov		r20, r26						;
;	andi	r20, xioFLAG_SHORT+xioFLAG_LONG	;
;	breq	31b								; Test if h or l already passed
;	#endif

											; ----------------------------------------------
											; Parse type
											; ----------------------------------------------
;50:
40:	ld		r22, Y+							; Get value (low word)
	ld		r23, Y+							; /
	#if USE_XQTOA
	cpi		r19, 'k'						; Is type fixed point accum?
	brne	80f								;
	rjmp	50f								;
80:	cpi		r19, 'K'						; Is type fixed point unsigned accum?
	brne	96f								;
	ori		r26, xioFLAG_UNSIGNED			;
	rjmp	50f								;
96:	cpi		r19, 'q'						; Is type fixed point variable frac bits?
	brne	81f								;
	rjmp	52f								;
81:	cpi		r19, 'Q'						; Is type fixed point unsigned variable frac bits?
	brne	97f								;
	ori		r26, xioFLAG_UNSIGNED			;
	rjmp	52f								;
97:	cpi		r19, 'r'						; Is type fixed point frac?
	brne	83f								;
82:	sbrc	r26, xioFLAG_SHORT_BIT			;
	sbiw	YL, 1							; Give back a byte
	rjmp	60f								;
83:	cpi		r19, 'R'						; Is type fixed point unsigned frac?
	brne	98f								;
	ori		r26, xioFLAG_UNSIGNED			;
	rjmp	82b								;
98:	mov		r20, r26						;
	andi	r20, xioFLAG_UNSIGNED_BIT + xioFLAG_LEFT_JUSTIFIED + xioFLAG_LEADING_PLUS_BIT + xioFLAG_LEADING_SPACE_BIT
	breq	95f								; Only fixed point types can have above
99:	rjmp	90b								;
	#endif	// USE_XQTOA
95:	cpi		r19, 'd'						; Is type signed decimal?
	ldi		r18, -10						;
	breq	41f								; /
	cpi		r19, 'u'						; Is type unsigned decimal?
	ldi		r18, 10							;
	breq	41f								; /
	cpi		r19, 'X'						; Is type hexadecimal?
	ldi		r18, 16							;
	breq	41f								; /
	cpi		r19, 'b'						; Is type binary?
	ldi		r18, 2							;
	brne	47f								; /
41:	X_movw	r20, r22						;
	sbrs	r26, xioFLAG_SHORT_BIT			;
	rjmp	42f								; Not 8 bit
	clr		r21								; 8 bit
	sbrs	r18, 7							; Signed value?
	rjmp	43f								;
	sbrc	r20, 7							; Copy sign bit if set
	ser		r21								;
	rjmp	43f								;
42:	sbrs	r26, xioFLAG_LONG_BIT			;
	rjmp	43f								; 16 bit
	ld		r22, Y+							; Get high word
	ld		r23, Y+							; /
	rjmp	45f								;
43:	clr		r22								;
	sbrs	r18, 7							; Signed value?
	rjmp	44f								;
	sbrc	r21, 7							; Copy sign bit if set
	ser		r22								;
44:	mov		r23, r22						;
45:	push	ZH								;
	push	ZL								;
	sbrc	r26, xioFLAG_ZERO_FILL_BIT		;
	neg		r16								;
	#if COUNT_CHARS
	push	r25								; Save output func pointer
	push	r24								; /
	#endif
	rcall	xitoa							;
	#if COUNT_CHARS
	rcall	xvcountchars					;
	pop		r24								; Restore output func pointer
	pop		r25								; /
	#endif
46:	pop		ZL								;
	pop		ZH								;
	rjmp	0b								; Head of parse loop
47:	mov		r20, r26						;
	andi	r20, xioFLAG_SHORT+xioFLAG_LONG	;
	#if USE_XQTOA
94:	brne	99b								; Only numeric types can be short/long
	#else
94:	brne	90b								; Only numeric types can be short/long
	#endif
	cpi		r19, 'c'						; Is type character? R22 is char to output
	brne	48f								;
	rjmp	1b								; /
	#if USE_UPPERCASEC
48:	cpi		r19, 'C'						; Is type ASCII [0x09-0x0D][0x20-0x7E]?
	brne	48f								;
	cpi		r22, 0x7F						; R22 is char to output
	brsh	11f								;
	cpi		r22, 0x20						;
	brsh	10f								;
	cpi		r22, 0x0E						;
	brsh	11f								;
	cpi		r22, 0x09						;
	brlo	11f								;
10:	rjmp	1b								; Output character
11:	rjmp	0b								; Ignore character
	#endif  // USE_UPPERCASEC
48:	cpi		r19, 's'						; Is type RAM string?
	brne	49f								;
	push	ZH								;
	push	ZL								;
	#if COUNT_CHARS
	push	r25								; Save output func pointer
	push	r24								; /
	#endif
	rcall	xputs							;
	#if COUNT_CHARS
	rcall	xvcountchars					;
	pop		r24								; Restore output func pointer
	pop		r25								; /
	#endif
	rjmp	46b								; /
49:	cpi		r19, 'S'						; Is type ROM string?
	brne	94b								; Invalid conversion character
	push	ZH								;
	push	ZL								;
	#if COUNT_CHARS
	push	r25								; Save output func pointer
	push	r24								; /
	#endif
	rcall	xputs_P							;
	#if COUNT_CHARS
	rcall	xvcountchars					;
	pop		r24								; Restore output func pointer
	pop		r25								; /
	#endif
	rjmp	46b								; /

#if USE_XQTOA
											; ----------------------------------------------
											; Output fixed point accum k/K/q/Q
											; ----------------------------------------------
											; Type k/K
;80:
50:	sbrc	r26, xioFLAG_UNSIGNED_BIT		; Check if unsigned
	rjmp	51f								; /
	ldi		r18, xioFRACBITS_K				; Load default signed fractional bits
	sbrc	r26, xioFLAG_SHORT_BIT			; Check if short
	ldi		r18, xioFRACBITS_HK				; /
	#if USE_XLQTOA
	sbrc	r26, xioFLAG_LONG_BIT			; Check if long
	ldi		r18, xioFRACBITS_LK				; /
	#endif
	rjmp	53f								;
51:	ldi		r18, xioFRACBITS_UK				; Load default unsinged fractional bits
	sbrc	r26, xioFLAG_SHORT_BIT			; Check if short
	ldi		r18, xioFRACBITS_UHK			; /
	#if USE_XLQTOA
	sbrc	r26, xioFLAG_LONG_BIT			; Check if long
	ldi		r18, xioFRACBITS_ULK			; /
	#endif
	rjmp	53f								;
											; Type q/Q
52:	mov		r18, r22						; Fractional bits
	ld		r22, Y+							; Get value (low word)
	ld		r23, Y+							; /
53:
	#if USE_XLQTOA == 0
	sbrc	r26, xioFLAG_LONG_BIT			;
	rjmp	90b								; Long k/K/q/Q (64 bit) not supported
	#endif // USE_XLQTOA == 0
	X_movw	r20, r22						; Move to bottom word
	sbrs	r26, xioFLAG_SHORT_BIT			;
	rjmp	55f								; Not 16 bit
	ldi		r22, 0							;
	sbrc	r26, xioFLAG_UNSIGNED_BIT		;
	rjmp	54f								; Unsigned
	tst		r21								; Copy signed bit if set
	brpl	54f								;
	ldi		r22, 0xFF						; /
54:	mov		r23, r22						;
	rjmp	56f								;
55:	ld		r22, Y+							; Get value (high word)
	ld		r23, Y+							; /
56:
	#if USE_XLQTOA
	cpi		r27, 0xFF						; Check if using default precision
	brne	57f								;
	ldi		r27, xioPRECISION_K				;
	sbrc	r26, xioFLAG_SHORT_BIT			;
	ldi		r27, xioPRECISION_HK			;
	sbrc	r26, xioFLAG_LONG_BIT			;
	ldi		r27, xioPRECISION_LK			;
57:	sbrs	r26, xioFLAG_LONG_BIT			;
	rjmp	58f								; Not 64 bit
	mov		r14, r18						; Frac bits
	mov		r12, r16						; Min width
	mov		r10, r26						; Flags
	mov		r8,  r27						; Precision
	X_movw	r16, r20						; 64 bit fixed point
	X_movw	r18, r22						;
	ld		r20, Y+							;
	ld		r21, Y+							;
	ld		r22, Y+							;
	ld		r23, Y+							;
	push	ZH								;
	push	ZL								;
	#if COUNT_CHARS
	push	r25								; Save output func pointer
	push	r24								; /
	#endif // COUNT_CHARS
	rcall	xlqtoa							;
	#if COUNT_CHARS
	rcall	xvcountchars					;
	pop		r24								; Restore output func pointer
	pop		r25								; /
	#endif // COUNT_CHARS
	rjmp	46b								;
	#else	// USE_XLQTOA
	cpi		r27, 0xFF						; Check if using default precision
	brne	58f								;
	ldi		r27, xioPRECISION_K				;
	sbrc	r26, xioFLAG_SHORT_BIT			;
	ldi		r27, xioPRECISION_HK			;
	#endif	// USE_XLQTOA
58:	mov		r14, r26						; Flags
	mov		r12, r27						; Precision
	push	ZH								; 32 bit fixed point
	push	ZL								;
	#if COUNT_CHARS
	push	r25								; Save output func pointer
	push	r24								; /
	#endif // COUNT_CHARS
	rcall	xqtoa							;
	#if COUNT_CHARS
	rcall	xvcountchars					;
	pop		r24								; Restore output func pointer
	pop		r25								; /
	#endif // COUNT_CHARS
	rjmp	46b								;

											; ----------------------------------------------
											; Output fixed point fract r/R
											; ----------------------------------------------
;90:
60:	ldi		r18, xioFRACBITS_R				; Load default fractional bits
	sbrc	r26, xioFLAG_SHORT_BIT			; Check if short
	ldi		r18, xioFRACBITS_HR				; /
	sbrc	r26, xioFLAG_LONG_BIT			; Check if long
	ldi		r18, xioFRACBITS_LR				; /
	sbrc	r26, xioFLAG_UNSIGNED_BIT		; Check if unsigned
	inc		r18								; /
	cpi		r27, 0xFF						; Check if using default precision
	brne	61f								;
	ldi		r27, xioPRECISION_R				; Load default precision
	sbrc	r26, xioFLAG_SHORT_BIT			; Check if short
	ldi		r27, xioPRECISION_HR			; /
	sbrc	r26, xioFLAG_LONG_BIT			; Check if long
	ldi		r27, xioPRECISION_LR			; /
61:	X_movw	r20, r22						; Move to bottom word
	sbrs	r26, xioFLAG_SHORT_BIT			;
	rjmp	63f								;
	ldi		r21, 0							; 8 bit fixed point
	sbrc	r26, xioFLAG_UNSIGNED_BIT		;
	rjmp	62f								; Unsigned
	tst		r20								; Copy signed bit if set
	brpl	62f								;
	ldi		r21, 0xFF						; /
62:	mov		r22, r21						;
	mov		r23, r21						;
	rjmp	58b								;
63:	sbrc	r26, xioFLAG_LONG_BIT			;
	rjmp	65f								;
	ldi		r22, 0							; 16 bit fixed point
	sbrc	r26, xioFLAG_UNSIGNED_BIT		;
	rjmp	64f								; Unsinged
	tst		r21								; Copy signed bit if set
	brpl	64f								;
	ldi		r22, 0xFF						;
64:	mov		r23, r22						;
	rjmp	58b								;
65:	ld		r22, Y+							; Get value (high word)
	ld		r23, Y+							; /
	rjmp	58b								;

#endif	// USE_XQTOA

.endfunc	// xioprintf_P

;----------------------------------------------------------------------------
; xprintf_P
; xvprintf_P
;----------------------------------------------------------------------------
.section .text.lib_xio.xprintf_P
.func xprintf_P
.global	xvprintf_P
xvprintf_P:
	set										; Pointer to arguments after format string
	rjmp	1f								;

.global xprintf_P
xprintf_P:
	clt										; Arguments are after format string
1:	push	YH								;
	push	YL								;
	in		YL, _SFR_IO_ADDR(SPL)			;
#ifdef SPH
	in		YH, _SFR_IO_ADDR(SPH)			;
#else
	clr		YH								;
#endif
	lds		r24, xfunc_out+0				; r25:r24 = output function
	lds		r25, xfunc_out+1				; /
	adiw	YL, 5							; Y = pointer to arguments
	rcall	xioprintf_P						;
	pop		YL								;
	pop		YH								;
	ret										;

.endfunc	// xprintf_P


#if USE_XSPRINTF_P

;----------------------------------------------------------------------------
; xsprintf_P
; xvsprintf_P
;
; Local Variables
;	r3:r2		Pointer to string buffer. Must not be modified by subroutines
;----------------------------------------------------------------------------
.section .text.lib_xio.xsprintf_P
.func xsprintf_P
.global	xvsprintf_P
xvsprintf_P:
	set										; Pointer to arguments after format string
	rjmp	1f								;

.global xsprintf_P

putram:
	X_movw	ZL, r2							;
	st		Z+, r24							;
	X_movw	r2, ZL							;
	ret										;

xsprintf_P:
	clt										; Arguments are after format string
1:	push	YH								;
	push	YL								;
	in		YL, _SFR_IO_ADDR(SPL)			;
#ifdef SPH
	in		YH, _SFR_IO_ADDR(SPH)			;
#else
	clr		YH								;
#endif
	adiw	YL, 5							; Y = pointer to arguments
	ldi		r24, lo8(pm(putram))			; Set local output function
	ldi		r25, hi8(pm(putram))			;
	push	r3								; Initialize pointer to string buffer
	push	r2								;
	ld		r2, Y+							;
	ld		r3, Y+							; /
	rcall	xioprintf_P						;
	X_movw	YL, r2							;
	st		Y, r1							; Terminate string
	pop		r2								;
	pop		r3								; /
	pop		YL								;
	pop		YH								;
	ret										;

.endfunc	// xvsprintf_P

#endif	// USE_XSPRINTF_P


#if USE_XSNPRINTF_P

;----------------------------------------------------------------------------
; xsnprintf_P
; xvsnprintf_P
;
;Local Variables
;	r5:r4		Remaining bytes in buffer. Must not be modified by subroutines
;	r3:r2		Pointer to string buffer. Must not be modified by subroutines
;----------------------------------------------------------------------------
.section .text.lib_xio.xsnprintf_P
.func xsnprintf_P
.global	xvsnprintf_P
xvsnprintf_P:
	set										; Pointer to arguments after format string
	rjmp	03f								;

.global xsnprintf_P

putnram:
	X_movw	ZL, r4							; Check if space for another byte
	or		ZL, ZH							;
	breq	01f								; /
	sbiw	ZL, 1							; Decrement remaining bytes
	X_movw	r4, ZL							; /
	or		ZL, ZH							; If last byte then set to 0x00
	breq	02f								; /
	X_movw	ZL, r2							; Place byte into buffer
	st		Z+, r24							;
	X_movw	r2, ZL							; /
01:	ret										;
02: st		Z, r1							; Place 0x00 into last position in buffer
	ret										;

xsnprintf_P:
	clt										; Arguments are after format string
03:	push	YH								;
	push	YL								;
	in		YL, _SFR_IO_ADDR(SPL)			;
#ifdef SPH
	in		YH, _SFR_IO_ADDR(SPH)			;
#else
	clr		YH								;
#endif
	adiw	YL, 5							; Y = pointer to arguments
	ldi		r24, lo8(pm(putnram))			; Set local output function
	ldi		r25, hi8(pm(putnram))			;
	push	r3								; Initialize pointer to string buffer
	push	r2								;
	ld		r2, Y+							;
	ld		r3, Y+							; /
	push	r5								; Initialize buffer size
	push	r4								;
	ld		r4, Y+							;
	ld		r5, Y+							; /
	rcall	xioprintf_P						;
	X_movw	YL, r2							;
	st		Y, r1							; Terminate string
	pop		r4								;
	pop		r5								;
	pop		r2								;
	pop		r3								;
	pop		YL								;
	pop		YH								;
	ret										;

.endfunc	// xvsnprintf_P

#endif	// USE_XSNPRINTF_P


#if USE_XFPRINTF_P

;----------------------------------------------------------------------------
; xfprintf_P
;
;----------------------------------------------------------------------------
.section .text.lib_xio.xfprintf_P
.func xfprintf_P
.global xfprintf_P
xfprintf_P:
	clt										; Arguments are after format string
	push	YH								;
	push	YL								;
	in		YL, _SFR_IO_ADDR(SPL)			;
#ifdef SPH
	in		YH, _SFR_IO_ADDR(SPH)			;
#else
	clr		YH								;
#endif
	adiw	YL, 5							; Y = pointer to arguments
	ld		r24, Y+							; Output function
	ld		r25, Y+							;
	rcall	xioprintf_P						;
	pop		YL								;
	pop		YH								;
	ret										;

.endfunc

#endif	// USE_XFPRINTF_P

#endif	// USE_XPRINTF_P


;----------------------------------------------------------------------------
; Extended direct numeral string output (32bit version)
;
; Prototype: void xitoa (void (*xfunc_out)(uint8_t),	// output function
; 						 long value,					// value to be output
;                        char radix,					// radix
;                        char width);					// minimum width
;
; Arguments:
;	r25:r24		pointer to xfunc_out
;	r23:r20		32 bit value
;		r18		radix
;		r16		minimum width
;	r7: r6		Count of chars output if COUNT_CHARS is nonzero
;
; Returns:
;	r24:r25		int
;	If COUNT_CHARS is nonzero then the number of chars output is returned
;	or a negative value if there was an error.
;----------------------------------------------------------------------------
.section .text.lib_xio.xitoa
.func xitoa
.global xitoa
#if COUNT_CHARS
xicountchars:
	cpi		r25, 0xFF						; Assume char count never over 0xFEFF
	breq	1f								;
	add		r6, r24							; Add to running total
	adc		r7, r25							; /
	ret										;
1:	pop		r0								; Trash return address
	pop		r0								; /
	pop		r0								; r25:r24
	pop		r0								; /
	rjmp	90f								;
#endif

xitoa:
											; r25:24 = output func, r23:r20 = value, r18 = base, r16 = digits, r27 = filler
	push	r16								;
	#if COUNT_CHARS
	push	r7								;
	push	r6								;
	clr		r7								; Zero char count
	clr		r6								; /
	#endif
	clr		r31								; r31 = stack level
	ldi		r30, ' '						; r30 = sign
	ldi		r27, ' '						; r27 = filler
	sbrs	r18, 7							; When base indicates signed format and the value
	rjmp	0f								; is minus, add a '-'.
	neg		r18								;
	sbrs	r23, 7							;
	rjmp	0f								;
	ldi		r30, '-'						;
	com		r20								;
	com		r21								;
	com		r22								;
	com		r23								;
	adc		r20, r1							;
	adc		r21, r1							;
	adc		r22, r1							;
	adc		r23, r1							; /
0:	sbrs	r16, 7							; When digits indicates zero filled,
	rjmp	1f								; filler is '0'.
	neg		r16								;
	ldi		r27, '0'						; /

											; ----- string conversion loop
1:	ldi		r19, 32							; r26 = r23:r20 % r18
	clr		r26								; r23:r20 /= r18
2:	lsl		r20								;
	rol		r21								;
	rol		r22								;
	rol		r23								;
	rol		r26								;
	cp		r26, r18						;
	brcs	3f								;
	sub		r26, r18						;
	inc		r20								;
3:	dec		r19								;
	brne	2b								; /
	cpi		r26, 10							; r26 is a numeral digit '0'-'F'
	brcs	4f								;
	subi	r26, -7							;
4:	subi	r26, -'0'						; /
	push	r26								; Stack it
	inc		r31								; /
	cp		r20, r1							; Repeat until r23:r20 gets zero
	cpc		r21, r1							;
	cpc		r22, r1							;
	cpc		r23, r1							;
	brne	1b								; /

	cpi		r30, '-'						; Minus sign if needed
	brne	5f								;
	push	r30								;
	inc		r31								; /
5:	cp		r31, r16						; Filler
	brcc	6f								;
	push	r27								;
	inc		r31								;
	rjmp	5b								; /

6:	pop		r22								; Flush stacked digits and exit
	#if COUNT_CHARS
	push	r25								;
	push	r24								;
	#endif
	rcall	xputc							;
	#if COUNT_CHARS
	rcall	xicountchars					;
	pop		r24								;
	pop		r25								;
	#endif
	dec		r31								;
	brne	6b								; /

	#if COUNT_CHARS
	X_movw	r24,r6							; Return char count r25:r24

90:	pop		r6								;
	pop		r7								;
	#endif
	pop		r16								;
	ret										;

.endfunc	// xitoa

#if USE_XQTOA || USE_XLQTOA

.section .text
xio_ovrflw:
	.asciz	"OVERFLOW"

#endif // USE_XQTOA || USE_XLQTOA

#if USE_XQTOA
;----------------------------------------------------------------------------
; Extended direct fixed point string output (32 bit version)
;
; Prototype: void xqtoa (void (*xfunc_out)(uint8_t),// output function
;						 long fp,					// fixed point value to be output
;						 char fbits,				// number of bits in fraction
;						 char width,				// minimum width to print value, max 127
;						 char flags,				// format flags
;						 char precision)			// number of digits after decimal
;
; Arguments:
;	r25:r24		pointer to xfunc_out
;	r23:r20		32 bit fixed point value
;		r18		number of bits in fraction
;		r16		minimum width
;		r14		format flags
;		r12		precision
;
; Local Variables:
;		T		Set if fp is negative
;		r31		Holds CD or CC for multiply
;		r30		0x0A for multiply. Width
;	r29:r28		char count if COUNT_CHARS is nonzero
;		r27		Precision
;		r26		Temporary
;		r23		Temporary
;		r22		Char for xputc
;		r21		Format flags
;		r20		Count of digits on stack
;		r19		Minimum width
;	r17:r14		fp
;	r13:r10		Mask, rounding constant
;	r9 :r6		fp from r23:r20
;
;	r5 :r4		Buffer size for xsnprintf_P
;	r3 :r2		Pointer to buffer for xsprintf_P
;
; Returns:
;	r24:r25		int
;	If COUNT_CHARS is nonzero then the number of chars output is returned
;	or a negative value if there was an error.
;----------------------------------------------------------------------------
.section .text.lib_xio.xqtoa
.func xqtoa
.global xqtoa

;Rounding constants in q.32 unsigned fixed point format
rounding_const32:
;			LSB				  MSB
	.byte	0x00, 0x00, 0x00, 0x80						; 0 digits	0.5
	.byte	0xCC, 0xCC, 0xCC, 0x0C						; 1 digit	0.05
	.byte	0x14, 0xAE, 0x47, 0x01						; 2 digits	0.005
	.byte	0x99, 0xC4, 0x20, 0x00						; 3 digits	0.0005
	.byte	0xDC, 0x46, 0x03, 0x00						; 4 digits	0.00005
	.byte	0xE2, 0x53, 0x00, 0x00						; 5 digits	0.000005
	.byte	0x63, 0x08, 0x00, 0x00						; 6 digits	0.0000005
	.byte	0xD6, 0x00, 0x00, 0x00						; 7 digits	0.00000005
	.byte	0x15, 0x00, 0x00, 0x00						; 8 digits	0.000000005

xputc32:
	push	r18								;
	push	r19								;
	push	r20								;
	push	r21								;
	push	r22								;
	push	r23								;
	push	r26								;
	push	r27								;
	#if COUNT_CHARS
	push	r24								;
	push	r25								;
	#endif // COUNT_CHARS
	rcall	xputc							;
	#if COUNT_CHARS
	cpi		r25, 0xFF						; Assume char count never over 0xFEFF
	breq	1f								;
	add		r28, r24						; Add to running total
	adc		r29, r25						; /
	clz										; Ensure instruction breq 4f does not branch
	rjmp	2f								;
1:	pop		r0								; Trash return address
	pop		r0								; /
	pop		r0								; Trash r25:r24
	pop		r0								; /
	rjmp	3f								;
2:	pop		r25								;
	pop		r24								;
	#endif // COUNT_CHARS
3:	pop		r27								;
	pop		r26								;
	pop		r23								;
	pop		r22								;
	pop		r21								;
	pop		r20								;
	pop		r19								;
	pop		r18								;
	breq	4f								;
	ret										;
4:	rjmp	90f								;

#if OPTIMIZE_SPEED
ror32:
	; r13:r10	fp
	; r23		shift count
00:	cpi		r23, 24							; Shift by 24 or more bits
	brlo	00f								;
	mov		r10, r13						;
	clr		r11								;
	clr		r12								;
	clr		r13								;
	subi	r23, 24							;
	breq	02f								;
01:	lsr		r10								; Shift bits right
	dec		r23								;
	brne	01b								; /
02:	ret										;

00:	cpi		r23, 16							; Shift by 16 or more bits
	brlo	00f								;
	X_movw	r10, r12						;
	clr		r12								;
	clr		r13								;
	subi	r23, 16							;
	breq	02f								;
01:	lsr		r11								; Shift bits right
	ror		r10								;
	dec		r23								;
	brne	01b								; /
02:	ret										;

00:	cpi		r23, 8							; Shift by 8 or more bits
	brlo	00f								;
	X_movw	r10, r11						;
	mov		r12, r13						;
	clr		r13								;
	subi	r23, 8							;
	breq	02f								;

00:	cpi		r23, 0							; Shift by 7 or less bits
	breq	02f								;
01:	lsr		r13								; Shift bits right
	ror		r12								;
	ror		r11								;
	ror		r10								;
	dec		r23								;
	brne	01b								; /
02:	ret										;
#else // OPTIMIZE_SPEED
	; r13:r10	fp
	; r23		shift count
ror32:										;
	cpi		r23, 0							;
	breq	02f								;
01:	lsr		r13								; Shift bits right
	ror		r12								;
	ror		r11								;
	ror		r10								;
	dec		r23								;
	brne	01b								;
02:	ret										;
#endif // OPTIMIZE_SPEED

#if OPTIMIZE_SPEED
mask32:
	; r13:r10	fp
	; r23		shift count
	; r13-r11 are already set to zero
	ser		r26								;
00:	cpi		r23, 24							; Shift by 24 or more bits
	brlo	00f								;
	mov		r10, r26						;
	mov		r11, r26						;
	mov		r12, r26						;
;	clr		r13								;
	subi	r23, 24							;
	breq	02f								;
01:	sec										; Left shift in a 1
	rol		r13								;
	dec		r23								;
	brne	01b								; /
02:	ret										;

00:	cpi		r23, 16							; Shift by 16 or more bits
	brlo	00f								;
	mov		r10, r26						;
	mov		r11, r26						;
;	clr		r12								;
;	clr		r13								;
	subi	r23, 16							;
	breq	02f								;
01:	sec										; Left shift in a 1
	rol		r12								;
	dec		r23								;
	brne	01b								; /
02:	ret										;

00:	cpi		r23, 8							; Shift by 8 or more bits
	brlo	00f								;
	mov		r10, r26						;
;	clr		r11								;
;	clr		r12								;
;	clr		r13								;
	subi	r23, 8							;
	breq	02f								;
01:	sec										; Left shift in a 1
	rol		r11								;
	dec		r23								;
	brne	01b								; /
02:	ret										;

00:	cpi		r23, 0							; Shift by 7 or less bits
	breq	02b								;
	clr		r10								; 
;	clr		r11								;
;	clr		r12								;
;	clr		r13								;
01:	sec										; Left shift in a 1
	rol		r10								;
	dec		r23								;
	brne	01b								; /
02:	ret										;
#else // OPTIMIZE_SPEED
	; r13:r10	fp
	; r23		shift count
	; r13-r11 are already set to zero
mask32:										;
	cpi		r23, 0							;
	breq	02f								;
	clr		r10								; 
01:	sec										; Left shift in a 1
	rol		r10								;
	rol		r11								;
	rol		r12								;
	rol		r13								;
	dec		r23								;
	brne	01b								; /
02:	ret										;
#endif // OPTIMIZE_SPEED

#if COUNT_CHARS
xqcountchars:
	cpi		r25, 0xFF						; Assume char count never over 0xFEFF
	breq	1f								;
	add		r28, r24						; Add to running total
	adc		r29, r25						; /
	ret										;
1:	pop		r0								; Trash return address
	pop		r0								; /
	pop		r0								; r25:r24
	pop		r0								; /
	rjmp	91f								;
#endif

xqtoa:
	push	r6								;
	push	r7								;
	push	r8								;
	push	r9								;
	push	r10								;
	push	r11								;
	push	r12								;
	push	r13								;
	push	r14								;
	push	r15								;
	push	r16								;
	push	r17								;
	#if COUNT_CHARS
	push	r28								;
	push	r29								;

	clr		r29								; Zero char counter r29:r28
	clr		r28								; /
	#endif

	X_movw	r6, r20							; r9:r6 is now the fp value
	X_movw	r8, r22							; /
	mov		r19, r16						; r19 is now minimum width
	mov		r21, r14						; r21 is now format flags
	mov		r27, r12						; r27 is now precision

											; Convert fp to positive if negative
01:	clt										; Mark fp as positive
	sbrc	r21, xioFLAG_UNSIGNED_BIT
	rjmp	10f								; fp is unsigned
	and		r9, r9							;
	brge	10f								; fp is positive
	com		r9								; One's compliment of fp
	com		r8								;
	com		r7								;
	com		r6								;
	adc		r6, r1							; Two's compliment of fp
	adc		r7, r1							;
	adc		r8, r1							;
	adc		r9, r1							;
	set										; Mark fp as negative

10:	sbrc	r21, xioFLAG_NO_ROUNDING_BIT
	rjmp	30f								; No Rounding

											; Round decimals
	mov		r23, r27						; Get precision
	cpi		r23, 9							; Only round on precision of 8 or less
	brsh	30f
	lsl		r23								; Calculate offset into table
	lsl		r23								;
	ldi		ZL, lo8(rounding_const32)		;
	ldi		ZH, hi8(rounding_const32)		;
	add		ZL, r23							;
	adc		ZH, r1							;
	X_lpm	r10, Z+							; Load rounding constant
	X_lpm	r11, Z+							;
	X_lpm	r12, Z+							;
	X_lpm	r13, Z+							; /

20:	ldi		r23, 0x20						; Max fbits is 32
	sub		r23, r18						; Bits to shift rounding constant
	rcall	ror32							;

	add		r6, r10							; Add constant to fp
	adc		r7, r11							;
	adc		r8, r12							;
	adc		r9, r13							; /

	brcc	30f								; Check for overflow
	ldi		r22, lo8(xio_ovrflw)			;
	ldi		r23, hi8(xio_ovrflw)			;
	#if COUNT_CHARS
	push	r25								;
	push	r24								;
	#endif
	rcall	xputs_P							;
	#if COUNT_CHARS
	rcall	xqcountchars					;
	pop		r24								;
	pop		r25								;
	#endif
	rjmp	90f								;

											; Push integer digits onto stack
30:	clr		r20								; Initialize size of integer on stack
	X_movw	r10, r6							;
	X_movw	r12, r8							;
	mov		r23, r18						; Get fractional bits
	rcall	ror32							;

	; 32x32_Top32: Multiply 32 by 32 bits. Keep top 32 bits.
	; The table below shows r11:r8 = r29:r26 * 0xCCCCCCCD
	; The entries in each row show which registers that
	; r1:r0 are added to after a multiply. The first row
	; shows that after multiplying r26 by 0xCD that r1 is
	; added to r9 and r0 is discarded. A 'c' means that
	; the carry bit is added to the register
	;
	; Row 17 16 15 14   32
	;  1   0  0 R1  0   Lo   R10*CD R1 copied to R15. R14, R16, R17 zeroed
	;  2     R1:R0      Lo   R11*CD
	;  3  R1:R0         Lo   R12*CD
	;  4  R0       R1   L/H  R13*CD
	;  5   c R1:R0      Lo   R10*CC
	;  6  R1:R0  0  c   Lo   R10*CC Reuse R1:R0 R7 zeroed
	;  7  R0     c R1   L/H  R10*CC Reuse R1:R0
	;  8  R1:R0     c   Lo   R11*CC
	;  9  R0  0  c R1   L/H  R11*CC Reuse R1:R0 R8 zeroed
	; 10        R1:R0   Hi   R11*CC Reuse R1:R0
	; 11  R0     c R1   L/H  R12*CC
	; 12   0  c R1:R0   Hi   R12*CC Reuse R1:R0 R9 zeroed
	; 13     R1:R0      Hi   R12*CC Reuse R1:R0
	; 14      c R1:R0   Hi   R13*CC
	; 15   c R1:R0      Hi   R13*CC Reuse R1:R0
	; 16  R1:R0         Hi   R13*CC Reuse R1:R0

	; r17:r14 = r13:r10 / 10
	; This is done by multiplying integer with 0xCCCCCCCD
	; then shifting right 3 bits.
	ldi		r30, 0x0A						;
	clr		r23								; Used as zero
	ldi		r26, '0'						; Used to convert decimal to ASCII
	
33:											; Head of loop
	ldi		r31, 0xCD						;
	mul		r10, r31						; Row 1
	mov		r15, r1							;
	clr		r14								;
	clr		r16								;
	clr		r17								;

	mul		r11, r31						; Row 2
	add		r15, r0							;
	adc		r16, r1							;
;	adc		r17, r23						; Carry needed?

	mul		r12, r31						; Row 3
	add		r16, r0							;
	adc		r17, r1							;
;	adc		r14, r23						; Carry needed?

	mul		r13, r31						; Row 4
	add		r17, r0							;
	adc		r14, r1							;
;	adc		r15, r23						; Carry needed?

	ldi		r31, 0xCC						;

	mul		r10, r31						; Row 5
	add		r15, r0							;
	adc		r16, r1							;
	adc		r17, r23						; Carry needed?

;	mul		r10, r31						; Row 6
	add		r16, r0							;
	adc		r17, r1							;
	adc		r14, r23						; Carry needed?

	clr		r15								;

;	mul		r10, r31						; Row 7
	add		r17, r0							;
	adc		r14, r1							;
	adc		r15, r23						; Carry needed?

	mul		r11, r31						; Row 8
	add		r16, r0							;
	adc		r17, r1							;
	adc		r14, r23						; Carry needed?

;	mul		r11, r31						; Row 9
	add		r17, r0							;
	adc		r14, r1							;
	adc		r15, r23						; Carry needed?

	clr		r16								;

;	mul		r11, r31						; Row 10
	add		r14, r0							;
	adc		r15, r1							;
;	adc		r16, r23						; Carry needed?

	mul		r12, r31						; Row 11
	add		r17, r0							;
	adc		r14, r1							;
	adc		r15, r23						; Carry needed?

;	mul		r12, r31						; Row 12
	add		r14, r0							;
	adc		r15, r1							;
	adc		r16, r23						; Carry needed?

	clr		r17								;

;	mul		r12, r31						; Row 13
	add		r15, r0							;
	adc		r16, r1							;
;	adc		r17, r23						; Carry needed?

	mul		r13, r31						; Row 14
	add		r14, r0							;
	adc		r15, r1							;
	adc		r16, r23						; Carry needed?

;	mul		r13, r31						; Row 15
	add		r15, r0							;
	adc		r16, r1							;
	adc		r17, r23						; Carry needed?

;	mul		r13, r31						; Row 16
	add		r16, r0							;
	adc		r17, r1							;

											; ror 3 bits
	lsr		r17								;
	ror		r16								;
	ror		r15								;
	ror		r14								;
	lsr		r17								;
	ror		r16								;
	ror		r15								;
	ror		r14								;
	lsr		r17								;
	ror		r16								;
	ror		r15								;
	ror		r14								;

											; digit = LSB(r13:r10) - LSB(LSB(r17:r14) * 0x0A)
	mul		r14, r30						; r1:r0 = r14 * 10
	mov		r1, r26							; r19 = '0'
	sub		r1, r0							;
	add		r1, r10							; Convert to ASCII
	push	r1								;
	inc		r20								;

	and		r14, r14						; More digits if r11:r8 > 0
	brne	34f								;
	and		r15, r15						;
	brne	34f								;
	and		r16, r16						;
	brne	34f								;
	and		r16, r16						;
	brne	34f								;
	rjmp	35f								; No more digits

							
34:	X_movw	r10, r14						;
	X_movw	r12, r16						;
	rjmp	33b								;

35:	clr		r1								;
											; Determine if leading sign needed
	brtc	40f								; fp not negative
	ldi		r22, '-'						;
	rjmp	42f								;
40:	sbrs	r21, xioFLAG_LEADING_PLUS_BIT	;
	rjmp	41f								;
	ldi		r22, '+'						; Leading plus for positive fp
	rjmp	42f								;
41:	sbrs	r21, xioFLAG_LEADING_SPACE_BIT	;
	rjmp	44f								;
	ldi		r22, ' '						; Leading space for positive fp
42:	sbrs	r21, xioFLAG_ZERO_FILL_BIT		;
	rjmp	43f								;
	rcall	xputc32							;
	dec		r19								; Decrease width by 1
	rjmp	44f								;
43:	push	r22								; Save leading sign on stack
	inc		r20								;

44:	ldi		r22, ' '						; Default filler
	sbrc	r21, xioFLAG_ZERO_FILL_BIT
	ldi		r22, '0'						; Zero filler
	
											; Determine size of fp in ASCII
	mov		r0, r20							; Get size of integer on stack
	cpse	r27, r1							; Skip next instruction if precision is zero
	inc		r0								; Include decimal point
	add		r0, r27							; Add precision
	cp		r0, r19							; Compare to minimum width
	brsh	48f								; Exceeds or equal minimum

											; Check justification
	mov		r23, r19						; Minimum width
	sub		r23, r0							; r23 = min width - size of fp in ASCII
	sbrc	r21, xioFLAG_LEFT_JUSTIFIED_BIT	;
	rjmp	47f								;
46:	rcall	xputc32							;
	dec		r23								;
	brne	46b								;
47:	mov		r19, r23						; r19 = remaining padding needed
	rjmp	49f								;

48:	clr		r19								; r19 = no padding needed


49:	pop		r22								; Flush stacked digits
	rcall	xputc32							;
	dec		r20								;
	brne	49b								;

	and		r27, r27						; Any digits after decimal?
	brne	99f								;
	rjmp	80f								;

99:	ldi		r22, '.'						; Output decimal point
	rcall	xputc32							;

50:											; Calculate mask for fbits
	mov		r23, r18						; r23 = fbits
	rcall	mask32							; Mask = (1 << fbits) - 1

	ldi		r23, 0x0A						;

53:	and		r6, r10							; Get fractional part of fp
	and		r7, r11							;
	and		r8, r12							;
	and		r9, r13							;

	mul		r6, r23							; Multiply fp by 10
	X_movw	r14, r0							;
	clr		r16								;
	clr		r17								;
	mul		r7, r23							;
	add		r15, r0							;
	adc		r16, r1							;
	mul		r8, r23							;
	add		r16, r0							;
	adc		r17, r1							;
	mul		r9, r23							;
	add		r17, r0							;
	clr		r0								;
	adc		r1, r0							;

	X_movw	r6, r14							;
	X_movw	r8, r16							;

											; Get integer
	mov		r26, r18						; r26 = frac bits
#if OPTIMIZE_SPEED
	cpi		r26, 32							; Shift by 32 or more bits
	brlo	54f								;
	mov		r22, r1							; Get digit
	rjmp	58f								;

54: cpi		r26, 24							; Shift by 24 or more bits
	brlo	54f								;
	subi	r26, 24							;
	breq	56f								;
55:	lsr		r1								;
	ror		r17								;
	dec		r26								;
	brne	55b								;
56:	mov		r22, r17						; Get digit
	rjmp	58f								;

54: cpi		r26, 16							; Shift by 16 or more bits
	brlo	54f								;
	subi	r26, 16							;
	breq	56f								;
55:	lsr		r17								;
	ror		r16								;
	dec		r26								;
	brne	55b								;
56:	mov		r22, r16						; Get digit
	rjmp	58f								;

54: cpi		r26, 8							; Shift by 8 or more bits
	brlo	54f								;
	subi	r26, 8							;
	breq	56f								;
55:	lsr		r16								;
	ror		r15								;
	dec		r26								;
	brne	55b								;
56:	mov		r22, r15						; Get digit
	rjmp	58f								;

54:	cpi		r26, 0							;  Shift by 7 or less bits 
	breq	56f								;
55:	lsr		r15								;
	ror		r14								;
	dec		r26								;
	brne	55b								;
56:	mov		r22, r14						; Get digit
#else // OPTIMIZE_SPEED
	cpi		r26, 0							;
	breq	56f								;
55:	lsr		r1								;
	ror		r17								;
	ror		r16								;
	ror		r15								;
	ror		r14								;
	dec		r26								; 
	brne	55b								;
56:	mov		r22, r14						; Get digit
#endif // OPTIMIZE_SPEED

58:	subi	r22, -('0')						; Output digit
	rcall	xputc32							;

	dec		r27								; Check precision
	breq	59f								;
	rjmp	53b								;

59:	clr		r1								;

80:	cpi		r19, 0							; Output any remaining padding
	breq	90f								;
	mov		r30, r19						;
81:	ldi		r22, ' '						;
	#if COUNT_CHARS
	push	r25								;
	push	r24								;
	#endif
	rcall	xputc							;
	#if COUNT_CHARS
	rcall	xqcountchars					;
	pop		r24								;
	pop		r25								;
	#endif
	dec		r30								;
	brne	81b								;

90:
	#if COUNT_CHARS
	X_movw	r24, r28						; Return char count

91:	pop		r29								;
	pop		r28								;
	#endif
	pop		r17								;
	pop		r16								;
	pop		r15								;
	pop		r14								;
	pop		r13								;
	pop		r12								;
	pop		r11								;
	pop		r10								;
	pop		r9								;
	pop		r8								;
	pop		r7								;
	pop		r6								;
	ret										;

.endfunc	// xqtoa

#endif // USE_XQTOA


#if USE_XLQTOA
;----------------------------------------------------------------------------
; Extended direct fixed point string output (64 bit version)
;
; Prototype: void xlqtoa (void (*xfunc_out)(uint8_t),	// output function
;						  long long fp,					// fixed point value to be output
;						  char fbits,					// number of bits in fraction
;						  char width,					// minimum width to print value, max 127
;						  char flags,					// format flags
;						  char precision)				// number of digits after decimal
;
; Arguments:
;	r25:r24		pointer to xfunc_out
;	r23:r16		64 bit fixed point value
;		r14		number of bits in fraction
;		r12		minimum width
;		r10		format flags
;		r8		precision
;
; Local Variables:
;		T		Set if fp is negative
;		r31		Holds CC & CD for multiply
;		r30		Count of digits in interger
;		r28		Fractional bits
;		r23		Shift count, temp
;		r22		char for xputc
;		r21		Min width
;		r20		Format flags
;		r19		Precision
;		r18		10 for division
;	r17:r10		FP: rounding const, integer
;	r9 :r2		FP
;
;	Mask		r31,r30,r29,r27,r26,r21,r20,r18
;
;	r5 :r4		Buffer size for xsnprintf_P
;	r3 :r2		Pointer to buffer for xsprintf_P
;----------------------------------------------------------------------------
#if COUNT_CHARS
#error "xlqtoa does not support returning number of chars output"
#endif

#if USE_XSPRINTF_P
#warning "xlqtoa and printf functions that use 64 bit fixed point are not reentrant"
.section .bss.lib_xio
.global xioR3R2
xioR3R2:	.ds.b	2						; Holder for xsprintf_P/xsnprintf_P buffer pointer (r3:r2)
#endif

#if USE_XSNPRINTF_P
.section .bss.lib_xio
.global xioR5R4
xioR5R4:	.ds.b	2						; Holder for xsnprintf_P buffer size (r5:r4)
#endif

.section .text.lib_xio.xlqtoa
.func xlqtoa
.global xlqtoa

;Rounding constants in q.64 unsigned fixed point format
rounding_const64:
;			LSB										  MSB
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80	; 0 digits	0.5
	.byte	0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x0C	; 1 digit	0.05
	.byte	0xAE, 0x47, 0xE1, 0x7A, 0x14, 0xAE, 0x47, 0x01	; 2 digits	0.005
	.byte	0xF7, 0x53, 0xE3, 0xA5, 0x9B, 0xC4, 0x20, 0x00	; 3 digits	0.0005
	.byte	0x65, 0x88, 0x63, 0x5D, 0xDC, 0x46, 0x03, 0x00	; 4 digits	0.00005
	.byte	0xA3, 0x8D, 0x23, 0xD6, 0xE2, 0x53, 0x00, 0x00	; 5 digits	0.000005
	.byte	0xF6, 0x5A, 0xD0, 0x7B, 0x63, 0x08, 0x00, 0x00	; 6 digits	0.0000005
	.byte	0xE5, 0xD5, 0x94, 0xBF, 0xD6, 0x00, 0x00, 0x00	; 7 digits	0.00000005
	.byte	0x30, 0xE2, 0x8E, 0x79, 0x15, 0x00, 0x00, 0x00	; 8 digits	0.000000005
	.byte	0x04, 0x7D, 0xC1, 0x25, 0x02, 0x00, 0x00, 0x00	; 9 digits
	.byte	0xB3, 0xBF, 0xF9, 0x36, 0x00, 0x00, 0x00, 0x00	; 10 digits
	.byte	0xF8, 0x5F, 0x7F, 0x05, 0x00, 0x00, 0x00, 0x00	; 11 digits
	.byte	0xCC, 0xBC, 0x8C, 0x00, 0x00, 0x00, 0x00, 0x00	; 12 digits
	.byte	0xE1, 0x12, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00	; 13 digits
	.byte	0x49, 0x68, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00	; 14 digits
	.byte	0x07, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	; 15 digits
	.byte	0x9A, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	; 16 digits
	.byte	0x5C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	; 17 digits
	.byte	0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	; 18 digits

xputc64:
#if USE_XSPRINTF_P
	push	r2								; xsprintf_P uses r3:r2 as pointer to the
	push	r3								; string buffer.
	lds		r2, xioR3R2						;
	lds		r3, xioR3R2+1					;
#endif
#if USE_XSNPRINTF_P
	push	r4								; xsnprintf_P uses r5:r4 as buffer size
	push	r5								;
	lds		r4, xioR5R4						;
	lds		r5, xioR5R4+1					;
#endif
	push	r18								;
	push	r19								;
	push	r20								;
	push	r21								;
	push	r22								;
	push	r23								;
	push	r26								;
	push	r27								;
	rcall	xputc							;
	pop		r27								;
	pop		r26								;
	pop		r23								;
	pop		r22								;
	pop		r21								;
	pop		r20								;
	pop		r19								;
	pop		r18								;
#if USE_XSNPRINTF_P
	sts		xioR5R4, r4						;
	sts		xioR5R4+1, r5					;
	pop		r5								;
	pop		r4								;
#endif
#if USE_XSPRINTF_P
	sts		xioR3R2, r2						;
	sts		xioR3R2+1, r3					;
	pop		r3								;
	pop		r2								;
#endif
	ret										;

#if OPTIMIZE_SPEED
ror64:
	; r17:r10	fp
	; r23		shift count
	cpi		r23, 56							; Shift by 56 or more bits
	brlo	00f								;
	mov		r10, r17						;
	clr		r11								;
	clr		r12								;
	clr		r13								;
	X_movw	r16, r12						;
	X_movw	r14, r12						;
	subi	r23, 56							;
	breq	02f								;
01:	lsr		r10								; Shift bits right
	dec		r23								;
	brne	01b								;
02:	ret										; /

00:	cpi		r23, 48							; Shift by 48 or more bits
	brlo	00f								;
	X_movw	r10, r16						;
	clr		r12								;
	clr		r13								;
	X_movw	r16, r12						;
	X_movw	r14, r12						;
	subi	r23, 48							;
	breq	02f								;
01:	lsr		r11								; Shift bits right
	ror		r10								;
	dec		r23								;
	brne	01b								;
02:	ret										; /

00:	cpi		r23, 40							; Shift by 40 or more bits
	brlo	00f								;
	X_movw	r10, r15						;
	mov		r12, r17						;
	clr		r13								;
	clr		r14								;
	clr		r15								;
	X_movw	r16, r14						;
	subi	r23, 40							;
	breq	02f								;
01:	lsr		r12								; Shift bits right
	ror		r11								;
	ror		r10								;
	dec		r23								;
	brne	01b								;
02:	ret										; /

00:	cpi		r23, 32							; Shift by 32 or more bits
	brlo	00f								;
	X_movw	r10, r14						;
	X_movw	r12, r16						;
	clr		r14								;
	clr		r15								;
	X_movw	r16, r14						;
	subi	r23, 32							;
	breq	02f								;
01:	lsr		r13								; Shift bits right
	ror		r12								;
	ror		r11								;
	ror		r10								;
	dec		r23								;
	brne	01b								;
02:	ret										; /

00:	cpi		r23, 24							; Shift by 24 or more bits
	brlo	00f								;
	X_movw	r10, r13						;
	X_movw	r12, r15						;
	mov		r14, r17						;
	clr		r15								;
	clr		r16								;
	clr		r17								;
	subi	r23, 24							;
	breq	02f								;
01:	lsr		r14								; Shift bits right
	ror		r13								;
	ror		r12								;
	ror		r11								;
	ror		r10								;
	dec		r23								;
	brne	01b								;
02:	ret										; /

00:	cpi		r23, 16							; Shift by 16 or more bits
	brlo	00f								;
	X_movw	r10, r12						;
	X_movw	r12, r14						;
	X_movw	r14, r16						;
	clr		r16								;
	clr		r17								;
	subi	r23, 16							;
	breq	02f								;
01:	lsr		r15								; Shift bits right
	ror		r14								;
	ror		r13								;
	ror		r12								;
	ror		r11								;
	ror		r10								;
	dec		r23								;
	brne	01b								;
02:	ret										; /

00:	cpi		r23, 8							; Shift by 8 or more bits
	brlo	02f								;
	X_movw	r10, r11						;
	X_movw	r12, r13						;
	X_movw	r14, r15						;
	mov		r16, r17						;
	clr		r17								;
	subi	r23, 8							;
	breq	02f								;

01:	lsr		r17								; Shift bits right
	ror		r16								;
	ror		r15								;
	ror		r14								;
	ror		r13								;
	ror		r12								;
	ror		r11								;
	ror		r10								;
	dec		r23								;
02:	brne	01b								;
03:	ret										; /
#else // OPTIMIZE_SPEED
	; r17:r10	fp
	; r23		shift count
ror64:										;
	cpi		r23, 0							;
	breq	01f								;
00:	lsr		r17								; Shift bits right
	ror		r16								;
	ror		r15								;
	ror		r14								;
	ror		r13								;
	ror		r12								;
	ror		r11								;
	ror		r10								;
	dec		r23								;
	brne	00b								; /
01:	ret										;
#endif // OPTIMIZE_SPEED

#if OPTIMIZE_SPEED
mask64:
	; r17:r10	fp
	; r23		shift count
	; r17-r11 are already set to zero
	ser		r31								;
00:	cpi		r23, 56							; Shift by 56 or more bits
	brlo	00f								;
	mov		r10, r31						;
	mov		r11, r31						;
	X_movw	r12, r10						;
	X_movw	r14, r10						;
	ser		r16								;
	subi	r23, 56							;
	breq	02f								;
01:	sec										; Left shift in a 1
	rol		r17								;
	dec		r23								;
	brne	01b								;
02:	ret										;

00:	cpi		r23, 48							; Shift by 48 or more bits
	brlo	00f								;
	mov		r10, r31						;
	mov		r11, r31						;
	X_movw	r12, r10						;
	X_movw	r14, r10						;
	subi	r23, 48							;
	breq	02f								;
01:	sec										; Left shift in a 1
	rol		r16								;
	dec		r23								;
	brne	01b								;
02:	ret										;

00:	cpi		r23, 40							; Shift by 40 or more bits
	brlo	00f								;
	mov		r10, r31						;
	mov		r11, r31						;
	X_movw	r12, r10						;
	mov		r14, r31						;
	subi	r23, 40							;
	breq	02f								;
01:	sec										; Left shift in a 1
	rol		r15								;
	dec		r23								;
	brne	01b								;
02:	ret										;

00:	cpi		r23, 32							; Shift by 32 or more bits
	brlo	00f								;
	mov		r10, r31						;
	mov		r11, r31						;
	X_movw	r12, r10						;
	subi	r23, 32							;
	breq	02f								;
01:	sec										; Left shift in a 1
	rol		r14								;
	dec		r23								;
	brne	01b								;
02:	ret										;

00:	cpi		r23, 24							; Shift by 24 or more bits
	brlo	00f								;
	mov		r10, r31						;
	mov		r11, r31						;
	mov		r12, r31						;
	subi	r23, 24							;
	breq	02f								;
01:	sec										; Left shift in a 1
	rol		r13								;
	dec		r23								;
	brne	01b								;
02:	ret										;

00:	cpi		r23, 16							; Shift by 16 or more bits
	brlo	00f								;
	mov		r10, r31						;
	mov		r11, r31						;
	subi	r23, 16							;
	breq	02f								;
01:	sec										; Left shift in a 1
	rol		r12								;
	dec		r23								;
	brne	01b								;
02:	ret										;

00:	cpi		r23, 8							; Shift by 8 or more bits
	brlo	00f								;
	mov		r10, r31						;
	subi	r23, 8							;
	breq	02f								;
01:	sec										; Left shift in a 1
	rol		r11								;
	dec		r23								;
	brne	01b								;
02:	ret										;

00:	clr		r10								; Shift by 7 or less bits
	cpi		r23, 0							;
	breq	02f								;
01:	sec										; Left shift in a 1
	rol		r10								;
	dec		r23								;
	brne	01b								;
02:	ret										;
#else // OPTIMIZE_SPEED
	; r17:r10	fp
	; r23		shift count
	; r17-r11 are already set to zero
mask64:										;
	clr		r10								;
	cpi		r23, 0							; Shift bits
	breq	02f								;
01:	sec										; Left shift in a 1
	rol		r10								;
	rol		r11								;
	rol		r12								;
	rol		r13								;
	rol		r14								;
	rol		r15								;
	rol		r16								;
	rol		r17								;
	dec		r23								;
	brne	01b								;
02:	ret										;
#endif // OPTIMIZE_SPEED

xlqtoa:
#if USE_XSPRINTF_P == 0 && USE_XSNPRINTF_P == 0
	push	r2								;
	push	r3								;
#endif
#if USE_XSNPRINTF_P == 0
	push	r4								;
	push	r5								;
#endif
	push	r6								;
	push	r7								;
	push	r8								;
	push	r9								;
	push	r10								;
	push	r11								;
	push	r12								;
	push	r13								;
	push	r14								;
	push	r15								;
	push	r16								;
	push	r17								;
	push	r28								;
	push	r29								;

#if USE_XSPRINTF_P || USE_XSNPRINTF_P
	sts		xioR3R2, r2						; Save xsprintf_P string buffer pointer
	sts		xioR3R2+1, r3					; /
#endif
#if USE_XSNPRINTF_P
	sts		xioR5R4, r4						; Save xsnprintf_P string buffer size
	sts		xioR5R4+1, r5					; /
#endif

	X_movw	r2,  r16						; r9:r2 is now the fp value (r23:r16)
	X_movw	r4,  r18						;
	mov		r19, r8							; Save precision
	X_movw	r6,  r20						;
	X_movw	r8,  r22						;
	mov		r28, r14						; Save fractional bits
	mov		r21, r12						; Save min width
	mov		r20, r10						; Save format flags

											; Convert fp to positive if negative
01:	clt										; Mark fp as positive
	sbrc	r20, xioFLAG_UNSIGNED_BIT
	rjmp	10f								; fp is unsigned
	and		r9, r9							;
	brge	10f								; fp is positive
	com		r9								; One's compliment of fp
	com		r8								;
	com		r7								;
	com		r6								;
	com		r5								;
	com		r4								;
	com		r3								;
	com		r2								; /
	adc		r2, r1							; Two's compliment of fp
	adc		r3, r1							;
	adc		r4, r1							;
	adc		r5, r1							;
	adc		r6, r1							;
	adc		r7, r1							;
	adc		r8, r1							;
	adc		r9, r1							; /
	set										; Mark fp as negative

10:	sbrc	r20, xioFLAG_NO_ROUNDING_BIT	;
	rjmp	30f								; No Rounding

											; Round decimals
	mov		r23, r19						; Get precision
	cpi		r23, 19							; Only round on precision of 18 or less
	brsh	30f
	lsl		r23								; Calculate offset into table
	lsl		r23								;
	lsl		r23								;
	ldi		ZL, lo8(rounding_const64)		;
	ldi		ZH, hi8(rounding_const64)		;
	add		ZL, r23							;
	adc		ZH, r1							; /
	X_lpm	r10, Z+							; Load rounding constant
	X_lpm	r11, Z+							;
	X_lpm	r12, Z+							;
	X_lpm	r13, Z+							;
	X_lpm	r14, Z+							;
	X_lpm	r15, Z+							;
	X_lpm	r16, Z+							;
	X_lpm	r17, Z+							; /

20:	ldi		r23, 0x40						; Max fbits is 64
	sub		r23, r28						; Bits to shift rounding constant
	rcall	ror64							;

	add		r2, r10							; Add constant to fp
	adc		r3, r11							;
	adc		r4, r12							;
	adc		r5, r13							;
	adc		r6, r14							;
	adc		r7, r15							;
	adc		r8, r16							;
	adc		r9, r17							; /

	brcc	30f								; Check for overflow
	ldi		r22, lo8(xio_ovrflw)			;
	ldi		r23, hi8(xio_ovrflw)			;
	rcall	xputs_P							;
	rjmp	90f								;

											; Push integer digits onto stack
30:	clr		r30								; Initialize count of digits on stack
	X_movw	r10, r2							;
	X_movw	r12, r4							;
	X_movw	r14, r6							;
	X_movw	r16, r8							;
	mov		r23, r28						; Get fractional bits
	rcall	ror64							;

	and		r19, r19						; Any digits after decimal?
	breq	31f								;

	push	r2								; Save fp onto stack
	push	r3								;
	push	r4								;
	push	r5								;
	push	r6								;
	push	r7								;
	push	r8								;
	push	r9								; /

	; 64x64_Top64: Multiply 64 by 64 bits. Keep top 64 bits.
	; The table below shows FP8:FP1 = TFP8:TFP1 * 0xCC...CD
	; The entries in each row show which registers that
	; r1:r0 are added to after a multiply. The first row
	; shows that after multiplying TFP1 by 0xCD that r1 is
	; added to F2 and r0 is discarded. A 'c' means that
	; the carry bit is added to the register
	;
	; Row    R9 R8 R7 R6 R5 R4 R3 R2
	;  1      0  0  0  0  0  0 R1  0	R10*CD	R1 copied to F2. All others zeroed.
	;  2                    R1 R0		R11*CD
	;  3                 R1 R0   		R12*CD
	;  4              R1 R0      		R13*CD
	;  5           R1 R0         		R14*CD
	;  6        R1 R0            		R15*CD
	;  7     R1 R0               		R16*CD
	;  8     R0                   R1	R17*CD
	;  9                  c R1 R0		R10*CC
	; 10               c R1 R0  0		R10*CC	Reuse R1:R0 F2 zeroed
	; 11            c R1 R0      		R10*CC	Reuse R1:R0
	; 12         c R1 R0         		R10*CC	Reuse R1:R0
	; 13      c R1 R0            		R10*CC	Reuse R1:R0
	; 14     R1 R0                 c	R10*CC	Reuse R1:R0
	; 15     R0                 c R1 	R10*CC	Reuse R1:R0
	; 16               c R1 R0   		R11*CC
	; 17            c R1 R0  0   		R11*CC	Reuse R1:R0 F3 zeroed
	; 18         c R1 R0         		R11*CC	Reuse R1:R0
	; 19      c R1 R0            		R11*CC	Reuse R1:R0
	; 20     R1 R0                 c	R11*CC	Reuse R1:R0
	; 21     R0                 c R1	R11*CC	Reuse R1:R0
	; 22                       R1 R0	R11*CC	Reuse R1:R0
	; 23            c R1 R0      		R12*CC
	; 24         c R1 R0  0      		R12*CC	Reuse R1:R0 F4 zeroed
	; 25      c R1 R0            		R12*CC	Reuse R1:R0
	; 26     R1 R0                 c	R12*CC	Reuse R1:R0
	; 27     R0                 c R1	R12*CC	Reuse R1:R0
	; 28                     c R1 R0	R12*CC	Reuse R1:R0
	; 29                    R1 R0		R12*CC	Reuse R1:R0
	; 30         c R1 R0         		R13*CC
	; 31      c R1 R0  0         		R13*CC	Reuse R1:R0 F5 zeroed
	; 32     R1 R0                 c	R13*CC	Reuse R1:R0
	; 33     R0                 c R1	R13*CC	Reuse R1:R0
	; 34                     c R1 R0	R13*CC	Reuse R1:R0
	; 35                  c R1 R0   	R13*CC	Reuse R1:R0
	; 36                 R1 R0   		R13*CC	Reuse R1:R0
	; 37      c R1 R0            		R14*CC
	; 38     R1 R0  0              c	R14*CC	Reuse R1:R0 F6 zeroed
	; 39     R0                 c R1 	R14*CC	Reuse R1:R0
	; 40                     c R1 R0	R14*CC	Reuse R1:R0
	; 41                  c R1 R0		R14*CC	Reuse R1:R0
	; 42               c R1 R0   		R14*CC	Reuse R1:R0
	; 43              R1 R0      		R14*CC	Reuse R1:R0
	; 44     R1 R0                 c	R15*CC
	; 45     R0  0              c R1	R15*CC	Reuse R1:R0 F7 zeroed
	; 46                     c R1 R0	R15*CC	Reuse R1:R0
	; 47                  c R1 R0		R15*CC	Reuse R1:R0
	; 48               c R1 R0   		R15*CC	Reuse R1:R0
	; 49            c R1 R0      		R15*CC	Reuse R1:R0
	; 50           R1 R0         		R15*CC	Reuse R1:R0
	; 51     R0                 c R1	R16*CC
	; 52      0              c R1 R0	R16*CC	Reuse R1:R0 F8 zeroed
	; 53                  c R1 R0		R16*CC	Reuse R1:R0
	; 54               c R1 R0   		R16*CC	Reuse R1:R0
	; 55            c R1 R0      		R16*CC	Reuse R1:R0
	; 56         c R1 R0         		R16*CC	Reuse R1:R0
	; 57        R1 R0            		R16*CC	Reuse R1:R0
	; 58                     c R1 R0	R17*CC
	; 59                  c R1 R0		R17*CC	Reuse R1:R0
	; 60               c R1 R0   		R17*CC	Reuse R1:R0
	; 61            c R1 R0      		R17*CC	Reuse R1:R0
	; 62         c R1 R0         		R17*CC	Reuse R1:R0
	; 63     c  R1 R0            		R17*CC	Reuse R1:R0
	; 64     R1 R0               		R17*CC	Reuse R1:R0

	; r9:r2 = r17:r10 / 10
	; This is done by multiplying integer with 0xCCCCCCCCCCCCCCCD
	; then shifting right 3 bits.
31:	ldi		r18, 0x0A						;
	clr		r23								; Used as zero
	ldi		r26, '0'						; Used to convert decimal to ASCII
	
33:											; Head of loop
	ldi		r31, 0xCD						;
	mul		r10, r31						; Row 1
	mov		r3, r1							;
	clr		r2								;
	clr		r4								;
	clr		r5								;
	X_movw	r6, r4							;
	X_movw	r8, r4							;

	mul		r11, r31						; Row 2
	add		r3, r0							;
	adc		r4, r1							;
;	adc		r5, r23							;

	mul		r12, r31						; Row 3
	add		r4, r0							;
	adc		r5, r1							;
;	adc		r6, r23							;

	mul		r13, r31						; Row 4
	add		r5, r0							;
	adc		r6, r1							;
;	adc		r7, r23							;

	mul		r14, r31						; Row 5
	add		r6, r0							;
	adc		r7, r1							;
;	adc		r8, r23							;

	mul		r15, r31						; Row 6
	add		r7, r0							;
	adc		r8, r1							;
;	adc		r9, r23							;

	mul		r16, r31						; Row 7
	add		r8, r0							;
	adc		r9, r1							;
;	adc		r2, r23							;

	mul		r17, r31						; Row 8
	add		r9, r0							;
	adc		r2, r1							;
;	adc		r3, r23							;

	ldi		r31, 0xCC						;

	mul		r10, r31						; Row 9
	add		r3, r0							;
	adc		r4, r1							;
	adc		r5, r23							;
	adc		r6, r23							; Extra
	adc		r7, r23							; Extra
	adc		r8, r23							; Extra

;	mul		r10, r31						; Row 10
	add		r4, r0							;
	adc		r5, r1							;
	adc		r6, r23							;

	clr		r3								;

;	mul		r10, r31						; Row 11
	add		r5, r0							;
	adc		r6, r1							;
	adc		r7, r23							;
	adc		r8, r23							; Extra

;	mul		r10, r31						; Row 12
	add		r6, r0							;
	adc		r7, r1							;
	adc		r8, r23							;

;	mul		r10, r31						; Row 13
	add		r7, r0							;
	adc		r8, r1							;
	adc		r9, r23							;

;	mul		r10, r31						; Row 14
	add		r8, r0							;
	adc		r9, r1							;
	adc		r2, r23							;

;	mul		r10, r31						; Row 15
	add		r9, r0							;
	adc		r2, r1							;
	adc		r3, r23							;

	mul		r11, r31						; Row 16
	add		r4, r0							;
	adc		r5, r1							;
	adc		r6, r23							;
	adc		r7, r23							; Extra
	adc		r8, r23							; Extra

;	mul		r11, r31						; Row 17
	add		r5, r0							;
	adc		r6, r1							;
	adc		r7, r23							;
	adc		r8, r23							; Extra

	clr		r4								;

;	mul		r11, r31						; Row 18
	add		r6, r0							;
	adc		r7, r1							;
	adc		r8, r23							;

;	mul		r11, r31						; Row 19
	add		r7, r0							;
	adc		r8, r1							;
	adc		r9, r23							;
	adc		r2, r23							; Extra

;	mul		r11, r31						; Row 20
	add		r8, r0							;
	adc		r9, r1							;
	adc		r2, r23							;
	adc		r3, r23							; Extra

;	mul		r11, r31						; Row 21
	add		r9, r0							;
	adc		r2, r1							;
	adc		r3, r23							;

;	mul		r11, r31						; Row 22
	add		r2, r0							;
	adc		r3, r1							;
;	adc		r4, r23							;

	mul		r12, r31						; Row 23
	add		r5, r0							;
	adc		r6, r1							;
	adc		r7, r23							;
	adc		r8, r23							; Extra

;	mul		r12, r31						; Row 24
	add		r6, r0							;
	adc		r7, r1							;
	adc		r8, r23							;

	clr		r5								;

;	mul		r12, r31						; Row 25
	add		r7, r0							;
	adc		r8, r1							;
	adc		r9, r23							;
	adc		r2, r23							; Extra

;	mul		r12, r31						; Row 26
	add		r8, r0							;
	adc		r9, r1							;
	adc		r2, r23							;

;	mul		r12, r31						; Row 27
	add		r9, r0							;
	adc		r2, r1							;
	adc		r3, r23							;

;	mul		r12, r31						; Row 28
	add		r2, r0							;
	adc		r3, r1							;
	adc		r4, r23							;

;	mul		r12, r31						; Row 29
	add		r3, r0							;
	adc		r4, r1							;
;	adc		r5, r23							;

	mul		r13, r31						; Row 30
	add		r6, r0							;
	adc		r7, r1							;
	adc		r8, r23							;

;	mul		r13, r31						; Row 31
	add		r7, r0							;
	adc		r8, r1							;
	adc		r9, r23							;
	adc		r2, r23							; Extra

	clr		r6								;

;	mul		r13, r31						; Row 32
	add		r8, r0							;
	adc		r9, r1							;
	adc		r2, r23							;

;	mul		r13, r31						; Row 33
	add		r9, r0							;
	adc		r2, r1							;
	adc		r3, r23							;

;	mul		r13, r31						; Row 34
	add		r2, r0							;
	adc		r3, r1							;
	adc		r4, r23							;

;	mul		r13, r31						; Row 35
	add		r3, r0							;
	adc		r4, r1							;
	adc		r5, r23							;

;	mul		r13, r31						; Row 36
	add		r4, r0							;
	adc		r5, r1							;
;	adc		r6, r23							;

	mul		r14, r31						; Row 37
	add		r7, r0							;
	adc		r8, r1							;
	adc		r9, r23							;
	adc		r2, r23							; Extra

;	mul		r14, r31						; Row 38
	add		r8, r0							;
	adc		r9, r1							;
	adc		r2, r23							;

	clr		r7								;

;	mul		r14, r31						; Row 39
	add		r9, r0							;
	adc		r2, r1							;
	adc		r3, r23							;

;	mul		r14, r31						; Row 40
	add		r2, r0							;
	adc		r3, r1							;
	adc		r4, r23							;

;	mul		r14, r31						; Row 41
	add		r3, r0							;
	adc		r4, r1							;
	adc		r5, r23							;

;	mul		r14, r31						; Row 42
	add		r4, r0							;
	adc		r5, r1							;
	adc		r6, r23							;

;	mul		r14, r31						; Row 43
	add		r5, r0							;
	adc		r6, r1							;
;	adc		r7, r23							;

	mul		r15, r31						; Row 44
	add		r8, r0							;
	adc		r9, r1							;
	adc		r2, r23							;

;	mul		r15, r31						; Row 45
	add		r9, r0							;
	adc		r2, r1							;
	adc		r3, r23							;

	clr		r8								;

;	mul		r15, r31						; Row 46
	add		r2, r0							;
	adc		r3, r1							;
	adc		r4, r23							;

;	mul		r15, r31						; Row 47
	add		r3, r0							;
	adc		r4, r1							;
	adc		r5, r23							;

;	mul		r15, r31						; Row 48
	add		r4, r0							;
	adc		r5, r1							;
	adc		r6, r23							;

;	mul		r15, r31						; Row 49
	add		r5, r0							;
	adc		r6, r1							;
	adc		r7, r23							;

;	mul		r15, r31						; Row 50
	add		r6, r0							;
	adc		r7, r1							;
;	adc		r8, r23							;

	mul		r16, r31						; Row 51
	add		r9, r0							;
	adc		r2, r1							;
	adc		r3, r23							;

;	mul		r16, r31						; Row 52
	add		r2, r0							;
	adc		r3, r1							;
	adc		r4, r23							;

	clr		r9								;

;	mul		r16, r31						; Row 53
	add		r3, r0							;
	adc		r4, r1							;
	adc		r5, r23							;

;	mul		r16, r31						; Row 54
	add		r4, r0							;
	adc		r5, r1							;
	adc		r6, r23							;

;	mul		r16, r31						; Row 55
	add		r5, r0							;
	adc		r6, r1							;
	adc		r7, r23							;

;	mul		r16, r31						; Row 56
	add		r6, r0							;
	adc		r7, r1							;
	adc		r8, r23							;

;	mul		r16, r31						; Row 57
	add		r7, r0							;
	adc		r8, r1							;
;	adc		r9, r23							;

	mul		r17, r31						; Row 58
	add		r2, r0							;
	adc		r3, r1							;
	adc		r4, r23							;

;	mul		r17, r31						; Row 59
	add		r3, r0							;
	adc		r4, r1							;
	adc		r5, r23							;

;	mul		r17, r31						; Row 60
	add		r4, r0							;
	adc		r5, r1							;
	adc		r6, r23							;

;	mul		r17, r31						; Row 61
	add		r5, r0							;
	adc		r6, r1							;
	adc		r7, r23							;

;	mul		r17, r31						; Row 62
	add		r6, r0							;
	adc		r7, r1							;
	adc		r8, r23							;

;	mul		r17, r31						; Row 63
	add		r7, r0							;
	adc		r8, r1							;
	adc		r9, r23							;

;	mul		r17, r31						; Row 64
	add		r8, r0							;
	adc		r9, r1							;
;	adc		r2, r23							;

	ldi		r23, 4							; fp >> 3
34:	lsr		r9								;
	ror		r8								;
	ror		r7								;
	ror		r6								;
	ror		r5								;
	ror		r4								;
	ror		r3								;
	ror		r2								;
	lsr		r23								;
	brne	34b								; /

											; digit = LSB(r17:r10) - LSB(LSB(r9:r2) * 0x0A)
	mul		r2, r18							; r1:r0 = r2 * 10
	mov		r1, r26							; r19 = '0'
	sub		r1, r0							;
	add		r1, r10							; Convert to ASCII
	push	r1								;
	inc		r30								;

	and		r2, r2							; More digits if r9:r2 > 0
	brne	35f								;
	and		r3, r3							;
	brne	35f								;
	and		r4, r4							;
	brne	35f								;
	and		r5, r5							;
	brne	35f								;
	and		r6, r6							;
	brne	35f								;
	and		r7, r7							;
	brne	35f								;
	and		r8, r8							;
	brne	35f								;
	and		r9, r9							;
	brne	35f								;
	rjmp	36f								; No more digits

							
35:	X_movw	r10, r2							; Get next digit
	X_movw	r12, r4							;
	X_movw	r14, r6							;
	X_movw	r16, r8							;
	rjmp	33b								;

36:	clr		r1								;

											; Determine if leading sign needed
	brtc	40f								; fp not negative
	ldi		r22, '-'						;
	rjmp	42f								;
40:	sbrs	r20, xioFLAG_LEADING_PLUS_BIT	;
	rjmp	41f								;
	ldi		r22, '+'						; Leading plus for positive fp
	rjmp	42f								;
41:	sbrs	r20, xioFLAG_LEADING_SPACE_BIT	;
	rjmp	44f								;
	ldi		r22, ' '						; Leading space for positive fp
42:	sbrs	r20, xioFLAG_ZERO_FILL_BIT		;
	rjmp	43f								;
	rcall	xputc64							;
	dec		r21								; Decrease width by 1
	rjmp	44f								;
43:	push	r22								; Save leading sign on stack
	inc		r30								;

44:	ldi		r22, ' '						; Default filler
	sbrc	r20, xioFLAG_ZERO_FILL_BIT		;
	ldi		r22, '0'						; Zero filler
	
											; Determine size of fp in ASCII
	mov		r0, r30							; Get size of integer on stack
	cpse	r19, r1							; Skip next instruction if precision is zero
	inc		r0								; Include decimal point
	add		r0, r19							; Add precision
	cp		r0, r21							; Compare to minimum width
	brsh	48f								; Exceeds or equal minimum

											; Check justification
	mov		r23, r21						; Minimum width
	sub		r23, r0							; r23 = min width - size of fp in ASCII
	sbrc	r20, xioFLAG_LEFT_JUSTIFIED_BIT	;
	rjmp	47f								;
46:	rcall	xputc64							;
	dec		r23								;
	brne	46b								;
47:	mov		r21, r23						; r21 = remaining padding needed
	rjmp	49f								;

48:	clr		r21								; r21 = no padding needed


49:	pop		r22								; Flush stacked digits
	rcall	xputc64							;
	dec		r30								;
	brne	49b								;

	and		r19, r19						; Any digits after decimal?
	brne	60f								;
	rjmp	80f								;
60:

	ldi		r22, '.'						; Output decimal point
	rcall	xputc64							;

50:
	pop		r9								; Get fp from stack
	pop		r8								;
	pop		r7								;
	pop		r6								;
	pop		r5								;
	pop		r4								;
	pop		r3								;
	pop		r2								; /

											; Calculate mask for fbits
	mov		r23, r28						; r23 = fbits
	rcall	mask64							; Mask = (1 << fbits) - 1

	push	r21								; Save remaining width

	mov		r18, r10						; Store mask in r31-r29,r27-r26,r21-r20,r18
	X_movw	r20, r11						;
	X_movw	r26, r13						;
	X_movw	r29, r15						;
	mov		r31, r17						; /

53:	and		r2, r18							; Get fractional part of fp
	and		r3, r20							;
	and		r4, r21							;
	and		r5, r26							;
	and		r6, r27							;
	and		r7, r29							;
	and		r8, r30							;
	and		r9, r31							; /

	ldi		r23, 0x0A						;
	mul		r2, r23							; Multiply fp by 10
	X_movw	r10, r0							;
	clr		r12								;
	clr		r13								;
	X_movw	r14, r12						;
	X_movw	r16, r12						;
	mul		r3, r23							;
	add		r11, r0							;
	adc		r12, r1							;
	mul		r4, r23							;
	add		r12, r0							;
	adc		r13, r1							;
	mul		r5, r23							;
	add		r13, r0							;
	adc		r14, r1							;
	mul		r6, r23							;
	add		r14, r0							;
	adc		r15, r1							;
	mul		r7, r23							;
	add		r15, r0							;
	adc		r16, r1							;
	mul		r8, r23							;
	add		r16, r0							;
	adc		r17, r1							;
	mul		r9, r23							;
	add		r17, r0							;
	clr		r0								;
	adc		r1, r0							;

	X_movw	r2, r10							;
	X_movw	r4, r12							;
	X_movw	r6, r14							;
	X_movw	r8, r16							;

											; Get integer
	mov		r23, r28						; r23 = fbits
#if OPTIMIZE_SPEED
	cpi		r23, 64							; Shift by 64 or more bits
	brlo	54f								;
	mov		r22, r1							; Get digit
	rjmp	58f								;

54: cpi		r23, 56							; Shift by 56 or more bits
	brlo	54f								;
	subi	r23, 56							;
	breq	56f								;
55:	lsr		r1								;
	ror		r17								;
	dec		r23								;
	brne	55b								;
56:	mov		r22, r17						; Get digit
	rjmp	58f								;

54: cpi		r23, 48							; Shift by 48 or more bits
	brlo	54f								;
	subi	r23, 48							;
	breq	56f								;
55:	lsr		r17								;
	ror		r16								;
	dec		r23								;
	brne	55b								;
56:	mov		r22, r16						; Get digit
	rjmp	58f								;

54: cpi		r23, 40							; Shift by 40 or more bits
	brlo	54f								;
	subi	r23, 40							;
	breq	56f								;
55:	lsr		r16								;
	ror		r15								;
	dec		r23								;
	brne	55b								;
56:	mov		r22, r15						; Get digit
	rjmp	58f								;

54: cpi		r23, 32							; Shift by 32 or more bits
	brlo	54f								;
	subi	r23, 32							;
	breq	56f								;
55:	lsr		r15								;
	ror		r14								;
	dec		r23								;
	brne	55b								;
56:	mov		r22, r14						; Get digit
	rjmp	58f								;

54: cpi		r23, 24							; Shift by 24 or more bits
	brlo	54f								;
	subi	r23, 24							;
	breq	56f								;
55:	lsr		r14								;
	ror		r13								;
	dec		r23								;
	brne	55b								;
56:	mov		r22, r13						; Get digit
	rjmp	58f								;

54: cpi		r23, 16							; Shift by 16 or more bits
	brlo	54f								;
	subi	r23, 16							;
	breq	56f								;
55:	lsr		r13								;
	ror		r12								;
	dec		r23								;
	brne	55b								;
56:	mov		r22, r12						; Get digit
	rjmp	58f								;

54: cpi		r23, 8							; Shift by 8 or more bits
	brlo	54f								;
	subi	r23, 8							;
	breq	56f								;
55:	lsr		r12								;
	ror		r11								;
	dec		r23								;
	brne	55b								;
56:	mov		r22, r11						; Get digit
	rjmp	58f								;

54:	breq	56f								; Branch if = 8
55: lsr		r11								; Shift by 7 or less
	ror		r10								;
	dec		r23								;
	brne	55b								;
56:	mov		r22, r10						; Get digit
#else // OPTIMIZE_SPEED
	cpi		r23, 0							;
	breq	56f								;
54:	lsr		r1								;
	ror		r17								;
	ror		r16								;
	ror		r15								;
	ror		r14								;
	ror		r13								;
	ror		r12								;
	ror		r11								;
	ror		r10								;
	dec		r23								;
	brne	54b								;
56:	mov		r22, r10						; Get digit
#endif // OPTIMIZE_SPEED

58:	subi	r22, -('0')						; Output digit
	rcall	xputc64							;

	dec		r19								; Check precision
	cpi		r19, 0							;
	breq	59f								;
	rjmp	53b								;

59:	clr		r1								;
	pop		r21								; Get remaining padding

80:	cpi		r21, 0							; Output any remaining padding
	breq	90f								;
	ldi		r22, ' '						;
81:	rcall	xputc64							;
	dec		r21								;
	brne	81b								;

90:
#if USE_XSPRINTF_P || USE_XSNPRINTF_P
	lds		r2, xioR3R2						; Load xsprintf_P string buffer pointer
	lds		r3, xioR3R2+1					; /
#endif
#if USE_XSNPRINTF_P
	lds		r4, xioR5R4						; Load xsnprintf_P string buffer size
	lds		r5, xioR5R4+1					; /
#endif

	pop		r29								;
	pop		r28								;
	pop		r17								;
	pop		r16								;
	pop		r15								;
	pop		r14								;
	pop		r13								;
	pop		r12								;
	pop		r11								;
	pop		r10								;
	pop		r9								;
	pop		r8								;
	pop		r7								;
	pop		r6								;
#if USE_XSNPRINTF_P == 0
	pop		r5								;
	pop		r4								;
#endif
#if USE_XSPRINTF_P ==0 && USE_XSNPRINTF_P == 0
	pop		r3								;
	pop		r2								;
#endif
	ret										;

.endfunc	// xlqtoa

#endif // USE_XLQTOA


;----------------------------------------------------------------------------
; Extended numeral string input
;
; Prototype:
; char xatoi (                  /* 1: Successful, 0: Failed */
;             const char **str, /* pointer to pointer to source string */
;             long *res         /* result */
;            );
;
; Arguments:
;	r25:r24		pointer to pointer to source string
;	r23:r22		pointer to result
;
; Local:
;		T		0: unsigned, 1: signed
;	r17:r14		temp
;	r21:r18		value
;	r23:r22		pointer to result
;		r24		radix
;		r25		current character
;	r27:r26		X pointer to string source
;	r31:r30		Z pointer to current location in string
;----------------------------------------------------------------------------
#if USE_XATOI

.section .text.lib_xio.xatoi
.func xatoi
.global xatoi
xatoi:
	push	r14								;
	push	r15								;
	push	r16								;
	push	r17								;

	X_movw	XL, r24							;
	ld		ZL, X+							;
	ld		ZH, X+							;
	clr		r18								; r21:r18 = 0;
	clr		r19								;
	X_movw	r20, r18						; /
	clt										; T = 0;

	ldi		r24, 10							; r24 = 10;
40:	ld		r25, Z+							; r25 = *Z++;
	cpi		r25, ' '						; if(r25 == ' ') continue
	breq	40b								; /
	brcs	70f								; if(r25 < ' ') error;
	cpi		r25, '-'						; if(r25 == '-') {
	brne	42f								;  T = 1;
	set										;  continue;
	rjmp	40b								; }
42:	cpi		r25, '9'+1						; if(r25 > '9') error;
	brcc	70f								; /
	cpi		r25, '0'						; if(r25 < '0') error;
	brcs	70f								; /
	brne	51f								; if(r25 > '0') cv_start;
	ldi		r24, 8							; r24 = 8;
	ld		r25, Z+							; r25 = *Z++;
	cpi		r25, ' '+1						; if(r25 <= ' ') exit;
	brcs	80f								; /
	cpi		r25, 'b'						; if(r25 == 'b') {
	brne	43f								;  r24 = 2;
	ldi		r24, 2							;  cv_start;
	rjmp	50f								; }
43:	cpi		r25, 'x'						; if(r25 != 'x') error;
	brne	51f								; /
	ldi		r24, 16							; r24 = 16;

50:	ld		r25, Z+							; r25 = *Z++;
51:	cpi		r25, ' '+1						; if(r25 <= ' ') break;
	brcs	80f								; /
	cpi		r25, 'a'						; if(r25 >= 'a') r25 =- 0x20;
	brcs	52f								;
	subi	r25, 0x20						; /
52:	subi	r25, '0'						; if((r25 - '0') < 0) error;
	brcs	70f								; /
	cpi		r25, 10							; if(r25 >= 10) {
	brcs	53f								;  r25 -= 7;
	subi	r25, 7							;  if(r25 < 10) 
	cpi		r25, 10							;
	brcs	70f								; }
53:	cp		r25, r24						; if(r25 >= r24) error;
	brcc	70f								; /

60:	mul		r18, r24						; r17:r14 = r21:r18 * r24
	X_movw	r14, r0							;
	clr		r16								;
	mul		r19, r24						;
	add		r15, r0							;
	adc		r16, r1							;
	clr		r17								;
	mul		r20, r24						;
	add		r16, r0							;
	adc		r17, r1							;
	clr		r18								;
	mul		r21, r24						;
	add		r17, r0							;
	add		r14, r25						; r17:r14 += r25
	adc		r15, r18						;
	adc		r16, r18						;
	adc		r17, r18						; /
	X_movw	r18, r14						; r21:r18 = r17:r14
	X_movw	r20, r16						; /
	rjmp	50b								; repeat

70:	ldi		r24, 0							;
	rjmp	81f								;
80:	ldi		r24, 1							;
81:	clr		r25								;
	brtc	82f								;
	com		r18								;
	com		r19								;
	com		r20								;
	com		r21								;
	adc		r18, r25						;
	adc		r19, r25						;
	adc		r20, r25						;
	adc		r21, r25						;
82:	subi	ZL, 1							; Dec Z to point to last char read
	sbci	ZH, 0							; /
	st		-X, ZH							;
	st		-X, ZL							;
	X_movw	XL, r22							;
	st		X+, r18							;
	st		X+, r19							;
	st		X+, r20							;
	st		X+, r21							;
	clr		r1								;

	pop		r17								;
	pop		r16								;
	pop		r15								;
	pop		r14								;
	ret										;

.endfunc	// xatoi

#endif	// USE_XATOI


;----------------------------------------------------------------------------
; Extended 32 bit fixed point string input
;
; This routine is based on strtod from the avr-libc-1.8.1 library.
; Copyright (c) 2002-2005  Michael Stumpf  <mistumpf@de.pepperl-fuchs.com>
; Copyright (c) 2006,2008  Dmitry Xmelkov
;
;Prototype:
; char xatoq (           // 1: Successful, 0: Failed
;      const char **str, // pointer to pointer to source string
;      long *fp,         // pointer to fixed point result
;	   char fbits,		 // number of bits in fraction
;	   char unsign		 // Non zero if fp is unsinged
; );
;
; Arguments:
;	r25:r24		pointer to pointer to source string
;	r23:r22		pointer to fixed point result
;		r20		number of bits in fraction
;		r18		unsigned fp
;
; Local:
;		T		0: unsigned, 1: signed
;	r11:r4		fp
;	r15:r12		temp fp
;	r19:r16		multiplier
;		r17		flags
;		r18		holds 0x0A
;		r20		current character from string
;		r21		count of digits after '.'
;	r23:r22		pointer to fix point result
;		r24		zero
;		r25		number of bits in fraction
;	r25:r24		pointer to pointer to source string
;	r27:r26		X pointer to string source
;	r29:r28		Y temp
;	r31:r30		Z pointer to current location in string
;----------------------------------------------------------------------------
#if USE_XATOQ

#define FP1			r4
#define FP2			r5
#define FP3			r6
#define FP4			r7
#define	FP5			r8
#define	FP6			r9
#define	FP7			r10
#define	FP8			r11
#define TFP1		r12
#define TFP2		r13
#define TFP3		r14
#define TFP4		r15
#define M1			r16
#define M2			r17
#define M3			r18
#define M4			r19
#define DECIMAL		r18
#define TEN			r19
#define CHAR		r20
#define FLAGS		r21
#define FPPTRLO		r22
#define FPPTRHI		r23
#define ZERO		r24
#define	FRACBITS	r25
#define STRPTRLO	r26
#define STRPTRHI	r27
#define	TEMP		r28
#define TEMPHI		r29

#define	FL_ANY		1						; A valid digit was read
#define	FL_DOT		2						; A valid decimal point was read
#define FL_SKP		4						; Skip remaining digits/decimal
#define FL_NEG		8						; Value is negative
#define	FL_ANY_BIT	0						; Bit position of FL_ANY
#define	FL_DOT_BIT	1						; Bit position of FL_DOT
#define FL_SKP_BIT	2						; Bit position of FL_SKP
#define	FL_NEG_BIT	3						; Bit position of FL_NEG

.section .text.lib_xio.xatoq
.func xatoq
.global xatoq
.global xatok
.global xatouk
.global xator

;			LSB				  MSB	RSHIFT
powm10:
	.byte	0xCD, 0xCC, 0xCC, 0xCC, 3		; 0.1
	.byte	0x3D, 0x0A, 0xD7, 0xA3, 6		; 0.01
	.byte	0x98, 0x6E, 0x12, 0x83, 9		; 0.001
	.byte	0x59, 0x17, 0xB7, 0xD1, 13		; 0.0001
	.byte	0x47, 0xAC, 0xC5, 0xA7, 16		; 0.00001
	.byte	0x06, 0xBD, 0x37, 0x86, 19		; 0.000001
	.byte	0xD6, 0x94, 0xBF, 0xD6, 23		; 0.0000001
	.byte	0x12, 0x77, 0xCC, 0xAB, 26		; 0.00000001
	.byte	0x41, 0x5F, 0x70, 0x89, 29		; 0.000000001
	.byte	0x00							; To keep the number of bytes even

check_int:									; Verify that integer part did not overflow
	clr		TFP1							; Returns:
	clr		TFP2							;   Carry Set: Good
	X_movw	TFP3, TFP1						;   Carry Clear: Overflow
	ldi		TEMP, 32						;
	brts	00f								;
	dec		TEMP							; Signed, so max 31 bits
00:	mov		TEMPHI, TEMP					;
	sub		TEMP, FRACBITS					;
	cp		TEMP, TEMPHI					;
	brlo	01f								;
	sec										; No integer if fracbits = max bits
	ret										;
01:	cpi		TEMP, 24						;
	brlo	02f								;
	inc		TFP4							; Set LSB of TFP4
	subi	TEMP, 24						;
	rjmp	06f								;
02:	cpi		TEMP, 16						;
	brlo	03f								;
	inc		TFP3							; Set LSB of TFP3
	subi	TEMP, 16						;
	rjmp	06f								;
03:	cpi		TEMP, 8							;
	brlo	04f								;
	inc		TFP2							; Set LSB of TFP2
	subi	TEMP, 8							;
	rjmp	06f								;
04:	inc		TFP1							; Set LSB of TFP1
	rjmp	06f								;
05:	lsl		TFP1							; Shift bit to fracbits position
	rol		TFP2							;
	rol		TFP3							;
	rol		TFP4							;
	dec		TEMP							;
06:	cp		TEMP, ZERO						;
	brne	05b								; /
	cp		FP1, TFP1						; Verify fp < tfp
	cpc		FP2, TFP2						;
	cpc		FP3, TFP3						;
	cpc		FP4, TFP4						;
	brcs	07f								; Integer is good
	clc										; Integer overflowed
07:	ret										;

divtfp:
	; 32x32_Prod64: Multiply 32 by 32 bits. 64 bit product.
	; The table below shows FP8:FP1 = TFP4:TFP1 * M4:M1
	; The entries in each row show which registers that
	; r1:r0 are added to after a multiply. The first row
	; shows that after multiplying TFP1 by M1 that r1:r0
	; is moved to FP2:FP1. A 'c' means that the carry bit
	; is added to the register.
	;
	; Row F8 F7 F6 F5 F4 F3 F2 F1
	;  1                    R1:R0	TF1*M1 R1:R0 moved to F2:F1
	;  2                 R1:R0      TF2*M1
	;  3               c R1:R0      TF1*M2
	;  4              R1:R0         TF3*M1
	;  5            c R1:R0         TF2*M2 
	;  6            c R1:R0         TF1*M3 
	;  7           R1:R0            TF4*M1
	;  8         c R1:R0            TF3*M2 
	;  9         c R1:R0            TF2*M3 
	; 10         c R1:R0            TF1*M4 
	; 11        R1:R0               TF4*M2
	; 12      c R1:R0               TF3*M3
	; 13      c R1:R0               TF2*M4
	; 14     R1:R0                  TF4*M3 
	; 15   c R1:R0                  TF3*M4 
	; 16  R1:R0                     TF4*M4 

	mov		YL, DECIMAL						; offset = DECIMAL * 5
	ldi		YH, 5							;
	mul		YL, YH							; /
	X_movw	YL, ZL							; Z = powm10 + offset
	ldi		ZL, lo8(powm10)					;
	ldi		ZH, hi8(powm10)					;
	add		ZL, r0							;
	adc		ZH, r1							; /
	X_lpm	M1, Z+							; Load M with multiplier
	X_lpm	M2, Z+							;
	X_lpm	M3, Z+							;
	X_lpm	M4, Z+							; /

	clr		FP3								;
	clr		FP4								;
	X_movw	FP5, FP3						;
	X_movw	FP7, FP3						;

	mul		TFP1, M1						; Row 1
	X_movw	FP1, r0							;

	mul		TFP2, M1						; Row 2
	add		FP2, r0							;
	adc		FP3, r1							;
;	adc		FP4, ZERO						;

	mul		TFP1, M2						; Row 3
	add		FP2, r0							;
	adc		FP3, r1							;
	adc		FP4, ZERO						;

	mul		TFP3, M1						; Row 4
	add		FP3, r0							;
	adc		FP4, r1							;
;	adc		FP5, ZERO						;

	mul		TFP2, M2						; Row 5
	add		FP3, r0							;
	adc		FP4, r1							;
	adc		FP5, ZERO						;

	mul		TFP1, M3						; Row 6
	add		FP3, r0							;
	adc		FP4, r1							;
	adc		FP5, ZERO						;

	mul		TFP4, M1						; Row 7
	add		FP4, r0							;
	adc		FP5, r1							;
;	adc		FP6, ZERO						;

	mul		TFP3, M2						; Row 8
	add		FP4, r0							;
	adc		FP5, r1							;
	adc		FP6, ZERO						;

	mul		TFP2, M3						; Row 9
	add		FP4, r0							;
	adc		FP5, r1							;
	adc		FP6, ZERO						;

	mul		TFP1, M4						; Row 10
	add		FP4, r0							;
	adc		FP5, r1							;
	adc		FP6, ZERO						;

	mul		TFP4, M2						; Row 11
	add		FP5, r0							;
	adc		FP6, r1							;
;	adc		FP7, ZERO						;

	mul		TFP3, M3						; Row 12
	add		FP5, r0							;
	adc		FP6, r1							;
	adc		FP7, ZERO						;

	mul		TFP2, M4						; Row 13
	add		FP5, r0							;
	adc		FP6, r1							;
	adc		FP7, ZERO						;

	mul		TFP4, M3						; Row 14
	add		FP6, r0							;
	adc		FP7, r1							;
;	adc		FP8, ZERO						;

	mul		TFP3, M4						; Row 15
	add		FP6, r0							;
	adc		FP7, r1							;
	adc		FP8, ZERO						;

	mul		TFP4, M4						; Row 16
	add		FP7, r0							;
	adc		FP8, r1							;

	X_lpm	M1, Z+							; M1 = shift right count
	X_movw	ZL, YL							;

shifttfp:
	ldi		M2, 32							;
	sub		M2, FRACBITS					;
	add		M1, M2							; /
#if OPTIMIZE_SPEED
	cpi		M1, 32							; Shift by 32 or more bits
	brlo	00f								;
	rol		FP4								; Save high bit in case needed for rounding
	ror		M3								; /
	X_movw	FP1, FP5						;
	X_movw	FP3, FP7						;
	subi	M1, 32							;
	mov		FP8, M3							; Save bit for rounding in case even 32 bits
	clr		FP5								;
	rjmp	03f								;
00:	cpi		M1, 24							; Shift by 24 or more bits
	brlo	01f								;
	rol		FP3								; Save high bit in case needed for rounding
	ror		M3								; /
	X_movw	FP1, FP4						;
	X_movw	FP3, FP6						;
	mov		FP5, FP8						;
	subi	M1, 24							;
	mov		FP8, M3							; Save bit for rounding in case even 24 bits
	rjmp	03f								;
01:	cpi		M1, 16							; Shift by 16 or more bits
	brlo	02f								;
	rol		FP2								; Save high bit in case needed for rounding
	ror		M3								; /
	X_movw	FP1, FP3						;
	X_movw	FP3, FP5						;
	mov		FP5, FP7						;
	subi	M1, 16							;
	mov		FP8, M3							; Save bit for rounding in case even 16 bits
	rjmp	03f								;
02:	cpi		M1, 8							; Shift by 8 or more bits
	brlo	03f								;
	rol		FP1								; Save high bit in case needed for rounding
	ror		M3								; /
	X_movw	FP1, FP2						;
	X_movw	FP3, FP4						;
	X_movw	FP5, FP6						;
	subi	M1, 8							;
	mov		FP8, M3							; Save bit for rounding in case even 8 bits
03:	cp		M1, ZERO						;
	breq	05f								;
04:	lsr		FP5								; Shift remaining bits
	ror		FP4								;
	ror		FP3								;
	ror		FP2								;
	ror		FP1								;
	ror		FP8								; Save bit for rounding
	dec		M1								;
	brne	04b								;
05:	sbrs	FP8, 7							;
	rjmp	05f								;
#else // OPTIMIZE_SPEED
00:	lsr		FP8								; Shift bits right
	ror		FP7								;
	ror		FP6								;
	ror		FP5								;
	ror		FP4								;
	ror		FP3								;
	ror		FP2								;
	ror		FP1								; /
	dec		M1								;
	brne	00b								;
	brcc	05f								;
#endif // OPTIMIZE_SPEED
	inc		M1								; Round up
	add		FP1, M1							;
	adc		FP2, ZERO						;
	adc		FP3, ZERO						;
	adc		FP4, ZERO						; Need to check for overflow?

05:	ret										;

xatoq:
xatok:
xatouk:
xator:
	tst		r25								; Check (r25:r24) *str is not zero
	brne	00f								;
	tst		r24								;
	brne	00f								;
	rjmp	01f								; /
00:	tst		FPPTRHI							; Check (FPPTRHI:FPPTRLO) *fp is not zero
	brne	02f								;
	tst		FPPTRLO							;
	brne	02f								;
01:	clr		24								; Error
	clr		25								;
	ret										;
02:	ldi		r19, 31							; Max signed bits: 31
	tst		r18								;
	breq	03f								;
	ldi		r19, 32							; Max unsinged bits: 32
03:	cp		r19, r20						; Verify fracbits <= max
	brcc	nocheck_xatoq					;
	mov		r20, r19						; /

nocheck_xatoq:
	push	r4								;
	push	r5								;
	push	r6								;
	push	r7								;
	push	r8								;
	push	r9								;
	push	r10								;
	push	r11								;
	push	r12								;
	push	r13								;
	push	r14								;
	push	r15								;
	push	r16								;
	push	r17								;
	push	r28								;
	push	r29								;

	X_movw	XL, r24							; X = *str
	clr		ZERO							;
	ld		ZL, X+							; Z = **str: pointer to source string
	ld		ZH, X+							;
	mov		FRACBITS, r20					;
	clr		FLAGS							;
	clr		DECIMAL							;
	set										; Signed fp
	tst		r18								;
	breq	00f								;
	clt										; fp is unsigned
00:	ldi		TEN, 10							;

											; Ignore leading white space: ' '
10:	ld		CHAR, Z+						; c = *Z++;
	cpi		CHAR, ' '						; if(c == ' ')
	breq	10b								;   continue
	brcc	20f								; if(c < ' ')
	rjmp	70f								;   error

											; Check for optional leading '-' or '+'
20:	cpi		CHAR, '-'						; if(c == '-')
	brne	21f								; {
	ori		FLAGS, FL_NEG					;   Value is negative
	set										;
	rjmp	25f								; }
21:	cpi		CHAR, '+'						; if(c == '+')
	brne	26f								;   get next char

											; Ignore leading zeros
25:	ld		CHAR, Z+						; c = *Z++;
26:	cpi		CHAR, '0'						;
	brne	27f								;
	ori		FLAGS, FL_ANY					;
	rjmp	25b								;

											; Parse digits
27:	clr		FP1								; FP4:FP1 = 0
	clr		FP2								;
	X_movw	FP3, FP1						; /
	X_movw	TFP1, FP1						; TFP4:TFP1 = 0
	X_movw	TFP3, FP1						; /
	rjmp	31f								;
30:	ld		CHAR, Z+						; c = *Z++;
31:	subi	CHAR, '0'						; c -= '0';
	cpi		CHAR, 10						; if (c <= 9)
	brsh	34f								; {
	ori		FLAGS, FL_ANY					;   A digit was read
	ldi		TEMP, 0x98						;
	cp		TFP1, TEMP						;
	ldi		TEMP, 0x99						;
	cpc		TFP2, TEMP						;
	cpc		TFP3, TEMP						;
	ldi		TEMP, 0x19						;
	cpc		TFP4, TEMP						;   if (tfp >= (0xFFFFFFFF - 9) / 10)
	brcs	33f								;	{
	cpi		CHAR, 6							;      if (c <= 5)
	brsh	32f								;	   {
	ldi		TEMP, 0x99						;
	cp		TFP1, TEMP						;
	cpc		TFP2, TEMP						;	      if (tfp == 0x19999999)
	cpc		TFP3, TEMP						;         {
	ldi		TEMP, 0x19						;            goto tfp = fp * 10 + c
	cpc		TFP4, TEMP						;		  }
	breq	33f								;	   }
32:	ori		FLAGS, FL_SKP					;      flags |= FL_SKP, skip digits, break
	rjmp	40f								;   }
33:	mul		FP1, TEN						;   tfp = fp * 10 + c
	X_movw	TFP1, r0						;
	clr		TFP3							;
	mul		FP2, TEN						;
	add		TFP2, r0						;
	adc		TFP3, r1						;
	clr		TFP4							;
	mul		FP3, TEN						;
	add		TFP3, r0						;
	adc		TFP4, r1						;
	mul		FP4, TEN						;
	add		TFP4, r0						;
	add		TFP1, CHAR						;
	adc		TFP2, ZERO						;
	adc		TFP3, ZERO						;
	adc		TFP4, ZERO						;   /
	X_movw	FP1, TFP1						;   FP4:FP1 = TFP4:TFP1
	X_movw	FP3, TFP3						;   /
	sbrc	FLAGS,FL_DOT_BIT				;
	inc		DECIMAL							;   Count digits after decimal point
	rjmp	30b								;   continue
34:	cpi		CHAR, '.' - '0'					; } else if (c == '.' - '0') && !(flags & FL_DOT)
	brne	35f								; {
	sbrc	FLAGS,FL_DOT_BIT				;
	rjmp	35f								;
	ori		FLAGS, FL_DOT					;    flags |= FL_DOT
	rcall	check_int						;    if integer value did not overflow
	brcs	30b								;      continue
	ori		FLAGS, FL_SKP					;    Overflow, skip digits
	rjmp	40f								;    break
35:	sbrc	FLAGS,FL_DOT_BIT				; } else if !(flags & FL_DOT)
	rjmp	50f								; {
	rcall	check_int						;    if integer value did not overflow
	brcs	50f								;	   convert to fixed point
	rjmp	70f								;	 else error
											; }

											; Skip remaining digits due to over/underflow
40:	ld		CHAR, Z+						; c = *Z++;
	subi	CHAR, '0'						; c -= '0';
	cpi		CHAR, 10						; if (c <= 9)
	brsh	41f								; { continue
	rjmp	40b								; }
41:	cpi		CHAR, '.' - '0'					; if (c == '.' - '0')
	brne	42f								; {
	sbrc	FLAGS,FL_DOT_BIT				;   if (flags & FL_DOT)
	rjmp	50f								;     Underflow, break
	sbrc	FLAGS,FL_SKP_BIT				;   if (flags & FL_SKP)
	rjmp	70f								;     Overflow, error
	ori		FLAGS, FL_SKP					;   flags |= FL_SKP
	rjmp	40b								; } continue
42:	sbrs	FLAGS,FL_DOT_BIT				; if (flag & FL_DOT)
	rjmp	70f								;   Error: The integer part overflowed

50:	sbrs	FLAGS,FL_ANY_BIT				; if nothing was parsed
	rjmp	70f								;   Error

											; Divide tfp to get fraction
	cp		DECIMAL, ZERO					;
	brne	60f								;
	X_movw	FP5, FP1						; Value is integer part only
	X_movw	FP7, FP3						;
	clr		FP1								;
	clr		FP2								;
	X_movw	FP3, FP1						;
	clr		M1								;
	rcall	shifttfp						;
	rjmp	80f								; /
60:	dec		DECIMAL							;
	rcall	divtfp							;
	rjmp	80f								;

70:	ldi		r24, 0							; Return 0: Error
	rjmp	82f								;
80:	ldi		r24, 1							; Return 1: Success
	sbrs	FLAGS,FL_NEG_BIT				;
	rjmp	81f								;
	clr		r25								; fp is negative
	com		FP1								;
	com		FP2								;
	com		FP3								;
	com		FP4								;
	adc		FP1, r25						;
	adc		FP2, r25						;
	adc		FP3, r25						;
	adc		FP4, r25						; /
81:	X_movw	YL, FPPTRLO						; *fp = result
	st		Y+, FP1							;
	st		Y+, FP2							;
	st		Y+, FP3							;
	st		Y+, FP4							; /
82:	subi	ZL, 1							; Dec Z to point to last char read
	sbci	ZH, 0							; /
	st		-X, ZH							; Update **str to point to last char
	st		-X, ZL							; /
	clr		r25								;
	clr		r1								;

	pop		r29								;
	pop		r28								;
	pop		r17								;
	pop		r16								;
	pop		r15								;
	pop		r14								;
	pop		r13								;
	pop		r12								;
	pop		r11								;
	pop		r10								;
	pop		r9								;
	pop		r8								;
	pop		r7								;
	pop		r6								;
	pop		r5								;
	pop		r4								;
	ret										;

.endfunc	// xatoq

#undef FP1
#undef FP2
#undef FP3
#undef FP4
#undef FP5
#undef FP6
#undef FP7
#undef FP8
#undef TFP1
#undef TFP2
#undef TFP3
#undef TFP4
#undef M1
#undef M2
#undef M3
#undef M4
#undef DECIMAL
#undef TEN
#undef CHAR
#undef FLAGS
#undef FPPTRLO
#undef FPPTRHI
#undef ZERO
#undef FRACBITS
#undef STRPTRLO
#undef STRPTRHI
#undef TEMP
#undef TEMPHI

#undef FL_ANY
#undef FL_DOT
#undef FL_SKP
#undef FL_NEG
#undef FL_ANY_BIT
#undef FL_DOT_BIT
#undef FL_SKP_BIT
#undef FL_NEG_BIT

#endif	// USE_XATOQ

;----------------------------------------------------------------------------
; Extended 64 bit fixed point string input
;
; This routine is based on strtod from the avr-libc-1.8.1 library.
; Copyright (c) 2002-2005  Michael Stumpf  <mistumpf@de.pepperl-fuchs.com>
; Copyright (c) 2006,2008  Dmitry Xmelkov
;
;Prototype:
; char xatolq (          // 1: Successful, 0: Failed
;      const char **str, // pointer to pointer to source string
;      long long  *fp,   // pointer to fixed point result
;	   char fbits,		 // number of bits in fraction
;	   char unsign		 // Non zero if fp is unsinged
; );
;
; Arguments:
;	r25:r24		pointer to pointer to source string
;	r23:r22		pointer to fixed point result
;		r20		number of bits in fraction
;		r18		unsigned fp
;
; Local:
;		T		0: unsigned, 1: signed
;	r11:r4		fp
;	r15:r12		temp fp
;	r19:r16		multiplier
;		r17		flags
;		r18		holds 0x0A
;		r20		current character from string
;		r21		count of digits after '.'
;	r23:r22		pointer to fix point result
;		r24		zero
;		r25		number of bits in fraction
;	r25:r24		pointer to pointer to source string
;	r27:r26		X pointer to string source
;	r29:r28		Y temp
;	r31:r30		Z pointer to current location in string
;----------------------------------------------------------------------------
#if USE_XATOLQ

;			LSB										  MSB	RSHIFT
powm10_64:
	.byte	0xCD, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 3	; 1E-01
	.byte	0x0A, 0xD7, 0xA3, 0x07, 0x3D, 0x0A, 0xD7, 0xA3, 6	; 1E-02
	.byte	0x3B, 0xDF, 0x4F, 0x8D, 0x97, 0x6E, 0x12, 0x83, 9	; 1E-03
	.byte	0x2C, 0x65, 0x19, 0xE2, 0x58, 0x17, 0xB7, 0xD1, 13	; 1E-04
	.byte	0x23, 0x84, 0x47, 0x1B, 0x47, 0xAC, 0xC5, 0xA7, 16	; 1E-05
	.byte	0xB6, 0x69, 0x6C, 0xAF, 0x05, 0xBD, 0x37, 0x86, 19	; 1E-06
	.byte	0xBC, 0x42, 0x7A, 0xE5, 0xD5, 0x94, 0xBF, 0xD6, 23	; 1E-07
	.byte	0xFD, 0xCE, 0x61, 0x84, 0x11, 0x77, 0xCC, 0xAB, 26	; 1E-08
	.byte	0x97, 0xA5, 0xB4, 0x36, 0x41, 0x5F, 0x70, 0x89, 29	; 1E-09
	.byte	0xBF, 0xD5, 0xED, 0xBD, 0xCE, 0xFE, 0xE6, 0xDB, 33	; 1E-10
	.byte	0xFF, 0xAA, 0x24, 0xCB, 0x0B, 0xFF, 0xEB, 0xAF, 36	; 1E-11
	.byte	0xCC, 0x88, 0x50, 0x6F, 0x09, 0xCC, 0xBC, 0x8C, 39	; 1E-12
	.byte	0x13, 0x0E, 0xB4, 0x4B, 0x42, 0x13, 0x2E, 0xE1, 43	; 1E-13
	.byte	0x0F, 0xD8, 0x5C, 0x09, 0x35, 0xDC, 0x24, 0xB4, 46	; 1E-14
	.byte	0xD9, 0xAC, 0xB0, 0x3A, 0xF7, 0x7C, 0x1D, 0x90, 49	; 1E-15
	.byte	0x5B, 0xE1, 0x4D, 0xC4, 0xBE, 0x94, 0x95, 0xE6, 53	; 1E-16
	.byte	0x49, 0xB4, 0xA4, 0x36, 0x32, 0xAA, 0x77, 0xB8, 56	; 1E-17
	.byte	0x07, 0x5D, 0x1D, 0x92, 0x8E, 0xEE, 0x92, 0x93, 59	; 1E-18
	.byte	0xA5, 0x61, 0x95, 0xB6, 0x7D, 0x4A, 0x1E, 0xEC, 63	; 1E-19
;	.byte	0xEB, 0x1A, 0x11, 0x92, 0x64, 0x08, 0xE5, 0xBC, 66	; 1E-20
	.byte	0x00												; To keep the number of bytes even

#endif	// USE_XATOLQ
