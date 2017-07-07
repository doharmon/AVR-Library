/*! @file
 *
 * Timer.h
 *
 * Created: 11/15/2014 11:11:22 AM
 *  Author: doharmon
 */ 


#ifndef Timer_H_
#define Timer_H_

#include <stdlib.h>		// abs

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! \addtogroup Timer_API
//! @{
//
//*****************************************************************************

//! Wait at least timeout_ms milliseconds until test is false.
#define WAIT_TILL_FALSE(test,timeout_ms) {uint32_t __m__ = millis(); while (test) if (abs(millis()-__m__) > timeout_ms) break;}

//! Wait at least timeout_ms milliseconds until test is false. Return if timeout exceeded
#define WAIT_TILL_FALSE_RETURN(test,timeout_ms) {uint32_t __m__ = millis(); while (test) if (abs(millis()-__m__) > timeout_ms) return;}

//! Wait at least timeout_ms milliseconds until test is false. Return rvalue if timeout exceeded
#define WAIT_TILL_FALSE_RETURN_VALUE(test,timeout_ms,rvalue) {uint32_t __m__ = millis(); while (test) if (abs(millis()-__m__) > timeout_ms) return rvalue;}

void          initTimer(void);
unsigned long millis(void);
unsigned long micros(void);
void          delay(unsigned long ms);

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

#ifdef __cplusplus
}
#endif

#endif /* Timer_H_ */