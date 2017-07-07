;---------------------------------------------------------------------------;
; Timer based software serial ports
;
; doharmon 2015
;---------------------------------------------------------------------------;

#ifdef tsBAUDRATE
	#define tsIMMEDIATE 1
#else
	#define tsIMMEDIATE 0
#endif

;---------------------------------------------------------------------------;
; Timer 0
;---------------------------------------------------------------------------;
#if (tsTX & tsconfigTIMER0) || (tsRX & tsconfigTIMER0)

	#ifdef tsTX
		#define tsconfigTX	tsTX
	#endif
	#ifdef tsRX
		#define tsconfigRX	tsRX
		#ifdef tsRX_PINS
			#define tsconfigRX_PINS	tsRX_PINS
		#endif
	#endif
	#define tsFUNC_PREFIX tsFUNC_TIMER_PREFIX
	#include "tserial_chips.h"
	#include "tserial_timers.h"
	#include "tserial_beginend.inc"

#endif

#if tsTX & tsconfigTIMER0

	#if tsTX & tsconfigOCR_A

		#define tsconfigTX ((tsTX & ~tsconfigOCR_MASK) | tsconfigOCR_A)
		#if tsTX & tsconfigASCII
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,AAscii)
		#else
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,A)
		#endif
		#define tsREG_TX_SUFFIX _A
		#include "tserial_tx.inc"

	#endif

	#if tsTX & tsconfigOCR_B

		#define tsconfigTX ((tsTX & ~tsconfigOCR_MASK) | tsconfigOCR_B)
		#if tsTX & tsconfigASCII
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,BAscii)
		#else
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,B)
		#endif
		#define tsREG_TX_SUFFIX _B
		#include "tserial_tx.inc"

	#endif
	
	#if tsTX & tsconfigOCR_C

		#define tsconfigTX ((tsTX & ~tsconfigOCR_MASK) | tsconfigOCR_C)
		#if tsTX & tsconfigASCII
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,CAscii)
		#else
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,C)
		#endif
		#define tsREG_TX_SUFFIX _C
		#include "tserial_tx.inc"

	#endif

#endif

#if tsRX & tsconfigTIMER0

	#ifdef tsRX_PINS
		#define tsconfigRX_PINS	tsRX_PINS
	#endif

#if defined(tsRX_PINS) && (((tsRX_PINS-1) & tsRX_PINS) > 0)

	#if !(tsRX & tsconfigASCII)
		#error "Multiple PCINT pins for RX only supported in ASCII mode"
	#endif

	#define tsconfigRX tsRX

	#if tsconfigRX & tsconfigPCPORTA
		#define tsFUNC_PREFIX_BASE tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,PA)
	#elif tsconfigRX & tsconfigPCPORTB
		#define tsFUNC_PREFIX_BASE tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,PB)
	#elif tsconfigRX & tsconfigPCPORTC
		#define tsFUNC_PREFIX_BASE tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,PC)
	#elif tsconfigRX & tsconfigPCPORTD
		#define tsFUNC_PREFIX_BASE tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,PD)
	#else
		#error "Invalid port defined for RX"
	#endif

	#include "tserial_rx_ir.inc"

	#if tsRX_PINS & tsconfigPCBIT0
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_PREFIX_BASE,0Ascii)
		#define tsconfigPCBIT 0
		#include "tserial_rx_funcs.inc"
	#endif

	#if tsRX_PINS & tsconfigPCBIT1
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_PREFIX_BASE,1Ascii)
		#define tsconfigPCBIT 1
		#include "tserial_rx_funcs.inc"
	#endif

	#if tsRX_PINS & tsconfigPCBIT2
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_PREFIX_BASE,2Ascii)
		#define tsconfigPCBIT 2
		#include "tserial_rx_funcs.inc"
	#endif

	#if tsRX_PINS & tsconfigPCBIT3
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_PREFIX_BASE,3Ascii)
		#define tsconfigPCBIT 3
		#include "tserial_rx_funcs.inc"
	#endif

	#if tsRX_PINS & tsconfigPCBIT4
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_PREFIX_BASE,4Ascii)
		#define tsconfigPCBIT 4
		#include "tserial_rx_funcs.inc"
	#endif

	#if tsRX_PINS & tsconfigPCBIT5
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_PREFIX_BASE,5Ascii)
		#define tsconfigPCBIT 5
		#include "tserial_rx_funcs.inc"
	#endif

	#if tsRX_PINS & tsconfigPCBIT6
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_PREFIX_BASE,6Ascii)
		#define tsconfigPCBIT 6
		#include "tserial_rx_funcs.inc"
	#endif

	#if tsRX_PINS & tsconfigPCBIT7
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_PREFIX_BASE,7Ascii)
		#define tsconfigPCBIT 7
		#include "tserial_rx_funcs.inc"
	#endif

#else // defined(tsRX_PINS) && (((tsRX_PINS-1) & tsRX_PINS) > 0)

	#if tsRX & tsconfigOCR_A

		#define tsconfigRX tsRX
		#if tsRX & tsconfigASCII
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,AAscii)
		#else
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,A)
		#endif
		#define tsREG_RX_SUFFIX _A
		#include "tserial_rx_ir.inc"
		#include "tserial_rx_funcs.inc"

	#endif

	#if tsRX & tsconfigOCR_B

		#define tsconfigRX tsRX
		#if tsRX & tsconfigASCII
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,BAscii)
		#else
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,B)
		#endif
		#define tsREG_RX_SUFFIX _B
		#include "tserial_rx_ir.inc"
		#include "tserial_rx_funcs.inc"

	#endif

	#if tsRX & tsconfigOCR_C

		#define tsconfigRX tsRX
		#if tsRX & tsconfigASCII
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,CAscii)
		#else
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,C)
		#endif
		#define tsREG_RX_SUFFIX _C
		#include "tserial_rx_ir.inc"
		#include "tserial_rx_funcs.inc"

	#endif

#endif // defined(tsRX_PINS) && (((tsRX_PINS-1) & tsRX_PINS) > 0)

#endif

#if (tsTX & tsconfigTIMER0) || (tsRX & tsconfigTIMER0)

	.section tsBSS_SECTIONNAME
	tsBSS_END = .
	.global tsBSS_END
	
#endif

;---------------------------------------------------------------------------;
; Timer 1
;---------------------------------------------------------------------------;
#if (tsTX & tsconfigTIMER1) || (tsRX & tsconfigTIMER1)

	#ifdef tsTX
		#define tsconfigTX	tsTX
	#endif
	#ifdef tsRX
		#define tsconfigRX	tsRX
		#ifdef tsRX_PINS
			#define tsconfigRX_PINS	tsRX_PINS
		#endif
	#endif
	#define tsFUNC_PREFIX tsFUNC_TIMER_PREFIX
	#include "tserial_chips.h"
	#include "tserial_timers.h"
	#include "tserial_beginend.inc"

#endif

#if tsTX & tsconfigTIMER1

	#if tsTX & tsconfigOCR_A

		#define tsconfigTX ((tsTX & ~tsconfigOCR_MASK) | tsconfigOCR_A)
		#if tsTX & tsconfigASCII
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,AAscii)
		#else
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,A)
		#endif
		#define tsREG_TX_SUFFIX _A
		#include "tserial_tx.inc"

	#endif

	#if tsTX & tsconfigOCR_B

		#define tsconfigTX ((tsTX & ~tsconfigOCR_MASK) | tsconfigOCR_B)
		#if tsTX & tsconfigASCII
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,BAscii)
		#else
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,B)
		#endif
		#define tsREG_TX_SUFFIX _B
		#include "tserial_tx.inc"

	#endif
	
	#if tsTX & tsconfigOCR_C

		#define tsconfigTX ((tsTX & ~tsconfigOCR_MASK) | tsconfigOCR_C)
		#if tsTX & tsconfigASCII
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,CAscii)
		#else
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,C)
		#endif
		#define tsREG_TX_SUFFIX _C
		#include "tserial_tx.inc"

	#endif

#endif

#if tsRX & tsconfigTIMER1

	#ifdef tsRX_PINS
		#define tsconfigRX_PINS	tsRX_PINS
	#endif

#if defined(tsRX_PINS) && (((tsRX_PINS-1) & tsRX_PINS) > 0)

	#if !(tsRX & tsconfigASCII)
		#error "Multiple PCINT pins for RX only supported in ASCII mode"
	#endif

	#define tsconfigRX tsRX

	#if tsconfigRX & tsconfigPCPORTA
		#define tsFUNC_PREFIX_BASE tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,PA)
	#elif tsconfigRX & tsconfigPCPORTB
		#define tsFUNC_PREFIX_BASE tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,PB)
	#elif tsconfigRX & tsconfigPCPORTC
		#define tsFUNC_PREFIX_BASE tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,PC)
	#elif tsconfigRX & tsconfigPCPORTD
		#define tsFUNC_PREFIX_BASE tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,PD)
	#else
		#error "Invalid port defined for RX"
	#endif

	#include "tserial_rx_ir.inc"

	#if tsRX_PINS & tsconfigPCBIT0
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_PREFIX_BASE,0Ascii)
		#define tsconfigPCBIT 0
		#include "tserial_rx_funcs.inc"
	#endif

	#if tsRX_PINS & tsconfigPCBIT1
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_PREFIX_BASE,1Ascii)
		#define tsconfigPCBIT 1
		#include "tserial_rx_funcs.inc"
	#endif

	#if tsRX_PINS & tsconfigPCBIT2
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_PREFIX_BASE,2Ascii)
		#define tsconfigPCBIT 2
		#include "tserial_rx_funcs.inc"
	#endif

	#if tsRX_PINS & tsconfigPCBIT3
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_PREFIX_BASE,3Ascii)
		#define tsconfigPCBIT 3
		#include "tserial_rx_funcs.inc"
	#endif

	#if tsRX_PINS & tsconfigPCBIT4
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_PREFIX_BASE,4Ascii)
		#define tsconfigPCBIT 4
		#include "tserial_rx_funcs.inc"
	#endif

	#if tsRX_PINS & tsconfigPCBIT5
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_PREFIX_BASE,5Ascii)
		#define tsconfigPCBIT 5
		#include "tserial_rx_funcs.inc"
	#endif

	#if tsRX_PINS & tsconfigPCBIT6
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_PREFIX_BASE,6Ascii)
		#define tsconfigPCBIT 6
		#include "tserial_rx_funcs.inc"
	#endif

	#if tsRX_PINS & tsconfigPCBIT7
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_PREFIX_BASE,7Ascii)
		#define tsconfigPCBIT 7
		#include "tserial_rx_funcs.inc"
	#endif

#else // defined(tsRX_PINS) && (((tsRX_PINS-1) & tsRX_PINS) > 0)

	#if tsRX & tsconfigOCR_A

		#define tsconfigRX tsRX
		#if tsRX & tsconfigASCII
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,AAscii)
		#else
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,A)
		#endif
		#define tsREG_RX_SUFFIX _A
		#include "tserial_rx_ir.inc"
		#include "tserial_rx_funcs.inc"

	#endif

	#if tsRX & tsconfigOCR_B

		#define tsconfigRX tsRX
		#if tsRX & tsconfigASCII
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,BAscii)
		#else
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,B)
		#endif
		#define tsREG_RX_SUFFIX _B
		#include "tserial_rx_ir.inc"
		#include "tserial_rx_funcs.inc"

	#endif

	#if tsRX & tsconfigOCR_C

		#define tsconfigRX tsRX
		#if tsRX & tsconfigASCII
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,CAscii)
		#else
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,C)
		#endif
		#define tsREG_RX_SUFFIX _C
		#include "tserial_rx_ir.inc"
		#include "tserial_rx_funcs.inc"

	#endif

#endif // defined(tsRX_PINS) && (((tsRX_PINS-1) & tsRX_PINS) > 0)

#endif

#if (tsTX & tsconfigTIMER1) || (tsRX & tsconfigTIMER1)

	.section tsBSS_SECTIONNAME
	tsBSS_END = .
	.global tsBSS_END

#endif
	
;---------------------------------------------------------------------------;
; Timer 2
;---------------------------------------------------------------------------;
#if (tsTX & tsconfigTIMER2) || (tsRX & tsconfigTIMER2)

	#ifdef tsTX
		#define tsconfigTX	tsTX
	#endif
	#ifdef tsRX
		#define tsconfigRX	tsRX
		#ifdef tsRX_PINS
			#define tsconfigRX_PINS	tsRX_PINS
		#endif
	#endif
	#define tsFUNC_PREFIX tsFUNC_TIMER_PREFIX
	#include "tserial_chips.h"
	#include "tserial_timers.h"
	#include "tserial_beginend.inc"

#endif

#if tsTX & tsconfigTIMER2

	#if tsTX & tsconfigOCR_A

		#define tsconfigTX ((tsTX & ~tsconfigOCR_MASK) | tsconfigOCR_A)
		#if tsTX & tsconfigASCII
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,AAscii)
		#else
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,A)
		#endif
		#define tsREG_TX_SUFFIX _A
		#include "tserial_tx.inc"

	#endif

	#if tsTX & tsconfigOCR_B

		#define tsconfigTX ((tsTX & ~tsconfigOCR_MASK) | tsconfigOCR_B)
		#if tsTX & tsconfigASCII
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,BAscii)
		#else
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,B)
		#endif
		#define tsREG_TX_SUFFIX _B
		#include "tserial_tx.inc"

	#endif
	
	#if tsTX & tsconfigOCR_C

		#define tsconfigTX ((tsTX & ~tsconfigOCR_MASK) | tsconfigOCR_C)
		#if tsTX & tsconfigASCII
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,CAscii)
		#else
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,C)
		#endif
		#define tsREG_TX_SUFFIX _C
		#include "tserial_tx.inc"

	#endif

#endif

#if tsRX & tsconfigTIMER2

	#ifdef tsRX_PINS
		#define tsconfigRX_PINS	tsRX_PINS
	#endif

#if defined(tsRX_PINS) && (((tsRX_PINS-1) & tsRX_PINS) > 0)

	#if !(tsRX & tsconfigASCII)
		#error "Multiple PCINT pins for RX only supported in ASCII mode"
	#endif

	#define tsconfigRX tsRX

	#if tsconfigRX & tsconfigPCPORTA
		#define tsFUNC_PREFIX_BASE tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,PA)
	#elif tsconfigRX & tsconfigPCPORTB
		#define tsFUNC_PREFIX_BASE tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,PB)
	#elif tsconfigRX & tsconfigPCPORTC
		#define tsFUNC_PREFIX_BASE tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,PC)
	#elif tsconfigRX & tsconfigPCPORTD
		#define tsFUNC_PREFIX_BASE tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,PD)
	#else
		#error "Invalid port defined for RX"
	#endif

	#include "tserial_rx_ir.inc"

	#if tsRX_PINS & tsconfigPCBIT0
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_PREFIX_BASE,0Ascii)
		#define tsconfigPCBIT 0
		#include "tserial_rx_funcs.inc"
	#endif

	#if tsRX_PINS & tsconfigPCBIT1
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_PREFIX_BASE,1Ascii)
		#define tsconfigPCBIT 1
		#include "tserial_rx_funcs.inc"
	#endif

	#if tsRX_PINS & tsconfigPCBIT2
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_PREFIX_BASE,2Ascii)
		#define tsconfigPCBIT 2
		#include "tserial_rx_funcs.inc"
	#endif

	#if tsRX_PINS & tsconfigPCBIT3
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_PREFIX_BASE,3Ascii)
		#define tsconfigPCBIT 3
		#include "tserial_rx_funcs.inc"
	#endif

	#if tsRX_PINS & tsconfigPCBIT4
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_PREFIX_BASE,4Ascii)
		#define tsconfigPCBIT 4
		#include "tserial_rx_funcs.inc"
	#endif

	#if tsRX_PINS & tsconfigPCBIT5
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_PREFIX_BASE,5Ascii)
		#define tsconfigPCBIT 5
		#include "tserial_rx_funcs.inc"
	#endif

	#if tsRX_PINS & tsconfigPCBIT6
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_PREFIX_BASE,6Ascii)
		#define tsconfigPCBIT 6
		#include "tserial_rx_funcs.inc"
	#endif

	#if tsRX_PINS & tsconfigPCBIT7
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_PREFIX_BASE,7Ascii)
		#define tsconfigPCBIT 7
		#include "tserial_rx_funcs.inc"
	#endif

#else // defined(tsRX_PINS) && (((tsRX_PINS-1) & tsRX_PINS) > 0)

	#if tsRX & tsconfigOCR_A

		#define tsconfigRX tsRX
		#if tsRX & tsconfigASCII
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,AAscii)
		#else
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,A)
		#endif
		#define tsREG_RX_SUFFIX _A
		#include "tserial_rx_ir.inc"
		#include "tserial_rx_funcs.inc"

	#endif

	#if tsRX & tsconfigOCR_B

		#define tsconfigRX tsRX
		#if tsRX & tsconfigASCII
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,BAscii)
		#else
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,B)
		#endif
		#define tsREG_RX_SUFFIX _B
		#include "tserial_rx_ir.inc"
		#include "tserial_rx_funcs.inc"

	#endif

	#if tsRX & tsconfigOCR_C

		#define tsconfigRX tsRX
		#if tsRX & tsconfigASCII
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,CAscii)
		#else
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,C)
		#endif
		#define tsREG_RX_SUFFIX _C
		#include "tserial_rx_ir.inc"
		#include "tserial_rx_funcs.inc"

	#endif

#endif // defined(tsRX_PINS) && (((tsRX_PINS-1) & tsRX_PINS) > 0)

#endif

#if (tsTX & tsconfigTIMER2) || (tsRX & tsconfigTIMER2)

	.section tsBSS_SECTIONNAME
	tsBSS_END = .
	.global tsBSS_END

#endif

;---------------------------------------------------------------------------;
; Timer 3
;---------------------------------------------------------------------------;
#if (tsTX & tsconfigTIMER3) || (tsRX & tsconfigTIMER3)

	#ifdef tsTX
		#define tsconfigTX	tsTX
	#endif
	#ifdef tsRX
		#define tsconfigRX	tsRX
		#ifdef tsRX_PINS
			#define tsconfigRX_PINS	tsRX_PINS
		#endif
	#endif
	#define tsFUNC_PREFIX tsFUNC_TIMER_PREFIX
	#include "tserial_chips.h"
	#include "tserial_timers.h"
	#include "tserial_beginend.inc"

#endif

#if tsTX & tsconfigTIMER3

	#if tsTX & tsconfigOCR_A

		#define tsconfigTX ((tsTX & ~tsconfigOCR_MASK) | tsconfigOCR_A)
		#if tsTX & tsconfigASCII
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,AAscii)
		#else
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,A)
		#endif
		#define tsREG_TX_SUFFIX _A
		#include "tserial_tx.inc"

	#endif

	#if tsTX & tsconfigOCR_B

		#define tsconfigTX ((tsTX & ~tsconfigOCR_MASK) | tsconfigOCR_B)
		#if tsTX & tsconfigASCII
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,BAscii)
		#else
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,B)
		#endif
		#define tsREG_TX_SUFFIX _B
		#include "tserial_tx.inc"

	#endif
	
	#if tsTX & tsconfigOCR_C

		#define tsconfigTX ((tsTX & ~tsconfigOCR_MASK) | tsconfigOCR_C)
		#if tsTX & tsconfigASCII
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,CAscii)
		#else
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,C)
		#endif
		#define tsREG_TX_SUFFIX _C
		#include "tserial_tx.inc"

	#endif

#endif

#if tsRX & tsconfigTIMER3

	#ifdef tsRX_PINS
		#define tsconfigRX_PINS	tsRX_PINS
	#endif

#if defined(tsRX_PINS) && (((tsRX_PINS-1) & tsRX_PINS) > 0)

	#if !(tsRX & tsconfigASCII)
		#error "Multiple PCINT pins for RX only supported in ASCII mode"
	#endif

	#define tsconfigRX tsRX

	#if tsconfigRX & tsconfigPCPORTA
		#define tsFUNC_PREFIX_BASE tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,PA)
	#elif tsconfigRX & tsconfigPCPORTB
		#define tsFUNC_PREFIX_BASE tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,PB)
	#elif tsconfigRX & tsconfigPCPORTC
		#define tsFUNC_PREFIX_BASE tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,PC)
	#elif tsconfigRX & tsconfigPCPORTD
		#define tsFUNC_PREFIX_BASE tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,PD)
	#else
		#error "Invalid port defined for RX"
	#endif

	#include "tserial_rx_ir.inc"

	#if tsRX_PINS & tsconfigPCBIT0
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_PREFIX_BASE,0Ascii)
		#define tsconfigPCBIT 0
		#include "tserial_rx_funcs.inc"
	#endif

	#if tsRX_PINS & tsconfigPCBIT1
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_PREFIX_BASE,1Ascii)
		#define tsconfigPCBIT 1
		#include "tserial_rx_funcs.inc"
	#endif

	#if tsRX_PINS & tsconfigPCBIT2
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_PREFIX_BASE,2Ascii)
		#define tsconfigPCBIT 2
		#include "tserial_rx_funcs.inc"
	#endif

	#if tsRX_PINS & tsconfigPCBIT3
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_PREFIX_BASE,3Ascii)
		#define tsconfigPCBIT 3
		#include "tserial_rx_funcs.inc"
	#endif

	#if tsRX_PINS & tsconfigPCBIT4
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_PREFIX_BASE,4Ascii)
		#define tsconfigPCBIT 4
		#include "tserial_rx_funcs.inc"
	#endif

	#if tsRX_PINS & tsconfigPCBIT5
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_PREFIX_BASE,5Ascii)
		#define tsconfigPCBIT 5
		#include "tserial_rx_funcs.inc"
	#endif

	#if tsRX_PINS & tsconfigPCBIT6
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_PREFIX_BASE,6Ascii)
		#define tsconfigPCBIT 6
		#include "tserial_rx_funcs.inc"
	#endif

	#if tsRX_PINS & tsconfigPCBIT7
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_PREFIX_BASE,7Ascii)
		#define tsconfigPCBIT 7
		#include "tserial_rx_funcs.inc"
	#endif

#else // defined(tsRX_PINS) && (((tsRX_PINS-1) & tsRX_PINS) > 0)

	#if tsRX & tsconfigOCR_A

		#define tsconfigRX tsRX
		#if tsRX & tsconfigASCII
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,AAscii)
		#else
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,A)
		#endif
		#define tsREG_RX_SUFFIX _A
		#include "tserial_rx_ir.inc"
		#include "tserial_rx_funcs.inc"

	#endif

	#if tsRX & tsconfigOCR_B

		#define tsconfigRX tsRX
		#if tsRX & tsconfigASCII
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,BAscii)
		#else
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,B)
		#endif
		#define tsREG_RX_SUFFIX _B
		#include "tserial_rx_ir.inc"
		#include "tserial_rx_funcs.inc"

	#endif

	#if tsRX & tsconfigOCR_C

		#define tsconfigRX tsRX
		#if tsRX & tsconfigASCII
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,CAscii)
		#else
		#define tsFUNC_PREFIX tsMAKE_FUNCNAME(tsFUNC_TIMER_PREFIX,C)
		#endif
		#define tsREG_RX_SUFFIX _C
		#include "tserial_rx_ir.inc"
		#include "tserial_rx_funcs.inc"

	#endif

#endif // defined(tsRX_PINS) && (((tsRX_PINS-1) & tsRX_PINS) > 0)

#endif

#if (tsTX & tsconfigTIMER3) || (tsRX & tsconfigTIMER3)

	.section tsBSS_SECTIONNAME
	tsBSS_END = .
	.global tsBSS_END

#endif


#undef tsTX
#undef tsRX
#undef tsRX_PINS
#undef tsconfigTX
#undef tsconfigRX
#undef tsconfigRX_PINS
#undef tsTXDDRA
#undef tsTXPORTA
#undef tsTXPINA
#undef tsBITS
#undef tsDS
#undef tsTCCRB
#undef tsTIMSK
#undef tsNO_PRESCALE
#undef tsPRESCALE
#undef tsPRESCALE_8
#undef tsPRESCALE_32
#undef tsPRESCALE_64
#undef tsPRESCALE_128
#undef tsPRESCALE_256
#undef tsPRESCALE_1024
#undef tsBSS_START
#undef tsBSS_END
#undef tsBSS_SECTIONNAME
#undef tsDATA_SECTIONNAME
#undef tsTEXT_SECTIONNAME
#undef tsVAR_TIMER_PREFIX
#undef tsTXTCNT
#undef tsTXTCCRA
#undef tsTXFOCR
#undef tsVAR_RX_PREFIX
#undef tsRXDDR
#undef tsRXPORT
#undef tsRXINPUT
#undef tsFUNC_TIMER_PREFIX
#undef tsTXTIFR
#undef tsTX_IR_A
#undef tsTXOCR_A
#undef tsTXOCF_A
#undef tsTXCOM0_A
#undef tsTXCOM1_A
#undef tsTXFOC_A
#undef tsTXOCIE_A
#undef tsVAR_TX_PREFIX_A
#undef tsRXTIFR
#undef tsRX_IR
#undef tsRXTCNT
#undef MAX_TICKS_PER_BIT
#undef tsRXPCMSK
#undef tsRXINT
#undef tsRXINTF
#undef tsFUNC_PREFIX_BASE
#undef tsVAR_RX_PCPORT
#undef rx_state
#undef rx_bit
#undef rx_byte
#undef rx_target
#undef rx_buffer_head
#undef rx_buffer_tail
#undef rx_buffer_overflow
#undef rx_buffer
#undef tsVAR_RX_SUFFIX
#undef tsRX_SB_IR
#undef tsRXOCR
#undef tsRXOCF
#undef tsRXOCIE
#undef tsRXEIMSK
#undef tsRXEIFR
#undef tsVAR_RX_PREFIX_B
#undef tsRXEICRA
#undef tsREG_INT_SUFFIX
#undef tsREG_RX_SUFFIX
#undef tsBAUDRATE
#undef TX_BUFFER_SIZE
#undef RX_BUFFER_SIZE