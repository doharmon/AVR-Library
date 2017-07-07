/*
---------------------------------------------------------------------------------------------------
	Configuration file for lib_xio
---------------------------------------------------------------------------------------------------
*/
#ifndef XIO_CONFIG_H_
#define XIO_CONFIG_H_

#define	LF_CRLF			0		// Convert \n to \r\n
#define OPTIMIZE_SPEED	0		// Optimize xqtoa and xlqtoa routines for speed
#define COUNT_CHARS		0		// Functions return count of chars output
#define USE_UPPERCASEC	0		// The 'C' format type outputs ASCII only [0x09-0x0D][0x20-0x7E]
#define USE_XPRINTF_P	1		// Enable xprintf_P and xvprintf_P functions
#define USE_XSPRINTF_P	0		// Add xsprintf_P and xvsprintf_P functions
#define USE_XSNPRINTF_P	1		// Add xsnprintf_P and xvsnprintf_P functions
#define USE_XFPRINTF_P	1		// Add xfprintf_P function
#define USE_XQTOA		1		// Enable xqtoa function
#define USE_XLQTOA		0		// Enable xlqtoa function
#define USE_XATOI		1		// Enable xatoi function
#define USE_XATOQ		1		// Enable xatoq function
#define USE_XATOLQ		0		// Enable xatolq function NOT IMPLEMENTED

/*
---------------------------------------------------------------------------------------------------
Note:	xlqtoa does not support returning count of chars output (COUNT_CHARS).

Note:	xqtoa must be enabled in order to have 32 bit fixed point
		functionality in the xprintf functions.

Note:	xqtoa and xlqtoa must both be enabled in order to have 64 bit
		fixed point functionality in the xprintf functions.
---------------------------------------------------------------------------------------------------
*/

#endif // XIO_CONFIG_H_