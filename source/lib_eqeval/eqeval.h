/*-------------------------------------------------------------------------------------------------
 * eqeval.h - include file for equation evaluator lib
 *
 * Copyright (c) 2017 David Harmon
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *-------------------------------------------------------------------------------------------------
 */

#ifndef EQEVAL_H_
#define EQEVAL_H_

#include "eqeval-config.h"

#ifdef __TINYC__
#include <stddef.h>
#else
#include <avr/io.h>
#endif

/*-------------------------------------------------------------------------------------------------
 * Symbol types
 *-------------------------------------------------------------------------------------------------
 */
#define EQEV_T_PRIORITY				0x04
#define EQEV_T_VAR					0x08							// Up to 8 variables
#define EQEV_T_CONST				0x10							// Up to 16 constants
#define EQEV_T_INT					0x20							// Integer values 0 to 31
#define EQEV_T_UNARY				0x40							// Up to 64 unary op codes
#define EQEV_T_BINARY				0x80							// Up to 64 binary op codes

/*-------------------------------------------------------------------------------------------------
 * Virtual Machine Op Codes
 *-------------------------------------------------------------------------------------------------
 */
#define EQEV_TOK_ADD				(EQEV_T_BINARY   + 1)
#define EQEV_TOK_SUB				(EQEV_T_BINARY   + 2)
#define EQEV_TOK_MUL				(EQEV_T_BINARY   + 3)
#define EQEV_TOK_DIV				(EQEV_T_BINARY   + 4)
#define EQEV_TOK_POW				(EQEV_T_BINARY   + 5)
#define EQEV_TOK_SQRT				(EQEV_T_UNARY    + 1)
#define EQEV_TOK_SIN				(EQEV_T_UNARY    + 2)
#define EQEV_TOK_COS				(EQEV_T_UNARY    + 3)
#define EQEV_TOK_TAN				(EQEV_T_UNARY    + 4)
#define EQEV_TOK_ATN				(EQEV_T_UNARY    + 5)
#define EQEV_TOK_LN					(EQEV_T_UNARY    + 6)
#define EQEV_TOK_NEG				(EQEV_T_UNARY    + 7)
#define EQEV_TOK_ABS				(EQEV_T_UNARY    + 8)
#define EQEV_TOK_EXP				(EQEV_T_UNARY    + 9)
#define EQEV_TOK_OP					(EQEV_T_PRIORITY + 1)			// Open parenthesis
#define EQEV_TOK_CP					(EQEV_T_PRIORITY + 2)			// Close parenthesis
#define EQEV_TOK_PI					(EQEV_T_CONST    + 1)
#define EQEV_TOK_E					(EQEV_T_CONST    + 2)
#define EQEV_TOK_FP					(EQEV_T_CONST    + 3)
#define EQEV_TOK_X					(EQEV_T_VAR      + 1)
#define EQEV_TOK_END				255

#define EQEV_TOK_MASK_PRIORITY		0xFC
#define EQEV_TOK_MASK_VAR			0xF8
#define EQEV_TOK_MASK_CONST			0xF0
#define EQEV_TOK_MASK_INT			0xE0
#define EQEV_TOK_MASK_UNARY			0xC0
#define EQEV_TOK_MASK_BINARY		0x80

/*-------------------------------------------------------------------------------------------------
 * Return Codes
 *-------------------------------------------------------------------------------------------------
 */
#define EQEV_E_OK					0								// Success
#define EQEV_E_INVARG				1								// Insufficient args for opcode
#define EQEV_E_TOOCOMPLEX			2								// Code/Temp stack too small
#define EQEV_E_FPTOOSMALL			3								// FP stack too small
#define EQEV_E_PARENTOODEEP			4								// Parenthesis level too deep
#define EQEV_E_UNBALPAREN			5								// Open/Close parenthesis mismatch
#define EQEV_E_BADOPCODE			6								// Internal error, bad opcode
#define EQEV_E_OPTIMIZERSP			7								// Stack for optimizer too small
#define EQEV_E_BADDEPTH				8								// Parser error, fp depth not 1
#define EQEV_E_BADTOKEN				254								// Invalid token
#define EQEV_E_END					255								// End of parse string

/*-------------------------------------------------------------------------------------------------
 * Structure for equation symbols
 *-------------------------------------------------------------------------------------------------
 */
typedef struct eqevsymbol
{
	const __flash char*	psSymbol;									// Symbol
	uint8_t				cbSymbol;									// Count of chars in symbol
	uint8_t				bType;										// Symbol type
	uint8_t				bPriority;									// Evaluation priority
	uint8_t				bToken;										// Op code
} eqevSymbol_t;

/*-------------------------------------------------------------------------------------------------
 * Structure for temporary stack
 *-------------------------------------------------------------------------------------------------
 */
typedef struct eqevtemp
{
	uint8_t				bPriority;									// Evaluation priority
	uint8_t				bToken;										// Op code
} eqevTemp_t;

/*-------------------------------------------------------------------------------------------------
 * Union to help store fp onto code stack
 *-------------------------------------------------------------------------------------------------
 */
typedef union eqevconvert
{
	_Accum	k;														// Fixed point value
	uint8_t	b[4];													// Byte array
} eqevConvert_t;

const __flash char* eqeval_get_err_msg(uint8_t isz);
uint8_t eqeval_parse(char* psz);
uint8_t eqeval_optimize(void);
_Accum eqeval_eval(_Accum x);

#endif // EQEVAL_H_
