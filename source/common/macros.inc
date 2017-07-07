;-----------------------------------------------------------------------------
; Various macros to support earlier chips, 8 or 16 bit variables, and 
; GPIO ports regardless of address
;
;	X_movw	Use "movw rdl, rdh" if chip supports it
;	X_lpm	Use "lpm reg, Z+" if chip supports it
;	X_in	Use "in reg, gpio" or "lds reg, gpio" depending on GPIO address
;	X_out	Use "out gpio, reg" or "sts gpio, reg" depending on GPIO address
;	X_inw	Reads 16 bit GPIO. Ensures low byte is read first
;	X_outw	Writes 16 bit GPIO. Ensures high byte is written first
;	X_sbi	Use "sbi gpio, bit" or "sbr" depending on GPIO address
;	X_sbif	Use "sbi gpio, bit" or "ldi, out" for GPIO interrupt flags
;	X_cbi	Use "cbi gpio, bit" or "cbr" depending on GPIO address
;	X_addw	Add two register bytes or words depending on variable size
;	X_addiw	Add reg to reg/immediate byte/word depending on variable size
;	X_subw	Subtract two register bytes or words depending on variable size
;	X_cpw	Compare two register bytes or words depending on variable size
;	X_cpzw	Compare a register byte or word to r1 depending on variable size
;	X_ldsw	Load registers with byte or word depending on variable size
;	X_ldsiw	Load addr/immediate byte or word depending on variable size
;	X_stsw	Store registers to byte or word depending on variable size
;	X_lslw	Perform an LSL on a register variable wither it is 8 or 16 bits
;	X_lsrw	Perform an LSR on a register variable wither it is 8 or 16 bits
;-----------------------------------------------------------------------------

#ifndef LIB_AVR_MACROS_INC_
#define LIB_AVR_MACROS_INC_

;-----------------------------------------------------------------------------
;  The REGNO, X_movw, and X_lpm marcros are from the avr-libc file asmdef.inc
;
;  Copyright (c) 2002, 2005, 2006, 2007 Marek Michalkiewicz
;  Copyright (c) 2006 Dmitry Xmelkov
;  All rights reserved.
;-----------------------------------------------------------------------------

;-----------------------------------------------------------------------------
;  Macro REGNO sets the symbol name 'name' to integer which is a 'reg'
;  number (0..31). If 'reg' is not a valid register, compilation error.
;  Valid regs are: r0..r31, R0..R31 and 0..31
;-----------------------------------------------------------------------------
.macro	REGNO	name, reg
  \name = -1

  .L__regno = 0
  .irp  sym,  r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, \
	     r10,r11,r12,r13,r14,r15,r16,r17,r18,r19, \
	     r20,r21,r22,r23,r24,r25,r26,r27,r28,r29, \
	     r30,r31
    .ifc  \sym,\reg
      \name = .L__regno
    .endif
    .L__regno = .L__regno + 1
  .endr

  .L__regno = 0
  .irp  sym,  R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, \
	     R10,R11,R12,R13,R14,R15,R16,R17,R18,R19, \
	     R20,R21,R22,R23,R24,R25,R26,R27,R28,R29, \
	     R30,R31
    .ifc  \sym,\reg
      \name = .L__regno
    .endif
    .L__regno = .L__regno + 1
  .endr

  .if  \name < 0
    .L__regno = 0
    .rept 32
      .if  \reg == .L__regno
        \name = .L__regno
      .endif
      .L__regno = .L__regno + 1
    .endr
  .endif

  .if  \name < 0
    .err	; Invalid reg arg.
  .endif

.endm	/* REGNO */

;-----------------------------------------------------------------------------
;  Smart version of movw:
;   - uses "movw" if possible (supported by MCU, and both registers even)
;   - handles overlapping register pairs correctly
;   - no instruction generated if source and destination are the same
;  (may expand to 0, 1 or 2 instructions).
;-----------------------------------------------------------------------------
.macro  X_movw dst, src, bits=16
	REGNO .L_movw_dst, \dst
	REGNO .L_movw_src, \src

	.if   (.L_movw_dst < 0) || (.L_movw_src < 0)
		.exitm    ; Invalid 'X_movw' arg.
	.endif

	.if ((.L_movw_src) - (.L_movw_dst))  /* different registers */
		.if (((.L_movw_src) | (.L_movw_dst)) & 0x01)
			.if \bits == 16
				.if (((.L_movw_src)-(.L_movw_dst)) & 0x80) /* src < dest */
					mov     (.L_movw_dst)+1, (.L_movw_src)+1
					mov     (.L_movw_dst), (.L_movw_src)
				.else                                      /* src > dest */
					mov     (.L_movw_dst), (.L_movw_src)
					mov     (.L_movw_dst)+1, (.L_movw_src)+1
				.endif
			.else
				mov     (.L_movw_dst), (.L_movw_src)
			.endif
		.else  /* both even -> overlap not possible */
			.if \bits == 16
#if  defined(__AVR_HAVE_MOVW__) && __AVR_HAVE_MOVW__
				movw    \dst, \src
#else
				mov     (.L_movw_dst), (.L_movw_src)
				mov     (.L_movw_dst)+1, (.L_movw_src)+1
#endif
			.else
				mov     (.L_movw_dst), (.L_movw_src)
			.endif
		.endif
	.endif
.endm

;-----------------------------------------------------------------------------
;  Macro 'X_lpm' extends enhanced lpm instruction for classic chips.
;  Usage:
;	X_lpm	reg, dst
;  where
;	reg	is 0..31, r0..r31 or R0..R31
;	dst	is z, Z, z+ or Z+
;  It is possible to omit both arguments.
;
;  Possible results for classic chips:
;	lpm
;	lpm / mov Rd,r0
;	lpm / adiw ZL,1
;	lpm / mov Rd,r0 / adiw ZL,1
;	
;  For enhanced chips it is one instruction always.
;
;  ATTENTION:  unlike enhanced chips SREG (S,V,N,Z,C) flags are
;  changed in case of 'Z+' dst.  R0 is scratch.
;-----------------------------------------------------------------------------
.macro	X_lpm	dst=r0, src=Z
  /* dst evaluation */
  REGNO .L_lpm_dst, \dst

  .if  (.L_lpm_dst < 0)
    .exitm	; Invalid dst arg of 'X_lpm' macro.
  .endif

  /* src evaluation	*/    
  .L_lpm_src = -1
  .L_lpm_n = 0
  .irp  reg,  z,Z,z+,Z+
    .ifc  \reg,\src
      .L_lpm_src = .L_lpm_n
    .endif
    .L_lpm_n = .L_lpm_n + 1
  .endr

  .if  (.L_lpm_src < 0)
    .err	; Invalid src arg of 'X_lpm' macro.
  .endif

  /* instruction(s)	*/    
  .if  .L_lpm_src < 2
    .if  .L_lpm_dst == 0
	lpm
    .else
#if  defined(__AVR_HAVE_LPMX__) && __AVR_HAVE_LPMX__
	lpm	.L_lpm_dst, Z
#else
	lpm
	mov	.L_lpm_dst, r0
#endif
    .endif
  .else
    .if  (.L_lpm_dst >= 30)
      .err	; Registers 30 and 31 are inhibited as 'X_lpm *,Z+' dst.
    .endif
#if  defined(__AVR_HAVE_LPMX__) && __AVR_HAVE_LPMX__
	lpm	.L_lpm_dst, Z+
#else
	lpm
    .if  .L_lpm_dst
	  mov	.L_lpm_dst, r0
    .endif
	adiw	r30, 1
#endif
  .endif
.endm

;-----------------------------------------------------------------------------
; X_in
;-----------------------------------------------------------------------------
.macro	X_in dst, gpio
	.if _SFR_IO_ADDR(\gpio) < 64
		in		\dst, _SFR_IO_ADDR(\gpio)
	.else
		lds		\dst, \gpio
	.endif
.endm

;-----------------------------------------------------------------------------
; X_out
;-----------------------------------------------------------------------------
.macro X_out gpio, src
	.if _SFR_IO_ADDR(\gpio) < 64
		out		_SFR_IO_ADDR(\gpio), \src
	.else
		sts		\gpio, \src
	.endif
.endm

;-----------------------------------------------------------------------------
; X_inw
; WARNING: Need to inhibit ineterrupts if dst uses TEMP1
;-----------------------------------------------------------------------------
.macro	X_inw dst, gpio, bits=8		; Read a byte/word from GPIO
	/* dst evaluation	*/
	REGNO .L_inw_dst, \dst

	.if  (.L_inw_dst < 0)
		.exitm	; Invalid dst arg of 'X_inw' macro.
	.endif

	X_in	\dst, \gpio							; Must read low byte first
	.if \bits == 16
		.if  (.L_inw_dst == 31)
			.err	; Invalid dst arg of 'X_inw' macro.
		.endif
		X_in	(.L_inw_dst)+1, (\gpio)+1
	.endif
.endm

;-----------------------------------------------------------------------------
; X_outw
; WARNING: Need to inhibit ineterrupts if src uses TEMP1
;-----------------------------------------------------------------------------
.macro	X_outw gpio, src, bits=8	; Write a byte/word to GPIO
	/* src evaluation	*/
	REGNO .L_outw_src, \src

	.if  (.L_outw_src < 0)
		.exitm	; Invalid src arg of 'X_outw' macro.
	.endif

	.if \bits == 16
		.if  (.L_outw_src == 31)
			.err	; Invalid src arg of 'X_outw' macro.
		.endif
		X_out	(\gpio)+1, (.L_outw_src)+1		; Must write high byte first
	.endif
	X_out	\gpio, \src
.endm

;-----------------------------------------------------------------------------
; X_sbi
;-----------------------------------------------------------------------------
.macro	X_sbi gpio, bit, reg
	.if _SFR_IO_ADDR(\gpio) < 32
		sbi		_SFR_IO_ADDR(\gpio), \bit
	.else
		X_in	\reg, \gpio
		sbr		\reg, 1 << \bit
		X_out	\gpio, \reg
	.endif
.endm

;-----------------------------------------------------------------------------
; X_sbif
;-----------------------------------------------------------------------------
.macro X_sbif gpio, bit, reg
	.if _SFR_IO_ADDR(\gpio) < 32
		sbi		_SFR_IO_ADDR(\gpio), \bit
	.else
		ldi		\reg, 1 << \bit
		X_out	\gpio, \reg
	.endif
.endm

;-----------------------------------------------------------------------------
; X_cbi
;-----------------------------------------------------------------------------
.macro X_cbi gpio, bit, reg
	.if _SFR_IO_ADDR(\gpio) < 32
		cbi		_SFR_IO_ADDR(\gpio), \bit
	.else
		X_in	\reg, \gpio
		cbr		\reg, 1 << \bit
		X_out	\gpio, \reg
	.endif
.endm

;-----------------------------------------------------------------------------
; X_addw
;-----------------------------------------------------------------------------
.macro	X_addw rd, rs, bits
	REGNO .L_addw_rd, \rd
	REGNO .L_addw_rs, \rs

	.if   (.L_addw_rd < 0) || (.L_addw_rs < 0)
		.exitm    ; Invalid 'X_addw' arg.
	.endif

	add		\rd, \rs
	.if \bits == 16
		adc		(.L_addw_rd)+1, (.L_addw_rs)+1
	.endif
.endm

;-----------------------------------------------------------------------------
; X_addiw
;-----------------------------------------------------------------------------
.macro	X_addiw rd, rs, bits, immediate
	REGNO .L_addiw_rd, \rd

	.if \immediate > 0
		.if   (.L_addiw_rd < 0)
			.exitm    ; Invalid 'X_addiw' arg.
		.endif

		subi	\rd, lo8(-(\rs))
		.if \bits == 16
			sbci	(.L_addiw_rd)+1, hi8(-(\rs))
		.endif
	.else
		X_addw \rd, \rs, \bits
	.endif
.endm

;-----------------------------------------------------------------------------
; X_subw
;-----------------------------------------------------------------------------
.macro	X_subw rd, rs, bits
	REGNO .L_subw_rd, \rd
	REGNO .L_subw_rs, \rs

	.if   (.L_subw_rd < 0) || (.L_subw_rs < 0)
		.exitm    ; Invalid 'X_subw' arg.
	.endif

	sub		\rd, \rs
	.if \bits == 16
		sbc		(.L_subw_rd)+1, (.L_subw_rs)+1
	.endif
.endm

;-----------------------------------------------------------------------------
; X_cpw
;-----------------------------------------------------------------------------
.macro	X_cpw rd, rs, bits
	REGNO .L_cpw_rd, \rd
	REGNO .L_cpw_rs, \rs

	.if   (.L_cpw_rd < 0) || (.L_cpw_rs < 0)
		.exitm    ; Invalid 'X_cpw' arg.
	.endif

	cp		\rd, \rs
	.if \bits == 16
		cpc		(.L_cpw_rd)+1, (.L_cpw_rs)+1
	.endif
.endm

;-----------------------------------------------------------------------------
; X_cpzw
;-----------------------------------------------------------------------------
.macro	X_cpzw rd, bits
	REGNO .L_cpzw_rd, \rd

	.if   (.L_cpzw_rd < 0)
		.exitm    ; Invalid 'X_cpw' arg.
	.endif

	cp		\rd, r1
	.if \bits == 16
		cpc		(.L_cpzw_rd)+1, r1
	.endif
.endm

;-----------------------------------------------------------------------------
; X_ldsw
;-----------------------------------------------------------------------------
.macro	X_ldsw rd, adr, bits
	REGNO .L_ldsw_rd, \rd

	.if   (.L_ldsw_rd < 0)
		.exit    ; Invalid 'X_ldsw' arg.
	.endif

	lds		\rd, \adr
	.if \bits == 16
		lds		(.L_ldsw_rd)+1, (\adr)+1
	.endif
.endm

;-----------------------------------------------------------------------------
; X_ldsiw
;-----------------------------------------------------------------------------
.macro	X_ldsiw rd, rs, bits, immediate
	REGNO .L_ldsiw_rd, \rd

	.if \immediate > 0
		.if   (.L_ldsiw_rd < 0)
			.exitm    ; Invalid 'X_ldsiw' arg.
		.endif

		ldi		\rd, lo8(\rs)
		.if \bits == 16
			ldi		(.L_ldsiw_rd)+1, hi8(\rs)
		.endif
	.else
		X_ldsw \rd, \rs, \bits
	.endif
.endm

;-----------------------------------------------------------------------------
; X_stsw
;-----------------------------------------------------------------------------
.macro	X_stsw adr, rs, bits
	REGNO .L_stsw_rs, \rs

	.if   (.L_stsw_rs < 0)
		.exitm    ; Invalid 'X_stsw' arg.
	.endif

	sts		\adr, \rs
	.if \bits == 16
		sts		(\adr)+1, (.L_stsw_rs)+1
	.endif
.endm

;-----------------------------------------------------------------------------
; X_lslw
;-----------------------------------------------------------------------------
.macro	X_lslw rd, bits
	REGNO .L_lslw_rd, \rd

	.if  (.L_lslw_rd < 0)
		.exitm	; Invalid rd arg of 'X_lslw' macro.
	.endif


	.if \bits == 8
		lsl		\rd
	.else
		lsl		(.L_lsrw_rd)+1
		rol		\rd
	.endif
.endm

;-----------------------------------------------------------------------------
; X_lsrw
;-----------------------------------------------------------------------------
.macro	X_lsrw rd, bits
	REGNO .L_lsrw_rd, \rd

	.if  (.L_lsrw_rd < 0)
		.exitm	; Invalid rd arg of 'X_lsrw' macro.
	.endif


	.if \bits == 8
		lsr		\rd
	.else
		lsr		(.L_lsrw_rd)+1
		ror		\rd
	.endif
.endm

#endif /* LIB_AVR_MACROS_INC_ */
