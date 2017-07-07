/*-------------------------------------------------------------------------------------------------
 *  Extended itoa, puts and printf                    (C)ChaN, 2011
 *
 *	doharmon 2015
 *	Added xqtoa, fixed point functionality, and xfunc_out as passed parameter
 *-------------------------------------------------------------------------------------------------
 */

#ifndef _XIO_H
#define _XIO_H

#include "xio-config.h"

/*-------------------------------------------------------------------------------------------------

	GCC Fixed Point formats supported by lib_xio

	K : Fixed point accumulator _Accum
		Size	FP		Bytes
		H		s 8.7	  2
		default	s16.15	  4
		L		s32.31	  8
		LL		s16.47	  8
		UH		  8.8	  2
		U		 16.16	  4
		UL		 32.32	  8
		ULL		 16.48	  8
	R : Fixed point fraction _Fract
		Size	FP		Bytes
		H		s  .7	  1
		default	s  .15	  2
		L		s  .31	  4
		LL		s  .63	  8
		UH		   .8	  1
		U		   .16	  2
		UL		   .32	  4
		ULL		   .64	  8
 *-------------------------------------------------------------------------------------------------
 */

// Default fractional bits for _Accum and _Fract
#define xioFRACBITS_HK					7
#define xioFRACBITS_K					15
#define xioFRACBITS_LK					31
#define xioFRACBITS_LLK					47
#define xioFRACBITS_UHK					8
#define xioFRACBITS_UK					16
#define xioFRACBITS_ULK					32
#define xioFRACBITS_ULLK				48
#define xioFRACBITS_HR					7
#define xioFRACBITS_R					15
#define xioFRACBITS_LR					31
#define xioFRACBITS_LLR					63
#define xioFRACBITS_UHR					8
#define xioFRACBITS_UR					16
#define xioFRACBITS_ULR					32
#define xioFRACBITS_ULLR				64

// Default precision when not given in conversion specification
#define xioPRECISION_HK					2
#define xioPRECISION_K					4
#define xioPRECISION_LK					8
#define xioPRECISION_HR					2
#define xioPRECISION_R					4
#define xioPRECISION_LR					8

#define xioFLAG_LEFT_JUSTIFIED			0x01	// Left justify print out
#define xioFLAG_LEADING_PLUS			0x02	// Positive numbers have leading '+'
#define xioFLAG_LEADING_SPACE			0x04	// Positive numbers have leading ' '
#define xioFLAG_UNSIGNED				0x08	// Unsigned fixed point value
#define xioFLAG_NO_ROUNDING				0x10	// Do not round value before print out
#define xioFLAG_ZERO_FILL				0x20	// Left-pad with '0'
#define xioFLAG_SHORT					0x40	// H size
#define xioFLAG_LONG					0x80	// L size

#define xioFLAG_LEFT_JUSTIFIED_BIT		0x00	// Left justify print out
#define xioFLAG_LEADING_PLUS_BIT		0x01	// Positive numbers have leading '+'
#define xioFLAG_LEADING_SPACE_BIT		0x02	// Positive numbers have leading ' '
#define xioFLAG_UNSIGNED_BIT			0x03	// Unsigned fixed point value
#define xioFLAG_NO_ROUNDING_BIT			0x04	// Do not round value before print out
#define xioFLAG_ZERO_FILL_BIT			0x05	// Left-pad with '0'
#define xioFLAG_SHORT_BIT				0x06	// H size
#define xioFLAG_LONG_BIT				0x07	// L size

// Disable rest of file if including into an assembler file
#ifndef xioASSEMBLER

#include <stdarg.h>
#include <inttypes.h>
#include <avr/pgmspace.h>

/*-------------------------------------------------------------------------------------------------
 *	This is a pointer to user defined output function. It must be initialized
 *	before using xprintf_P.
 *-------------------------------------------------------------------------------------------------
 */
extern void (*xfunc_out)(uint8_t);
#define xdev_out(func) xfunc_out = (void(*)(uint8_t))(func)

#if COUNT_CHARS
typedef int ret_t;
#else
typedef void ret_t;
#endif

/*-------------------------------------------------------------------------------------------------
 *	This is a stub function to forward output to user defined output function.
 *-------------------------------------------------------------------------------------------------
 */
void xputc(void (*xfunc_out)(uint8_t), char chr);



/*-------------------------------------------------------------------------------------------------
 *	The string stored in ROM is forwarded to xfunc_out() directly.
 *-------------------------------------------------------------------------------------------------
 */
ret_t xputs_P(void (*xfunc_out)(uint8_t), const char *string);



/*-------------------------------------------------------------------------------------------------
 *	The string stored in RAM is forwarded to xfunc_out() directly.
 *-------------------------------------------------------------------------------------------------
 */
ret_t xputs(void (*xfunc_out)(uint8_t), const char *string);



/*-------------------------------------------------------------------------------------------------
	Extended itoa().

			value  radix  width   output
			  100     10      6   "   100"
			  100     10     -6   "000100"
			  100     10      0   "100"
	   4294967295     10      0   "4294967295"
	   4294967295    -10      0   "-1"
		   655360     16     -8   "000A0000"
			 1024     16      0   "400"
			 0x55      2     -8   "01010101"
 *-------------------------------------------------------------------------------------------------
 */
ret_t xitoa(void (*xfunc_out)(uint8_t), long value, char radix, char width);



/*-------------------------------------------------------------------------------------------------
	Extended 32 bit fixed point out

			fp  fbits  flags  width   prec  output
		0x0CCD     15      0      6      2  "  0.10"
		0x199A     16   0x08      6      2  "  0.10"
		0x011A      8      0      6      2  "  1.10"
		0x011A      8   0x08      6      2  "  1.10"
		0xF333      8      0      6      2  " -0.10"
		0xFEE6      8      0      6      2  " -1.10"
	0x7DF126E9     20      0     10      4  " 2015.0720"
 *-------------------------------------------------------------------------------------------------
 */
ret_t xqtoa (void (*xfunc_out)(uint8_t),	// output function
			 long fp,						// fixed point value to be output
			 char fbits,					// number of bits in fraction
			 char width,					// minimum width to print value, max: 127
			 char flags,					// format flags
			 char precision);				// number of digits after decimal



/*-------------------------------------------------------------------------------------------------
	Extended 64 bit fixed point out

			fp  fbits  flags  width   prec  output
		0x0CCD     15      0      6      2  "  0.10"
		0x199A     16   0x08      6      2  "  0.10"
		0x011A      8      0      6      2  "  1.10"
		0x011A      8   0x08      6      2  "  1.10"
		0xF333      8      0      6      2  " -0.10"
		0xFEE6      8      0      6      2  " -1.10"
	0x7DF126E9     20      0     10      4  " 2015.0720"
 *-------------------------------------------------------------------------------------------------
 */
ret_t xlqtoa (void (*xfunc_out)(uint8_t),	// output function
			  long long fp,					// fixed point value to be output
			  char fbits,					// number of bits in fraction
			  char width,					// minimum width to print value, max: 127
			  char flags,					// format flags
			  char precision);				// number of digits after decimal



/*-------------------------------------------------------------------------------------------------
	Format string is stored in ROM. The format flags are similar to printf().

	%[flags][width][.precision][size]type

	flags
		0	: Left-pad with '0' when output is shorter than width.
			  Space is used by default.
		The following are valid for fixed point types only
		-	: Left justify output. Right justification is default.
		+	: Output a plus sign before positive numbers. 
			  Only '-' before negative numbers by default.
		' '	: Output a space before positive numbers if no sign is to 
			  be inserted.
	width
		Minimum width in decimal number. Maximum is 127. 
		This is effective only for numeric types. Default width is zero.
	.precision
		For fixed point only. Specifies number of digits after decimal point.
		Default is configurable. Maximum is 99.
	size
		For numeric types only.
		h : short		k/K/q/Q 16 bit, all others 8 bit
		Default			k/K/q/Q 32 bit, all others 16 bit
		l : long		k/K/q/Q 64 bit, all others 32 bit
	type
		% : '%'
		c : Character, argument is the value
		C : ASCII characters [0x09-0x0D][0x20-0x7E] ignore all others
		s : String stored in RAM, argument is the pointer
		S : String stored in ROM, argument is the pointer
		d : Signed decimal, argument is the value
		u : Unsigned decimal, argument is the value
		X : Hexadecimal, argument is the value
		b : Binary, argument is the value
		k : Fixed point accumulator _Accum
		K : Fixed point unsigned accumulator _Accum
			Size	FP		Bytes
			------- ------  -----
			h		s 8.7	  2
			default	s16.15	  4
			l		s32.31	  8
			LL		s16.47	  8		Need to use: "%lq", 47, llk
			Uh		  8.8	  2
			U		 16.16	  4
			Ul		 32.32	  8
			ULL		 16.48	  8		Need to use: "%lQ", 48, ullk
		r : Fixed point fraction _Fract
		R : Fixed point unsigned fraction _Fract
			Size	FP		Bytes
			------- ------  -----
			h		s  .7	  1
			default	s  .15	  2
			l		s  .31	  4
			LL		s  .63	  8		Need to use: "%lq", 63, llr
			Uh		   .8	  1
			U		   .16	  2
			Ul		   .32	  4
			ULL		   .64	  8		Need to use: "%lQ", 64, ullr
		q : Fixed point using passed fractional bits size
		Q : Fixed point unsigned using passed fractional bits size
				first argument is number of fractional bits (data type 16 bit)
				second argument is the value

	Returns
		If COUNT_CHARS > 0: Number of characters output. If snprintf function, number of characters
		that would be output regardless of the size of the output buffer.
		If COUNT_CHARS = 0: Return type is void.

 *-------------------------------------------------------------------------------------------------
 */
ret_t xprintf_P(const char* format, ...);							/* Send formatted string to the registered device */
ret_t xvprintf_P(const char* format, va_list);						/* Send formatted string to the registered device. Called when passed va_list */
ret_t xvsprintf_P(char*, const char* format, ...);					/* Put formatted string to memory. Called when passed va_list */
ret_t xsprintf_P(char*, const char* format, ...);					/* Put formatted string to memory */
ret_t xfprintf_P(void(*func)(uint8_t), const char* format, ...);	/* Send formatted string to the specified device */
ret_t xvsnprintf_P(char*, int size, const char *format, ...);		/* Put formatted string to memory up to size bytes. Called when passed va_list */
ret_t xsnprintf_P(char*, int size, const char *format, ...);		/* Put formatted string to memory up to size bytes. */

/*-------------------------------------------------------------------------------------------------
 	Format string is stored in ROM. The format flags are similar to scanf().

	const char* 		buff		String in RAM to be parsed
	uint8_t     		buff_sz		Length of buff
	const __flash char*	format		Conversion string in ROM

	%[flags][width]type

	flags
		* : Do not store the converted value
	width
		Maximum characters to parse. Default width is zero.
	type
		c : Character
		s : Zero terminated string
		d : Signed decimal, 16 bits
		i : Signed decimal, 16 bits, same as d
		u : Unsigned decimal, 16 bits
		k : Fixed point accumulator _Accum, s16.15, 32 bits
		r : Fixed point fraction _Fract, s.15, 16 bits
			
	Returns the number of conversion specifications processed
	(not including '*' flagged specifications)
 *-------------------------------------------------------------------------------------------------
 */
uint8_t xsscanf_P(const char* buff, const __flash char* format, ...);

/*-------------------------------------------------------------------------------------------------
	Get value of the numeral string. 

    "0b11001010"	binary
    "0377"			octal
    "0xff800"		hexadecimal
    "1250000"		decimal
    "-25000"		decimal

	Returns
		1: Successful
		0: Failed
 *-------------------------------------------------------------------------------------------------
 */
char xatoi(char **str, long *ret);

/*-------------------------------------------------------------------------------------------------
	Get value of the fixed point string. 

	Returns
		1: Successful
		0: Failed
 *-------------------------------------------------------------------------------------------------
 */
char xatoq (char**				str,		// pointer to pointer to source string
			long*				fp,			// pointer to fixed point result
			char				fbits,		// number of bits in fraction
			char				unsign);	// non zero if fp is unsigned

char xatok (char**				str,		// pointer to pointer to source string
			_Accum*				fp,			// pointer to fixed point result
			char				fbits,		// number of bits in fraction
			char				unsign);	// non zero if fp is unsigned

char xatouk (char**				str,		// pointer to pointer to source string
			unsigned _Accum*	fp,			// pointer to fixed point result
			char				fbits,		// number of bits in fraction
			char				unsign);	// non zero if fp is unsigned

char xator (char**				str,		// pointer to pointer to source string
			_Fract*				fp,			// pointer to fixed point result
			char				fbits,		// number of bits in fraction
			char				unsign);	// non zero if fp is unsigned

#endif /* xioASSEMBLER */


#endif	/* _XIO_H */
