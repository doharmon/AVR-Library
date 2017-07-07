;----------------------------------------------------------------------------
; wprintw.s - mcurses lib
;
; Copyright (c) 2016 David Harmon
;
; This program is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation; either version 2 of the License, or
; (at your option) any later version.
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

#if FLASHEND > 0x1FFFF
#error wprintw module does not support 256K devices
#endif

.nolist
#include <avr/io.h>							// Device specific definitions.
#include "../common/macros.inc"				// Assembler macros
.list

;----------------------------------------------------------------------------
; wprintw_P
;
; Local Variables
;	 r3:r2		win. Must not be modified by subroutines
;	r25:r24		Pointer to putByte routine
;
; Top of stack ->	...
;					PSTR high
;					PSTR low		Y+7
;					win  high		Y+6
;					win  low		Y+5
;					ret  high		Y+4
;					ret  low		Y+3
;					r29  YH			Y+2
;					r28  YL			Y+1
; Stack frame ->					Y
;----------------------------------------------------------------------------
.section .text.lib_mcurses.wprintw_P
.func wprintw_P
.global wprintw_P

putByte:
	mov		r22, r24						; r22 = ch
	X_movw	r24, r2							; r25:r24 = win
	ldi		r20, 0							; r20 = addch
	jmp		mcurses_addch_or_insch			; mcurses_addch_or_insch will return to calling code.

wprintw_P:
	clt										; Arguments are after format string
1:	push	YH								; Set up stack frame
	push	YL								;
	in		YL, _SFR_IO_ADDR(SPL)			;
#ifdef SPH
	in		YH, _SFR_IO_ADDR(SPH)			;
#else
	clr		YH								; /
#endif
	adiw	YL,  5							; Y = pointer to arguments
	ldi		r24, lo8(pm(putByte))			; Set local output function
	ldi		r25, hi8(pm(putByte))			;
	push	r3								;
	push	r2								;
	ld		r2,  Y+							; r3:r2 = win
	ld		r3,  Y+							; /
	rcall	xioprintf_P						;
	pop		r2								;
	pop		r3								;
	pop		YL								;
	pop		YH								;
	ret										;

.endfunc	// wprintw_P

;----------------------------------------------------------------------------
; wprintw_P_raw
;
; Local Variables
;	r25:r24		Pointer to putByte_raw routine
;
; Top of stack ->	...
;					PSTR high
;					PSTR low		Y+7
;					win  high		Y+6
;					win  low		Y+5
;					ret  high		Y+4
;					ret  low		Y+3
;					r29  YH			Y+2
;					r28  YL			Y+1
; Stack frame ->					Y
;----------------------------------------------------------------------------
.section .text.lib_mcurses.wprintw_P_raw
.func wprintw_P_raw
.global wprintw_P_raw

wprintw_P_raw:
	clt										; Arguments are after format string
1:	push	YH								; Set up stack frame
	push	YL								;
	in		YL, _SFR_IO_ADDR(SPL)			;
#ifdef SPH
	in		YH, _SFR_IO_ADDR(SPH)			;
#else
	clr		YH								; /
#endif
	adiw	YL,  5							; Y = pointer to arguments
	ld		r30, Y+							; Z = win
	ld		r31, Y							; /
	ld		r0,  Z+							; Z = win->scr
	ld		r31, Z							;
	mov		r30, r0							; /
	ld		r0,  Z+							; Z = win->scr->term
	ld		r31, Z							;
	mov		r30, r0							; /
	lpm		r24, Z+							; r25:r24 = win->scr->term->putByte
	lpm		r25, Z+							; /
	rcall	xioprintf_P						;
	pop		YL								;
	pop		YH								;
	ret										;

.endfunc	// wprintw_P_raw

;----------------------------------------------------------------------------
; mvwprintw_P
;
; Local Variables
;	 r3:r2		win. Must not be modified by subroutines
;	r25:r24		Pointer to putByte routine
;
; Top of stack ->	...
;					PSTR high
;					PSTR low		Y+11
;					x filler		Y+10
;					x				Y+9
;					y filler		Y+8
;					y				Y+7
;					win  high		Y+6
;					win  low		Y+5
;					ret  high		Y+4
;					ret  low		Y+3
;					r29  YH			Y+2
;					r28  YL			Y+1
; Stack frame ->					Y
;----------------------------------------------------------------------------
.section .text.lib_mcurses.mvwprintw_P
.func mvwprintw_P
.global mvwprintw_P

mvputByte:
	mov		r22, r24						; r22 = ch
	X_movw	r24, r2							; r25:r24 = win
	ldi		r20, 0							; r20 = addch
	jmp		mcurses_addch_or_insch			; mcurses_addch_or_insch will return to calling code.

mvwprintw_P:
	clt										; Arguments are after format string
1:	push	YH								; Set up stack frame
	push	YL								;
	in		YL, _SFR_IO_ADDR(SPL)			;
#ifdef SPH
	in		YH, _SFR_IO_ADDR(SPH)			;
#else
	clr		YH								; /
#endif
	push	r3								;
	push	r2								;
	ldd		r2,  Y+5						; r3:r2 = win
	ldd		r3,  Y+6						; /
	X_movw	r24, r2							; r25:r24 = win
	ldd		r22, Y+7						; y
	ldd		r20, Y+9						; x
	rcall	wmove							;
	ldi		r24, lo8(pm(putByte))			; Set local output function
	ldi		r25, hi8(pm(putByte))			;
	adiw	YL,  11							; Y = pointer to arguments
	rcall	xioprintf_P						;
	pop		r2								;
	pop		r3								;
	pop		YL								;
	pop		YH								;
	ret										;

.endfunc	// mvwprintw_P

;----------------------------------------------------------------------------
; mvwprintw_P_raw
;
; Local Variables
;	r25:r24		Pointer to putByte_raw routine
;
; Top of stack ->	...
;					PSTR high
;					PSTR low		Y+11
;					x filler		Y+10
;					x				Y+9
;					y filler		Y+8
;					y				Y+7
;					win  high		Y+6
;					win  low		Y+5
;					ret  high		Y+4
;					ret  low		Y+3
;					r29  YH			Y+2
;					r28  YL			Y+1
; Stack frame ->					Y
;----------------------------------------------------------------------------
.section .text.lib_mcurses.mvwprintw_P_raw
.func mvwprintw_P_raw
.global mvwprintw_P_raw

mvwprintw_P_raw:
	clt										; Arguments are after format string
1:	push	YH								; Set up stack frame
	push	YL								;
	in		YL, _SFR_IO_ADDR(SPL)			;
#ifdef SPH
	in		YH, _SFR_IO_ADDR(SPH)			;
#else
	clr		YH								; /
#endif
	ldd		r24, Y+5						; r25:r24 = win
	ldd		r25, Y+6						; /
	ldd		r22, Y+7						; y
	ldd		r20, Y+9						; x
	rcall	wmove_raw						;
	ldd		r30, Y+5						; Z = win
	ldd		r31, Y+6						; /
	ld		r0,  Z+							; Z = win->scr
	ld		r31, Z							;
	mov		r30, r0							; /
	ld		r0,  Z+							; Z = win->scr->term
	ld		r31, Z							;
	mov		r30, r0							; /
	lpm		r24, Z+							; r25:r24 = win->scr->term->putByte
	lpm		r25, Z+							; /
	adiw	YL,  11							; Y = pointer to arguments
	rcall	xioprintf_P						;
	pop		YL								;
	pop		YH								;
	ret										;

.endfunc	// mvwprintw_P_raw
