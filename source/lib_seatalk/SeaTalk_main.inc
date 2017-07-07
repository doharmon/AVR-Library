;---------------------------------------------------------------------------;
; Timer based software serial ports
;
; doharmon 2015
;---------------------------------------------------------------------------;

#ifdef stsBAUDRATE
	#define stsIMMEDIATE 1
#else
	#define stsIMMEDIATE 0
#endif

;---------------------------------------------------------------------------;
; Timer 0
;---------------------------------------------------------------------------;
#if (stsTX & stconfigTIMER0) || (stsRX & stconfigTIMER0)

	#ifdef stsTX
		#define stconfigTX	stsTX
	#endif
	#ifdef stsRX
		#define stconfigRX	stsRX
		#ifdef stsRX_PINS
			#define stconfigRX_PINS	stsRX_PINS
		#endif
	#endif
	#define stsFUNC_PREFIX stsFUNC_TIMER_PREFIX
	#include "SeaTalk_chips.h"
	#include "SeaTalk_timers.h"
	#include "SeaTalk_beginend.inc"

#endif

#if stsRX & stconfigTIMER0

	#ifdef stsRX_PINS
		#define stconfigRX_PINS	stsRX_PINS
	#endif

#if defined(stsRX_PINS) && (((stsRX_PINS-1) & stsRX_PINS) > 0)

	#error "Multiple PCINT pins for RX not supported"

#else // defined(stsRX_PINS) && (((stsRX_PINS-1) & stsRX_PINS) > 0)

	#if stsRX & stconfigOCR_A

		#define stconfigRX stsRX
		#define stsFUNC_PREFIX stsMAKE_FUNCNAME(stsFUNC_TIMER_PREFIX,A)
		#define stsREG_RX_SUFFIX _A
		#include "SeaTalk_rx_ir.inc"
		#include "SeaTalk_rx_funcs.inc"

	#endif

	#if stsRX & stconfigOCR_B

		#define stconfigRX stsRX
		#define stsFUNC_PREFIX stsMAKE_FUNCNAME(stsFUNC_TIMER_PREFIX,B)
		#define stsREG_RX_SUFFIX _B
		#include "SeaTalk_rx_ir.inc"
		#include "SeaTalk_rx_funcs.inc"

	#endif

	#if stsRX & stconfigOCR_C

		#define stconfigRX stsRX
		#define stsFUNC_PREFIX stsMAKE_FUNCNAME(stsFUNC_TIMER_PREFIX,C)
		#define stsREG_RX_SUFFIX _C
		#include "SeaTalk_rx_ir.inc"
		#include "SeaTalk_rx_funcs.inc"

	#endif

#endif // defined(stsRX_PINS) && (((stsRX_PINS-1) & stsRX_PINS) > 0)

#endif

#if stsTX & stconfigTIMER0

	#if stsTX & stconfigOCR_A

		#define stconfigTX ((stsTX & ~stconfigOCR_MASK) | stconfigOCR_A)
		#define stsFUNC_PREFIX stsMAKE_FUNCNAME(stsFUNC_TIMER_PREFIX,A)
		#define stsREG_TX_SUFFIX _A
		#include "SeaTalk_tx.inc"

	#endif

	#if stsTX & stconfigOCR_B

		#define stconfigTX ((stsTX & ~stconfigOCR_MASK) | stconfigOCR_B)
		#define stsFUNC_PREFIX stsMAKE_FUNCNAME(stsFUNC_TIMER_PREFIX,B)
		#define stsREG_TX_SUFFIX _B
		#include "SeaTalk_tx.inc"

	#endif
	
	#if stsTX & stconfigOCR_C

		#define stconfigTX ((stsTX & ~stconfigOCR_MASK) | stconfigOCR_C)
		#define stsFUNC_PREFIX stsMAKE_FUNCNAME(stsFUNC_TIMER_PREFIX,C)
		#define stsREG_TX_SUFFIX _C
		#include "SeaTalk_tx.inc"

	#endif

#endif

#if (stsTX & stconfigTIMER0) || (stsRX & stconfigTIMER0)

	.section stsBSS_SECTIONNAME
	stsBSS_END = .
	.global stsBSS_END

#endif

;---------------------------------------------------------------------------;
; Timer 1
;---------------------------------------------------------------------------;
#if (stsTX & stconfigTIMER1) || (stsRX & stconfigTIMER1)

	#ifdef stsTX
		#define stconfigTX	stsTX
	#endif
	#ifdef stsRX
		#define stconfigRX	stsRX
		#ifdef stsRX_PINS
			#define stconfigRX_PINS	stsRX_PINS
		#endif
	#endif
	#define stsFUNC_PREFIX stsFUNC_TIMER_PREFIX
	#include "SeaTalk_chips.h"
	#include "SeaTalk_timers.h"
	#include "SeaTalk_beginend.inc"

#endif

#if stsRX & stconfigTIMER1

	#ifdef stsRX_PINS
		#define stconfigRX_PINS	stsRX_PINS
	#endif

#if defined(stsRX_PINS) && (((stsRX_PINS-1) & stsRX_PINS) > 0)

	#error "Multiple PCINT pins for RX not supported"

#else // defined(stsRX_PINS) && (((stsRX_PINS-1) & stsRX_PINS) > 0)

	#if stsRX & stconfigOCR_A

		#define stconfigRX stsRX
		#define stsFUNC_PREFIX stsMAKE_FUNCNAME(stsFUNC_TIMER_PREFIX,A)
		#define stsREG_RX_SUFFIX _A
		#include "SeaTalk_rx_ir.inc"
		#include "SeaTalk_rx_funcs.inc"

	#endif

	#if stsRX & stconfigOCR_B

		#define stconfigRX stsRX
		#define stsFUNC_PREFIX stsMAKE_FUNCNAME(stsFUNC_TIMER_PREFIX,B)
		#define stsREG_RX_SUFFIX _B
		#include "SeaTalk_rx_ir.inc"
		#include "SeaTalk_rx_funcs.inc"

	#endif

	#if stsRX & stconfigOCR_C

		#define stconfigRX stsRX
		#define stsFUNC_PREFIX stsMAKE_FUNCNAME(stsFUNC_TIMER_PREFIX,C)
		#define stsREG_RX_SUFFIX _C
		#include "SeaTalk_rx_ir.inc"
		#include "SeaTalk_rx_funcs.inc"

	#endif

#endif // defined(stsRX_PINS) && (((stsRX_PINS-1) & stsRX_PINS) > 0)

#endif

#if stsTX & stconfigTIMER1

	#if stsTX & stconfigOCR_A

		#define stconfigTX ((stsTX & ~stconfigOCR_MASK) | stconfigOCR_A)
		#define stsFUNC_PREFIX stsMAKE_FUNCNAME(stsFUNC_TIMER_PREFIX,A)
		#define stsREG_TX_SUFFIX _A
		#include "SeaTalk_tx.inc"

	#endif

	#if stsTX & stconfigOCR_B

		#define stconfigTX ((stsTX & ~stconfigOCR_MASK) | stconfigOCR_B)
		#define stsFUNC_PREFIX stsMAKE_FUNCNAME(stsFUNC_TIMER_PREFIX,B)
		#define stsREG_TX_SUFFIX _B
		#include "SeaTalk_tx.inc"

	#endif
	
	#if stsTX & stconfigOCR_C

		#define stconfigTX ((stsTX & ~stconfigOCR_MASK) | stconfigOCR_C)
		#define stsFUNC_PREFIX stsMAKE_FUNCNAME(stsFUNC_TIMER_PREFIX,C)
		#define stsREG_TX_SUFFIX _C
		#include "SeaTalk_tx.inc"

	#endif

#endif

#if (stsTX & stconfigTIMER1) || (stsRX & stconfigTIMER1)

	.section stsBSS_SECTIONNAME
	stsBSS_END = .
	.global stsBSS_END

#endif
	
;---------------------------------------------------------------------------;
; Timer 2
;---------------------------------------------------------------------------;
#if (stsTX & stconfigTIMER2) || (stsRX & stconfigTIMER2)

	#ifdef stsTX
		#define stconfigTX	stsTX
	#endif
	#ifdef stsRX
		#define stconfigRX	stsRX
		#ifdef stsRX_PINS
			#define stconfigRX_PINS	stsRX_PINS
		#endif
	#endif
	#define stsFUNC_PREFIX stsFUNC_TIMER_PREFIX
	#include "SeaTalk_chips.h"
	#include "SeaTalk_timers.h"
	#include "SeaTalk_beginend.inc"

#endif

#if stsRX & stconfigTIMER2

	#ifdef stsRX_PINS
		#define stconfigRX_PINS	stsRX_PINS
	#endif

#if defined(stsRX_PINS) && (((stsRX_PINS-1) & stsRX_PINS) > 0)

	#error "Multiple PCINT pins for RX not supported"

#else // defined(stsRX_PINS) && (((stsRX_PINS-1) & stsRX_PINS) > 0)

	#if stsRX & stconfigOCR_A

		#define stconfigRX stsRX
		#define stsFUNC_PREFIX stsMAKE_FUNCNAME(stsFUNC_TIMER_PREFIX,A)
		#define stsREG_RX_SUFFIX _A
		#include "SeaTalk_rx_ir.inc"
		#include "SeaTalk_rx_funcs.inc"

	#endif

	#if stsRX & stconfigOCR_B

		#define stconfigRX stsRX
		#define stsFUNC_PREFIX stsMAKE_FUNCNAME(stsFUNC_TIMER_PREFIX,B)
		#define stsREG_RX_SUFFIX _B
		#include "SeaTalk_rx_ir.inc"
		#include "SeaTalk_rx_funcs.inc"

	#endif

	#if stsRX & stconfigOCR_C

		#define stconfigRX stsRX
		#define stsFUNC_PREFIX stsMAKE_FUNCNAME(stsFUNC_TIMER_PREFIX,C)
		#define stsREG_RX_SUFFIX _C
		#include "SeaTalk_rx_ir.inc"
		#include "SeaTalk_rx_funcs.inc"

	#endif

#endif // defined(stsRX_PINS) && (((stsRX_PINS-1) & stsRX_PINS) > 0)

#endif

#if stsTX & stconfigTIMER2

	#if stsTX & stconfigOCR_A

		#define stconfigTX ((stsTX & ~stconfigOCR_MASK) | stconfigOCR_A)
		#define stsFUNC_PREFIX stsMAKE_FUNCNAME(stsFUNC_TIMER_PREFIX,A)
		#define stsREG_TX_SUFFIX _A
		#include "SeaTalk_tx.inc"

	#endif

	#if stsTX & stconfigOCR_B

		#define stconfigTX ((stsTX & ~stconfigOCR_MASK) | stconfigOCR_B)
		#define stsFUNC_PREFIX stsMAKE_FUNCNAME(stsFUNC_TIMER_PREFIX,B)
		#define stsREG_TX_SUFFIX _B
		#include "SeaTalk_tx.inc"

	#endif
	
	#if stsTX & stconfigOCR_C

		#define stconfigTX ((stsTX & ~stconfigOCR_MASK) | stconfigOCR_C)
		#define stsFUNC_PREFIX stsMAKE_FUNCNAME(stsFUNC_TIMER_PREFIX,C)
		#define stsREG_TX_SUFFIX _C
		#include "SeaTalk_tx.inc"

	#endif

#endif

#if (stsTX & stconfigTIMER2) || (stsRX & stconfigTIMER2)

	.section stsBSS_SECTIONNAME
	stsBSS_END = .
	.global stsBSS_END

#endif

#undef stsTX
#undef stsRX
#undef stsRX_PINS
#undef stconfigTX
#undef stconfigRX
#undef stconfigRX_PINS
#undef stsTXDDRA
#undef stsTXPORTA
#undef stsTXPINA
#undef stsBITS
#undef stsDS
#undef stsTCCRB
#undef stsTIMSK
#undef stsNO_PRESCALE
#undef stsPRESCALE
#undef stsPRESCALE_8
#undef stsPRESCALE_32
#undef stsPRESCALE_64
#undef stsPRESCALE_128
#undef stsPRESCALE_256
#undef stsPRESCALE_1024
#undef stsBSS_START
#undef stsBSS_END
#undef stsBSS_SECTIONNAME
#undef stsDATA_SECTIONNAME
#undef stsTEXT_SECTIONNAME
#undef stsVAR_TIMER_PREFIX
#undef stsTXTCNT
#undef stsTXTCCRA
#undef stsTXFOCR
#undef stsVAR_RX_PREFIX
#undef stsRXDDR
#undef stsRXPORT
#undef stsRXINPUT
#undef stsFUNC_TIMER_PREFIX
#undef stsTXTIFR
#undef stsTX_IR_A
#undef stsTXOCR_A
#undef stsTXOCF_A
#undef stsTXCOM0_A
#undef stsTXCOM1_A
#undef stsTXFOC_A
#undef stsTXOCIE_A
#undef stsVAR_TX_PREFIX_A
#undef stsRXTIFR
#undef stsRX_IR
#undef stsRXTCNT
#undef MAX_TICKS_PER_BIT
#undef stsRXPCMSK
#undef stsRXINT
#undef stsRXINTF
#undef stsFUNC_PREFIX_BASE
#undef stsVAR_RX_PCPORT
#undef rx_state
#undef rx_bit
#undef rx_byte
#undef rx_target
#undef rx_buffer_head
#undef rx_buffer_tail
#undef rx_buffer_overflow
#undef rx_buffer
#undef stsVAR_RX_SUFFIX
#undef stsRX_SB_IR
#undef stsRXOCR
#undef stsRXOCF
#undef stsRXOCIE
#undef stsVAR_RX_PREFIX_B
#undef stsRXEICRA
#undef stsREG_INT_SUFFIX
#undef stsREG_RX_SUFFIX
#undef stsRXEIMSK
#undef stsRXEIFR
