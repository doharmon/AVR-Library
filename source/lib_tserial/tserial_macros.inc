#ifndef TSERIAL_MACROS_INC_
#define TSERIAL_MACROS_INC_

#include "../common/macros.inc"

//////////////////////////////////////////////////////////////////////////////
// Macro to build section names
//
#define tsEVALUATE(PREFIX,SUFFIX)			PREFIX ## SUFFIX
#define tsMAKE_SECTIONNAME(PREFIX,SUFFIX)	tsEVALUATE(PREFIX,SUFFIX)

//////////////////////////////////////////////////////////////////////////////
// Macro to build variable names
//
#define tsMAKE_VARNAME(PREFIX,SUFFIX)		tsEVALUATE(PREFIX,SUFFIX)

//////////////////////////////////////////////////////////////////////////////
// Macro to build function names
//
#define tsMAKE_FUNCNAME(PREFIX,SUFFIX)		tsEVALUATE(PREFIX,SUFFIX)

//////////////////////////////////////////////////////////////////////////////
// Macro to build register names
//
#define tsMAKE_REGNAME(PREFIX,SUFFIX)		tsEVALUATE(PREFIX,SUFFIX)

#endif /* TSERIAL_MACROS_INC_ */
