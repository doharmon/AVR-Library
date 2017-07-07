;---------------------------------------------------------------------------;
; Integer math routines
;
; doharmon 2015
;---------------------------------------------------------------------------;

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
; Source for above:
; http://www.atmel.com/webdoc/AVRLibcReferenceManual/FAQ_1faq_reg_usage.html
;---------------------------------------------------------------------------;

.nolist
#include <avr/io.h>					// Include device specific definitions.
#include "../common/macros.inc"		// Assembler macros
.list

#define	a0	r22
#define	a1	r23
#define	a2	r24
#define	a3	r25

#define	b0	r18
#define	b1	r19
#define	b2	r20
#define	b3	r21

#define	t0	r26
#define	t1	r27
#define	t2	r30
#define	t3	r31

#define	t4	r0

.section .text.lib_math.udiv32

;************************************************************************
;*                                                                      *
;*                      unsigned rounded division 32 bit                *
;*                                                                      *
;************************************************************************
; Original source: http://www.mikrocontroller.net/articles/AVR_Arithmetik

.global	urdiv32
.func	urdiv32

urdiv32:
	X_movw	t0, b0		;T = B
	X_movw	t2, b2	
	lsr		t3			;B / 2
	ror		t2
	ror		t1
	ror		t0
	add		a0, t0		;A = A + B / 2
	adc		a1, t1
	adc		a2, t2
	adc		a3, t3

;************************************************************************
;*                                                                      *
;*                      unsigned division 32 bit                        *
;*                                                                      *
;************************************************************************

; a3..0 = a3..0 / b3..0 (Ganzzahldivision)
; b3..0 = a3..0 % b3..0 (Rest)

; cycle: max 684

.global	udiv32
udiv32:
	ldi		t0, 32
	mov		t4, t0
	clr		t0
	clr		t1
	clr		t2
	clr		t3
udi1:
	lsl		a0
	rol		a1
	rol		a2
	rol		a3
	rol		t0
	rol		t1
	rol		t2
	rol		t3
	cp		t0, b0
	cpc		t1, b1
	cpc		t2, b2
	cpc		t3, b3
	brcs	udi2
	sub		t0, b0
	sbc		t1, b1
	sbc		t2, b2
	sbc		t3, b3
	inc		a0
udi2:
	dec		t4
	brne	udi1

	X_movw	b0, t0
	X_movw	b2, t2	
	ret

.endfunc	; urdiv32

#undef	a0
#undef	a1
#undef	a2
#undef	a3

#undef	b0
#undef	b1
#undef	b2
#undef	b3

#undef	t0
#undef	t1
#undef	t2
#undef	t3

#undef	t4

;************************************************************************
;*                                                                      *
;*                      uint32/10 keep top 32 bits                      *
;*                                                                      *
;************************************************************************
.section .text.lib_math.u32div10
.global	u32div10
.func	u32div10

u32div10:
	; 32x32_Top32: Multiply 32 by 32 bits. Keep top 32 bits.
	; Divide integer by 10
	; 0x1999999A as q0.32 format of 0.10
	; R31 = 0x19; R30 = 0x99; R26 = 0x9A
	; 25 24 23 22  32
	;  0  0 R1  0  Lo  R18*9A  R23 = R1. Clear R22, R24, R25
	;    R1:R0     Lo  R19*9A
	;  c R1:R0     Lo  R18*99
	; R1:R0        Lo  R18*99  Reuse R1:R0
	; R1:R0     c  Lo  R20*9A
	; R1:R0  0  c  Lo  R19*99  Clear R23
	; R0       R1  L/H R19*99  Reuse R1:R0
	; R0       R1  L/H R18*19
	; R0     c R1  L/H R21*9A
	; R0  0  c R1  L/H R20*99  Clear R24
	;       R1:R0  Hi  R20*99  Reuse R1:R0
	;       R1:R0  Hi  R19*19
	;     c R1:R0  Hi  R21*99
	;  0 R1:R0     Hi  R21*99  Reuse R1:R0 Clear R25
	;    R1:R0     Hi  R20*19
	; R1:R0        Hi  R21*19

	; r25:r22 = r21:r18 / 10 or r25:r22 = r21:r18 * 0.10
	; This is done by multiplying integer with 0x19999999A
	X_movw	r20, r24
	X_movw	r18, r22
	clr		r27				; Used as zero
	ldi		r31, 0x19		;
	ldi		r30, 0x99		;
	ldi		r26, 0x9A		;

	mul		r18, r26		; Row 1
	mov		r23, r1			;
	clr		r22				;
	clr		r24				;
	clr		r25				;

	mul		r19, r26		; Row 2
	add		r23, r0			;
	adc		r24, r1			;
;	adc		r25, r27		; Carry needed? No

	mul		r18, r30		; Row 3
	add		r23, r0			;
	adc		r24, r1			;
	adc		r25, r27		; Carry needed? Yes

;	mul		r18, r30		;
	add		r24, r0			; Row 4
	adc		r25, r1			;
;	adc		r22, r27		; Carry needed? No

	mul		r20, r26		; Row 5
	add		r24, r0			;
	adc		r25, r1			;
	adc		r22, r27		; Carry needed? Yes

	mul		r19, r30		; Row 6
	add		r24, r0			;
	adc		r25, r1			;
	adc		r22, r27		; Carry needed? Yes

	clr		r23				;

;	mul		r19, r30		;
	add		r25, r0			; Row 7
	adc		r22, r1			;
;	adc		r23, r27		; Carry needed? No

	mul		r18, r31		; Row 8
	add		r25, r0			;
	adc		r22, r1			;
;	adc		r23, r27		; Carry needed? No

	mul		r21, r26		; Row 9
	add		r25, r0			;
	adc		r22, r1			;
	adc		r23, r27		; Carry needed? Yes

	mul		r20, r30		; Row 10
	add		r25, r0			;
	adc		r22, r1			;
	adc		r23, r27		; Carry needed? Yes

	clr		r24				;

;	mul		r20, r30		;
	add		r22, r0			; Row 11
	adc		r23, r1			;
;	adc		r24, r27		; Carry needed? No

	mul		r19, r31		; Row 12
	add		r22, r0			;
	adc		r23, r1			;
;	adc		r24, r27		; Carry needed? No

	mul		r21, r30		; Row 13
	add		r22, r0			;
	adc		r23, r1			;
	adc		r24, r27		; Carry needed? Yes

;	mul		r21, r30		;
	add		r23, r0			; Row 14
	adc		r24, r1			;
;	adc		r25, r27		; Carry needed? No

	clr		r25				;

	mul		r20, r31		; Row 15
	add		r23, r0			;
	adc		r24, r1			;
;	adc		r25, r27		; Carry needed? No

	mul		r21, r31		; Row 16
	add		r24, r0			;
	adc		r25, r1			;

	clr		r1				;

	ret

.endfunc	; u32div10

;************************************************************************
;*                                                                      *
;*                uint32/10 return (s17.14)                             *
;*                                                                      *
;************************************************************************
.section .text.lib_math.q14u32div10
.global	q14u32div10
.func	q14u32div10

; R21:R18			Argument
; R25:R22			Result
; R26				Zero
; R31:R31:R31:R30	Multiplier 0xCCCCCCCD = 0.1 shifted left 3
q14u32div10:
	; Divide integer by 10
	; 0xCCCCCCCD as q0.32 format of 0.10 shifted left 3
	; R31 = 0xCC; R30 = 0xCD
	; 25 24 23 22  32
	;  0 R1  0  0  Lo  R18*CD  R24 = R1. Clear R22, R23, R25
	; R1:R0        Lo  R19*CD
	; R1:R0     c  Lo  R18*CC
	; R0       R1  Lo  R18*CC  Reuse R1:R0
	; R0     c R1  Lo  R20*CD
	; R0  0  c R1  Lo  R19*CC  Clear R24
	;  *    R1:R0  L/H R19*CC  Reuse R1:R0 *:Keep byte for ROL
	;       R1:R0  L/H R18*CC
	;     c R1:R0  L/H R21*CD
	;  0  c R1:R0  L/H R20*CC  Clear R25
	;    R1:R0     Hi  R20*CC  Reuse R1:R0
	;    R1:R0     Hi  R19*CC
	;  c R1:R0     Hi  R21*CC
	; R1:R0        Hi  R21*CC  Reuse R1:R0
	; R1:R0        Hi  R20*CC
	; R0           Hi  R21*CC  Discard R1

	; r25:r22 = r21:r18 / 10 or r25:r22 = r21:r18 * 0.10
	; This is done by multiplying integer with 0xCCCCCCCD
	X_movw	r20, r24		;
	X_movw	r18, r22		;
	clr		r26				; Used as zero
	ldi		r30, 0xCD		;
	ldi		r31, 0xCC		;

	mul		r18, r30		; Row 1
	mov		r24, r1			;
	clr		r22				;
	clr		r23				;
	clr		r25				;

	mul		r19, r30		; Row 2
	add		r24, r0			;
	adc		r25, r1			;
;	adc		r22, r26		; Carry needed? No

	mul		r18, r31		; Row 3
	add		r24, r0			;
	adc		r25, r1			;
	adc		r22, r26		; Carry needed? Yes

;	mul		r18, r31		; Row 4
	add		r25, r0			;
	adc		r22, r1			;
;	adc		r23, r26		; Carry needed? No

	mul		r20, r30		; Row 5
	add		r25, r0			;
	adc		r22, r1			;
	adc		r23, r26		; Carry needed? Yes

	mul		r19, r31		; Row 6
	add		r25, r0			;
	adc		r22, r1			;
	adc		r23, r26		; Carry needed? Yes

	clr		r24				;

;	mul		r19, r31		; Row 7
	add		r22, r0			;
	adc		r23, r1			;
;	adc		r24, r26		; Carry needed? No

	mul		r18, r31		; Row 8
	add		r22, r0			;
	adc		r23, r1			;
	adc		r24, r26		; Carry needed? Yes

	mul		r21, r30		; Row 9
	add		r22, r0			;
	adc		r23, r1			;
	adc		r24, r26		; Carry needed? Yes

	mul		r20, r31		; Row 10
	add		r22, r0			;
	adc		r23, r1			;
	adc		r24, r26		; Carry needed? Yes

	mov		r18, r25		; Save R25
	clr		r25				;

;	mul		r20, r31		; Row 11
	add		r23, r0			;
	adc		r24, r1			;
;	adc		r25, r26		; Carry needed? No

	mul		r19, r31		; Row 12
	add		r23, r0			;
	adc		r24, r1			;
;	adc		r25, r26		; Carry needed? No

	mul		r21, r31		; Row 13
	add		r23, r0			;
	adc		r24, r1			;
	adc		r25, r26		; Carry needed? Yes

;	mul		r21, r31		; Row 14
	add		r24, r0			;
	adc		r25, r1			;
;	adc		r22, r26		; Carry needed? No

	mul		r20, r31		; Row 15
	add		r24, r0			;
	adc		r25, r1			;
	adc		r22, r26		; Carry needed? No

	mul		r21, r31		; Row 16
	add		r25, r0			;
;	adc		r22, r1			;

	ldi		r26, 0b100		; Rotate left 3 times (set third bit)
01:
	rol		r18				; Rotate for s17.14
	rol		r22				;
	rol		r23				;
	rol		r24				;
	rol		r25				; /
	ror		r26				;
	brcc	01b				;

	clr		r1				;

	ret

.endfunc	; q14u32div10

;************************************************************************
;*                                                                      *
;*               uint32/100 return _Accum (s16.15)                      *
;*                                                                      *
;************************************************************************
.section .text.lib_math.ku32div100
.global	ku32div100
.func	ku32div100

; 0.01 in binary: 0.00000010100011110101110000101000111101
; Roatate left 6 keep first 32 bits: 10100011110101110000101000111101
;									 0xA3D70A3D
; R21:R18			Argument
; R25:R22			Result
; R26				Zero
; R31:R30:R27:R31	Multiplier 0xA3D70A3D = 0.01
ku32div100:
	; Divide integer by 100
	; 0xA3D70A3D as q0.32 format of 0.01
	; R31 = 0xA3; R30 = 0x99; R31 = 0x3D
	; 25 24 23 22  32
	;  0 R1  0  0  Lo  R18*3D  R24 = R1. Clear R22, R23, R25
	; R1:R0        Lo  R18*0A
	; R1:R0     c  Lo  R19*3D
	; R0       R1  Lo  R18*D7
	; R0     c R1  Lo  R19*0A
	; R0  0  c R1  Lo  R20*3D  Clear R24
	;  *    R1:R0  L/H R21*3D  *:Keep high bit for ROL
	;       R1:R0  L/H R18*A3
	;     c R1:R0  L/H R19*D7
	;  0  c R1:R0  L/H R20*0A  Clear R25
	;    R1:R0     Hi  R19*A3
	;    R1:R0     Hi  R20*D7
	;  c R1:R0     Hi  R21*0A
	; R1:R0        Hi  R20*A3
	; R1:R0        Hi  R21*D7
	; R0           Hi  R21*A3  Discard R1

	; r25:r22 = r21:r18 / 100 or r25:r22 = r21:r18 * 0.01
	; This is done by multiplying integer with 0xA3D70A3D
	; Rather than roll the result right 6 bits roll left
	; one bit. (If there were no sign bit then roll 2)
	; Result will be fixed point s16.15 in R25:R22.
	X_movw	r20, r24		;
	X_movw	r18, r22		;
	clr		r26				; Used as zero
	ldi		r30, 0xD7		;
	ldi		r27, 0x0A		;
	ldi		r31, 0x3D		;

	mul		r18, r31		; Row 1
	mov		r24, r1			;
	clr		r22				;
	clr		r23				;
	clr		r25				;

	mul		r18, r27		; Row 2
	add		r24, r0			;
	adc		r25, r1			;
;	adc		r22, r26		; Carry needed? No

	mul		r19, r31		; Row 3
	add		r24, r0			;
	adc		r25, r1			;
	adc		r22, r26		; Carry needed? Yes

	mul		r18, r30		; Row 4
	add		r25, r0			;
	adc		r22, r1			;
;	adc		r23, r26		; Carry needed? No

	mul		r19, r27		; Row 5
	add		r25, r0			;
	adc		r22, r1			;
	adc		r23, r26		; Carry needed? Yes

	mul		r20, r31		; Row 6
	add		r25, r0			;
	adc		r22, r1			;
	adc		r23, r26		; Carry needed? Yes

	clr		r24				;

	mul		r21, r31		; Row 7
	add		r22, r0			;
	adc		r23, r1			;
;	adc		r24, r26		; Carry needed? No

	ldi		r31, 0xA3		; No longer need 0x3D

	mul		r18, r31		; Row 8
	add		r22, r0			;
	adc		r23, r1			;
;	adc		r24, r26		; Carry needed? No

	mul		r19, r30		; Row 9
	add		r22, r0			;
	adc		r23, r1			;
	adc		r24, r26		; Carry needed? Yes

	mul		r20, r27		; Row 10
	add		r22, r0			;
	adc		r23, r1			;
	adc		r24, r26		; Carry needed? Yes

	mov		r18, r25		; Save R25
	clr		r25				;

	mul		r19, r31		; Row 11
	add		r23, r0			;
	adc		r24, r1			;
;	adc		r25, r26		; Carry needed? No

	mul		r20, r30		; Row 12
	add		r23, r0			;
	adc		r24, r1			;
;	adc		r25, r26		; Carry needed? No

	mul		r21, r27		; Row 13
	add		r23, r0			;
	adc		r24, r1			;
	adc		r25, r26		; Carry needed? Yes

	mul		r20, r31		; Row 14
	add		r24, r0			;
	adc		r25, r1			;
;	adc		r22, r26		; Carry needed? No

	mul		r21, r30		; Row 15
	add		r24, r0			;
	adc		r25, r1			;
;	adc		r22, r26		; Carry needed? No

	mul		r21, r31		; Row 16
	add		r25, r0			;
;	adc		r22, r1			;

	rol		r18				; Rotate for s16.15
	rol		r22				;
	rol		r23				;
	rol		r24				;
	rol		r25				; /

	clr		r1				;

	ret

.endfunc	; ku32div100

