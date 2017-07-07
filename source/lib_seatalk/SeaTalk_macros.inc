#ifndef SEATALK_MACROS_INC_
#define SEATALK_MACROS_INC_

#include "../common/macros.inc"

//////////////////////////////////////////////////////////////////////////////
// Macro to build section names
//
#define stsEVALUATE(PREFIX,SUFFIX)			PREFIX ## SUFFIX
#define stsMAKE_SECTIONNAME(PREFIX,SUFFIX)	stsEVALUATE(PREFIX,SUFFIX)

//////////////////////////////////////////////////////////////////////////////
// Macro to build variable names
//
#define stsMAKE_VARNAME(PREFIX,SUFFIX)		stsEVALUATE(PREFIX,SUFFIX)

//////////////////////////////////////////////////////////////////////////////
// Macro to build function names
//
#define stsMAKE_FUNCNAME(PREFIX,SUFFIX)		stsEVALUATE(PREFIX,SUFFIX)

//////////////////////////////////////////////////////////////////////////////
// Macro to build register names
//
#define stsMAKE_REGNAME(PREFIX,SUFFIX)		stsEVALUATE(PREFIX,SUFFIX)

#endif /* SEATALK_MACROS_INC_ */
