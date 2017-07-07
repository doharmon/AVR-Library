/*-------------------------------------------------------------------------------------------------
 * eqeval.c - equation evaluator lib
 *
 * Copyright (c) 2017 David Harmon
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *-------------------------------------------------------------------------------------------------
 */

#include "eqeval.h"

#ifdef __TINYC__
#include <stdlib.h>
#include <math.h>
#else
#include "../lib_gccfix/gccfix.h"
#include "../lib_xio/xio.h"
#include <string.h>
#include <ctype.h>
#endif

/*-------------------------------------------------------------------------------------------------
 * Global variables
 *-------------------------------------------------------------------------------------------------
 */
_Accum 		g_k;											// General use fixed point
uint8_t 	g_bPriority;									// Nested priority level
uint8_t 	g_bDepth;										// Depth of fp stack
uint8_t		g_tokPrev;										// Previous parsed token
uint8_t		g_atokCode[EQEV_SIZE_CODESTACK];				// VM code stack
uint8_t		g_itokCode;										// Index into VM code stack
eqevTemp_t	g_atemp[EQEV_SIZE_CODESTACK];					// Temporary holding stack
uint8_t		g_itemp;										// Index into temporary holding stack


/*-------------------------------------------------------------------------------------------------
 * Error messages
 *-------------------------------------------------------------------------------------------------
 */
const __flash char * const __flash g_aszErrMsg[] =
{
  (const __flash char[]) { EQEV_E_STR_INVARG		},
  (const __flash char[]) { EQEV_E_STR_TOOCOMPLEX	},
  (const __flash char[]) { EQEV_E_STR_FPTOOSMALL	},
  (const __flash char[]) { EQEV_E_STR_PARENTOODEEP	},
  (const __flash char[]) { EQEV_E_STR_UNBALPAREN	},
  (const __flash char[]) { EQEV_E_STR_BADOPCODE		},
  (const __flash char[]) { EQEV_E_STR_OPTIMIZERSP	},
  (const __flash char[]) { EQEV_E_STR_BADDEPTH		},
  (const __flash char[]) { EQEV_E_STR_BADTOKEN		}
};

/*-------------------------------------------------------------------------------------------------
 * Symbol Table
 *-------------------------------------------------------------------------------------------------
 */
const __flash char EQEV_SYM_ADD[]				= "+";
const __flash char EQEV_SYM_SUB[]				= "-";
const __flash char EQEV_SYM_MUL[]				= "*";
const __flash char EQEV_SYM_DIV[]				= "/";
const __flash char EQEV_SYM_POW[]				= "^";
const __flash char EQEV_SYM_OP[]				= "(";
const __flash char EQEV_SYM_CP[]				= ")";
const __flash char EQEV_SYM_SQRT[]				= "SQRT";
const __flash char EQEV_SYM_SIN[]				= "SIN";
const __flash char EQEV_SYM_COS[]				= "COS";
const __flash char EQEV_SYM_TAN[]				= "TAN";
const __flash char EQEV_SYM_ATN[]				= "ATAN";
const __flash char EQEV_SYM_LN[]				= "LN";
const __flash char EQEV_SYM_NEG[]				= "NEG";
const __flash char EQEV_SYM_ABS[]				= "ABS";
const __flash char EQEV_SYM_EXP[]				= "EXP";
const __flash char EQEV_SYM_PI[]				= "PI";
const __flash char EQEV_SYM_E[]					= "E";
const __flash char EQEV_SYM_X[]					= "X";

const __flash eqevSymbol_t g_aSymbol[] =
//		psSymbol		cbSymbol				bType				bPriority	bToken
{
	{	EQEV_SYM_ADD,	sizeof(EQEV_SYM_ADD)-1,	EQEV_T_BINARY,		0,			EQEV_TOK_ADD	},
	{	EQEV_SYM_SUB,	sizeof(EQEV_SYM_SUB)-1,	EQEV_T_BINARY,		0,			EQEV_TOK_SUB	},
	{	EQEV_SYM_MUL,	sizeof(EQEV_SYM_MUL)-1,	EQEV_T_BINARY,		1,			EQEV_TOK_MUL	},
	{	EQEV_SYM_DIV,	sizeof(EQEV_SYM_DIV)-1,	EQEV_T_BINARY,		1,			EQEV_TOK_DIV	},
	{	EQEV_SYM_POW,	sizeof(EQEV_SYM_POW)-1,	EQEV_T_BINARY,		3,			EQEV_TOK_POW	},
	{	EQEV_SYM_OP,	sizeof(EQEV_SYM_OP)-1,	EQEV_T_PRIORITY,	2,			EQEV_TOK_OP		},
	{	EQEV_SYM_CP,	sizeof(EQEV_SYM_CP)-1,	EQEV_T_PRIORITY,	2,			EQEV_TOK_CP		},
	{	EQEV_SYM_NEG,	sizeof(EQEV_SYM_NEG)-1,	EQEV_T_UNARY,		2,			EQEV_TOK_NEG	},	// Update EQEV_SYMTAB_NEG if order changes
	{	EQEV_SYM_SQRT,	sizeof(EQEV_SYM_SQRT)-1,EQEV_T_UNARY,		2,			EQEV_TOK_SQRT	},
	{	EQEV_SYM_SIN,	sizeof(EQEV_SYM_SIN)-1,	EQEV_T_UNARY,		2,			EQEV_TOK_SIN	},
	{	EQEV_SYM_COS,	sizeof(EQEV_SYM_COS)-1,	EQEV_T_UNARY,		2,			EQEV_TOK_COS	},
	{	EQEV_SYM_TAN,	sizeof(EQEV_SYM_TAN)-1,	EQEV_T_UNARY,		2,			EQEV_TOK_TAN	},
	{	EQEV_SYM_ATN,	sizeof(EQEV_SYM_ATN)-1,	EQEV_T_UNARY,		2,			EQEV_TOK_ATN	},
	{	EQEV_SYM_LN,	sizeof(EQEV_SYM_LN)-1,	EQEV_T_UNARY,		2,			EQEV_TOK_LN		},
	{	EQEV_SYM_ABS,	sizeof(EQEV_SYM_ABS)-1,	EQEV_T_UNARY,		2,			EQEV_TOK_ABS	},
	{	EQEV_SYM_EXP,	sizeof(EQEV_SYM_EXP)-1,	EQEV_T_UNARY,		2,			EQEV_TOK_EXP	},
	{	EQEV_SYM_PI,	sizeof(EQEV_SYM_PI)-1,	EQEV_T_CONST,		2,			EQEV_TOK_PI		},
	{	EQEV_SYM_E,		sizeof(EQEV_SYM_E)-1,	EQEV_T_CONST,		2,			EQEV_TOK_E		},
	{	EQEV_SYM_X,		sizeof(EQEV_SYM_X)-1,	EQEV_T_VAR,			2,			EQEV_TOK_X		},
	{	0,				0,						EQEV_T_CONST,		2,			EQEV_TOK_FP		}
};

#define EQEV_MAX_PRIORITY	3
#define EQEV_SYMTAB_SIZE	(sizeof(g_aSymbol)/sizeof(eqevSymbol_t))-1
#define EQEV_SYMTAB_NEG		7
#define EQEV_SYMTAB_FP		EQEV_SYMTAB_SIZE

/*-------------------------------------------------------------------------------------------------
 * EQEVAL INTERN: Scan for next delimited string
 *
 *	On return:
 *		*psz points to start of string
 *
 *	Return value is length of delimited string. Zero for end of string.
 *-------------------------------------------------------------------------------------------------
 */
const __flash char achDelim[] = "+-*/^()";
 
static uint8_t eqeval_scan(char** psz)
{
	uint8_t	i, j;
	char* 	sz = *psz;

	while (' ' == *sz)										// Skip leadng spaces
		sz++;

	*psz = sz;
	for (i = 0; isprint(*sz); i++, sz++)					// Non printable character is a delim
	{
		if (' ' == *sz)										// Space is a delim
		{
			return i;
		}
		for (j = 0; j < sizeof(achDelim)-1; j++)
			if (achDelim[j] == *sz)
			{
				if (0 == i)									// Start of string is a delim char
					i++;									// A delim char can also be a token

				return i;
			}
	}
	
	return i;
}

/*-------------------------------------------------------------------------------------------------
 * EQEVAL INTERN: Get next token
 *
 *	On return:
 *		*psz points to char after string containing token
 *		g_k equals value if token is a fixed point number
 *
 *	Return value:
 *		EQEV_E_END		End of string
 *		EQEV_E_BADTOKEN	Invalid token
 *		If not the above, then index into the g_aSymbol table.
 *-------------------------------------------------------------------------------------------------
 */
static uint8_t eqeval_get_token(char** psz)
{
	uint8_t i, j;
	
	i = eqeval_scan(psz);

	if (0 == i)
	{
		return EQEV_E_END;									// End of string
	}
	
	for (j = 0; j < EQEV_SYMTAB_SIZE; j++)					// Look up token in symbol table
	{
		if (i == g_aSymbol[j].cbSymbol && strncmp_P(*psz, g_aSymbol[j].psSymbol, i) == 0)
		{
			*psz += i;
			return j;
		}
	}

	for (j = 0; j < i; j++)									// Check if fixed point number
		if (!isdigit((*psz)[j]) && '.' != (*psz)[j])
			break;

	if (j == i)
	{
		#ifdef __TINYC__
		g_k = atof(*psz);
		*psz += i;
		#else
		xatok(psz, &g_k, xioFRACBITS_K, 0);
		#endif
		return EQEV_SYMTAB_FP;
	}

	return EQEV_E_BADTOKEN;
}

/*-------------------------------------------------------------------------------------------------
 * EQEVAL INTERN: Check if fp stack depth is valid for unary or binary token
 *
 *	Return value:
 *		EQEV_E_OK		Enough fp on stack for operation
 *		EQEV_E_INVARG	Not enough fp on stack. Also, first entry on code stack set to EQEV_TOK_END
 *-------------------------------------------------------------------------------------------------
 */
static uint8_t eqeval_check_depth(uint8_t bToken)
{
	if (((bToken & EQEV_T_BINARY) && g_bDepth < 2) ||
		((bToken & EQEV_T_UNARY)  && g_bDepth < 1))
	{
		g_atokCode[0] = EQEV_TOK_END;
//		xprintf_P(PSTR("\r\nError: " EQEV_E_STR_INVARG "\r\n"));
		return EQEV_E_INVARG;
	}
	if (bToken & EQEV_T_BINARY)
		g_bDepth--;
	
	return EQEV_E_OK;
}

/*-------------------------------------------------------------------------------------------------
 * EQEVAL INTERN: Push op code onto code stack
 *
 *	Return value:
 *		EQEV_E_OK			Op code place onto stack
 *		EQEV_E_TOOCOMPLEX	Code stack too small. First entry on code stack set to EQEV_TOK_END
 *-------------------------------------------------------------------------------------------------
 */
static uint8_t eqeval_push_op(uint8_t bOpCode)
{
	if (g_itokCode >= EQEV_SIZE_CODESTACK)
	{
		g_atokCode[0] = EQEV_TOK_END;
//		xprintf_P(PSTR("\r\nError: " EQEV_E_STR_TOOCOMPLEX "\r\n"));
		return EQEV_E_TOOCOMPLEX;
	}

	g_atokCode[g_itokCode++] = bOpCode;
	
	return EQEV_E_OK;
}
	
/*-------------------------------------------------------------------------------------------------
 * EQEVAL INTERN: Check depth of temp stack
 *
 *	Return value:
 *		EQEV_E_OK			Temp stack has not overflowed
 *		EQEV_E_TOOCOMPLEX	Temp stack too small. First entry on code stack set to EQEV_TOK_END
 *-------------------------------------------------------------------------------------------------
 */
static uint8_t eqeval_check_temp_sp(void)
{
	if (g_itemp >= EQEV_SIZE_CODESTACK)
	{
		g_atokCode[0] = EQEV_TOK_END;
//		xprintf_P(PSTR("\r\nError: " EQEV_E_STR_TOOCOMPLEX "\r\n"));
		return EQEV_E_TOOCOMPLEX;
	}
	
	return EQEV_E_OK;
}

/*-------------------------------------------------------------------------------------------------
 * EQEVAL INTERN: Check depth of fp stack
 *
 *	Return value:
 *		EQEV_E_OK			Fp stack has not overflowed
 *		EQEV_E_FPTOOSMALL	Fp stack too small. First entry on code stack set to EQEV_TOK_END
 *-------------------------------------------------------------------------------------------------
 */
static uint8_t eqeval_check_fp(void)
{
	if (g_bDepth >= EQEV_SIZE_FPSTACK)
	{
		g_atokCode[0] = EQEV_TOK_END;
//		xprintf_P(PSTR("\r\nError: " EQEV_E_STR_FPTOOSMALL "\r\n"));
		return EQEV_E_FPTOOSMALL;
	}

	return EQEV_E_OK;
}

/*-------------------------------------------------------------------------------------------------
 * EQEVAL INTERN: Check priorities of tokens on temp stack
 *
 *	On return, all higher priority op codes on temp stack are moved onto code stack
 *
 *	Return value:
 *		EQEV_E_OK			Success
 *		EQEV_E_INVARG		Not enough fp on stack. First entry on code stack set to EQEV_TOK_END
 *		EQEV_E_TOOCOMPLEX	Code stack too small. First entry on code stack set to EQEV_TOK_END
 *-------------------------------------------------------------------------------------------------
 */
static uint8_t eqeval_check_temp(uint8_t bPriority)
{
	uint8_t iTemp;
	uint8_t error;

	if (0 == g_itemp)
		return EQEV_E_OK;
	
	iTemp = g_itemp - 1;

	while (1)
	{
		if (g_atemp[iTemp].bPriority >= bPriority)
		{
			error = eqeval_check_depth(g_atemp[iTemp].bToken);
			if (error)
				return error;
			error = eqeval_push_op(g_atemp[iTemp].bToken);
			if (error)
				return error;
			g_itemp--;
		}
		else
			break;
		
		if (iTemp)
			iTemp--;
		else
			break;
	}

	return EQEV_E_OK;
}

/*-------------------------------------------------------------------------------------------------
 * EQEVAL INTERN: Verify stack pointers used by the optimizer do not overflow
 *
 *	Return value:
 *		EQEV_E_OK			Success
 *		EQEV_E_OPTIMIZERSP	Code or fp stack is too small
 *-------------------------------------------------------------------------------------------------
 */
static uint8_t eqeval_check_optimize_sps(uint8_t itok, uint8_t ifp)
{
	if (itok > EQEV_SIZE_CODESTACK-5 || ifp > EQEV_SIZE_FPSTACK)
	{
//		xprintf_P(PSTR("\r\nError: " EQEV_E_STR_OPTIMIZERSP "\r\n"));
		return EQEV_E_OPTIMIZERSP;
	}
	
	return EQEV_E_OK;
}

/*-------------------------------------------------------------------------------------------------
 * EQEVAL INTERN: Move all fp on stack to code stack
 *
 *	Return value:
 *		EQEV_E_OK			Success
 *		EQEV_E_OPTIMIZERSP	Code or fp stack is too small
 *-------------------------------------------------------------------------------------------------
 */
static uint8_t eqeval_move_fp_to_code(_Accum afp[], uint8_t* pifp, uint8_t atok[], uint8_t* pitok)
{
	uint8_t			i;
	uint8_t			j;
	uint8_t			error;
	eqevConvert_t	convert;

	i = 0;
	while (*pifp)
	{
		error = eqeval_check_optimize_sps(*pitok, *pifp);
		if (error)
			return error;

		--(*pifp);
		convert.k = afp[i++];								// Move from bottom to top of stack

		if (convert.k < EQEV_T_INT && convert.k >= 0 && (uint8_t)convert.k == convert.k)
		{
			for (j = 0; j < EQEV_T_INT; j++)
			{
				if (j == (uint8_t)convert.k)
				{
					atok[(*pitok)++] = EQEV_T_INT + j;		// Value is an integer between 0 and 31
					break;
				}
			}

			if (j < EQEV_T_INT)
				continue;
		}

		atok[(*pitok)++]	= EQEV_TOK_FP;
		atok[(*pitok)++] 	= convert.b[0];
		atok[(*pitok)++]	= convert.b[1];
		atok[(*pitok)++]	= convert.b[2];
		atok[(*pitok)++]	= convert.b[3];
	}
	
	return EQEV_E_OK;
}

/*-------------------------------------------------------------------------------------------------
 * EQEVAL: Optimize code. Currently just rolls constant values together.
 *
 *	Return value:
 *		EQEV_E_OK			Success
 *		EQEV_E_OPTIMIZERSP	Code or fp stack is too small
 *-------------------------------------------------------------------------------------------------
 */
uint8_t eqeval_optimize(void)
{
	uint8_t 		itokOld	= 0;
	uint8_t 		itokNew	= 0;
	uint8_t 		ifp		= 0;
	uint8_t 		tokTemp	= EQEV_TOK_END;
	uint8_t 		tok;
	uint8_t			atokNew[EQEV_SIZE_CODESTACK];
	uint8_t			error;
	_Accum			afp[EQEV_SIZE_FPSTACK];
	eqevConvert_t	convert;

	for (tok = g_atokCode[itokOld++]; tok != EQEV_TOK_END; tok = g_atokCode[itokOld++])
	{
		error = eqeval_check_optimize_sps(itokNew, ifp);
		if (error)
			return error;
															///////////////////////////////////////
		if (EQEV_T_BINARY & tok)							// Binary opcode
		{
			if (0 == ifp)
			{
				atokNew[itokNew++]	= tok;					// No constants on fp stack
			}
			else if (ifp > 1)
			{
				switch (tok)								// Perform operation on constants
				{
					case EQEV_TOK_ADD:
						ifp--;
						afp[ifp-1] += afp[ifp];
						break;

					case EQEV_TOK_SUB:
						ifp--;
						afp[ifp-1] -= afp[ifp];
						break;

					case EQEV_TOK_MUL:
						ifp--;
						afp[ifp-1] *= afp[ifp];
						break;

					case EQEV_TOK_DIV:
						ifp--;
						afp[ifp-1] /= afp[ifp];
						break;

					case EQEV_TOK_POW:
						ifp--;
						afp[ifp-1] = powk(afp[ifp-1], afp[ifp]);
						break;
				}
			}
			else if (EQEV_TOK_END == tokTemp)
			{
				tokTemp = tok;								// Store op code
			}
			else
			{												// Move previous temp to code stack
				error = eqeval_move_fp_to_code(afp, &ifp, atokNew, &itokNew);
				if (error)
					return error;
				error = eqeval_check_optimize_sps(itokNew, ifp);
				if (error)
					return error;
				atokNew[itokNew++]	= tokTemp;
				atokNew[itokNew++]	= tok;					// Move current op code to code stack
				tokTemp				= EQEV_TOK_END;
			}
			continue;
		}
															///////////////////////////////////////
		if (EQEV_T_UNARY & tok)								// Unary opcode
		{
			if (EQEV_TOK_END == tokTemp)
			{
				if (ifp)
				{
					switch (tok)							// Perform operation on constant
					{
						case EQEV_TOK_SQRT:
							afp[ifp-1] = sqrtk(afp[ifp-1]);
							break;

						case EQEV_TOK_SIN:
							afp[ifp-1] = sink(afp[ifp-1]);
							break;

						case EQEV_TOK_COS:
							afp[ifp-1] = cosk(afp[ifp-1]);
							break;

						case EQEV_TOK_TAN:
							afp[ifp-1] = tank(afp[ifp-1]);
							break;

						case EQEV_TOK_ATN:
							afp[ifp-1] = atank(afp[ifp-1]);
							break;

						case EQEV_TOK_LN:
							afp[ifp-1] = logk(afp[ifp-1]);
							break;

						case EQEV_TOK_NEG:
							afp[ifp-1] = -afp[ifp-1];
							break;

						case EQEV_TOK_ABS:
							if (afp[ifp-1] < 0)
								afp[ifp-1] = -afp[ifp-1];
							break;

						case EQEV_TOK_EXP:
							afp[ifp-1] = expk(afp[ifp-1]);
							break;
					}
					continue;
				}
			}
			else
			{												// Move previous temp to code stack
				error = eqeval_move_fp_to_code(afp, &ifp, atokNew, &itokNew);
				if (error)
					return error;
				error = eqeval_check_optimize_sps(itokNew, ifp);
				if (error)
					return error;
				atokNew[itokNew++] 	= tokTemp;
				tokTemp				= EQEV_TOK_END;
			}
			atokNew[itokNew++] = tok;						// Move current op code to code stack
			continue;
		}
															///////////////////////////////////////
		if (EQEV_T_INT == (EQEV_TOK_MASK_INT & tok))		// Integer value
		{
			afp[ifp++] = (EQEV_T_INT-1) & tok;				// Put constant onto fp stack
			continue;
		}
															///////////////////////////////////////
		if (EQEV_T_CONST & tok)								// FP value
		{
			convert.b[0] = g_atokCode[itokOld++];			// Put constant onto fp stack
			convert.b[1] = g_atokCode[itokOld++];
			convert.b[2] = g_atokCode[itokOld++];
			convert.b[3] = g_atokCode[itokOld++];
			afp[ifp++]	 = convert.k;
			continue;
		}
															///////////////////////////////////////
		if (EQEV_T_VAR & tok)								// X value
		{													// Move previous temp to code stack
			error = eqeval_move_fp_to_code(afp, &ifp, atokNew, &itokNew);
			if (error)
				return error;
			if (EQEV_TOK_END != tokTemp)
			{
				atokNew[itokNew++] 	= tokTemp;
				tokTemp				= EQEV_TOK_END;
			}
			atokNew[itokNew++] = tok;						// Move current op code to code stack
			continue;
		}
	}
															///////////////////////////////////////
															// End of code, clean up
	error = eqeval_move_fp_to_code(afp, &ifp, atokNew, &itokNew);
	if (error)
		return error;
	error = eqeval_check_optimize_sps(itokNew, ifp);
	if (error)
		return error;

	if (EQEV_TOK_END != tokTemp)
	{
		atokNew[itokNew++] = tokTemp;
	}
	atokNew[itokNew++] = EQEV_TOK_END;
	
	memcpy(g_atokCode, atokNew, itokNew);					// Move optimized code to code stack

	return EQEV_E_OK;
}

/*-------------------------------------------------------------------------------------------------
 * EQEVAL: Parse an equation string
 *
 *	Return value:
 *		EQEV_E_OK			Success
 *		EQEV_E_BADTOKEN		Invalid token
 *		EQEV_E_INVARG		Not enough fp on stack. First entry on code stack set to EQEV_TOK_END
 *		EQEV_E_TOOCOMPLEX	Code/Temp stack too small. First entry on code stack set EQEV_TOK_END
 *		EQEV_E_FPTOOSMALL	Fp stack too small. First entry on code stack set to EQEV_TOK_END
 *		EQEV_E_PARENTOODEEP	Parenthesis level too deep
 *		EQEV_E_UNBALPAREN	Open and close parenthesis do not match
 *-------------------------------------------------------------------------------------------------
 */
uint8_t eqeval_parse(char* psz)
{
//	char						szBuffer[EQEV_SIZE_EQ];
	char*						sz 		= psz;
	uint8_t 					tok  	= 0;
	uint8_t 					j;
	uint8_t 					error;
	const __flash eqevSymbol_t*	pSymbol;
	eqevConvert_t				convert;

	g_itokCode	  = 0;
	g_itemp		  = 0;
	g_bPriority   = 0;
	g_bDepth	  = 0;
	g_tokPrev	  = 0;
	g_atokCode[0] = EQEV_TOK_END;

	strupr(sz);
															///////////////////////////////////////
															// Main loop
	while ((tok = eqeval_get_token(&sz)) != EQEV_E_END)
	{
		if (EQEV_E_BADTOKEN == tok)
		{
			g_atokCode[0] = EQEV_TOK_END;
//			xprintf_P(PSTR("\r\nError: " EQEV_E_STR_ERROR "\r\n"));
			return EQEV_E_BADTOKEN;
		}	
		
		pSymbol = &(g_aSymbol[tok]);
		tok		= pSymbol->bToken;
		
		switch (pSymbol->bType)
		{													///////////////////////////////////////
			case EQEV_T_BINARY:								// Binary
				if (EQEV_TOK_SUB == tok)
				{											// Check if '-' is unary negate
					if (0 == g_tokPrev 								||
						(EQEV_T_UNARY + EQEV_T_BINARY) & g_tokPrev  ||
						EQEV_TOK_OP == g_tokPrev)
					{
						pSymbol = &(g_aSymbol[EQEV_SYMTAB_NEG]);
						tok 	= EQEV_TOK_NEG;
					}
				}
				if (EQEV_TOK_NEG != tok)
				{
					error = eqeval_check_temp(pSymbol->bPriority + g_bPriority);
					if (error)
						return error;
				}
				// No Break
															///////////////////////////////////////
			case EQEV_T_UNARY:								// Unary
				error = eqeval_check_temp_sp();
				if (error)
					return error;
				g_atemp[g_itemp].bPriority = pSymbol->bPriority + g_bPriority;
				g_atemp[g_itemp++].bToken  = tok;
				break;
															///////////////////////////////////////
			case EQEV_T_CONST:								// Constant value
				error = eqeval_check_fp();
				if (error)
					return error;
				g_bDepth++;
				if (EQEV_TOK_PI == tok)
				{
					g_k = 3.1415926535897932384626433832795;
					tok	= EQEV_TOK_FP;
				}
				else if (EQEV_TOK_E == tok)
				{
					g_k = 2.7182818284590452353602874713527;
					tok	= EQEV_TOK_FP;
				}
				if (g_k < EQEV_T_INT && g_k >= 0 && (uint8_t)g_k == g_k)
				{
					for (j = 0; j < EQEV_T_INT; j++)		// Check if integer between 0 to 31
					{
						if (j == (uint8_t)g_k)
						{
							error = eqeval_check_temp_sp();
							if (error)
								return error;
							g_atokCode[g_itokCode++] = EQEV_T_INT + j;
							break;
						}
					}
					
					if (j < EQEV_T_INT)
						break;
				}
				if (g_itokCode >= EQEV_SIZE_CODESTACK-4)
				{
					g_atokCode[0] = EQEV_TOK_END;
//					xprintf_P(PSTR("\r\nError: " EQEV_E_STR_TOOCOMPLEX "\r\n"));
					return EQEV_E_TOOCOMPLEX;
				}
				if (EQEV_TOK_NEG == g_tokPrev)				// Negate FP if previous op was neg
				{
					g_itemp--;
					g_k = -g_k;
				}
				convert.k = g_k;
				g_atokCode[g_itokCode++] = tok;
				g_atokCode[g_itokCode++] = convert.b[0];
				g_atokCode[g_itokCode++] = convert.b[1];
				g_atokCode[g_itokCode++] = convert.b[2];
				g_atokCode[g_itokCode++] = convert.b[3];
				break;
															///////////////////////////////////////
			case EQEV_T_VAR:								// X value
				error = eqeval_push_op(tok);
				if (error)
					return error;
				error = eqeval_check_fp();
				if (error)
					return error;
				g_bDepth++;
				break;
															///////////////////////////////////////
			case EQEV_T_PRIORITY:							// Open/Close parenthesis
				if (EQEV_TOK_OP == tok)
				{
					if (g_bPriority > (255-EQEV_MAX_PRIORITY+1))
					{
						g_atokCode[0] = EQEV_TOK_END;
//						xprintf_P(PSTR("\r\nError: " EQEV_E_STR_PARENTOODEEP "\r\n"));
						return EQEV_E_PARENTOODEEP;
					}
					g_bPriority += EQEV_MAX_PRIORITY+1;
				}
				else
				{
					if (0 == g_bPriority)
					{
						g_atokCode[0] = EQEV_TOK_END;
//						xprintf_P(PSTR("\r\nError: " EQEV_E_STR_UNBALPAREN "\r\n"));
						return EQEV_E_UNBALPAREN;
					}
					g_bPriority -= EQEV_MAX_PRIORITY+1;
				}
				break;
		}

		g_tokPrev = tok;									// Save token
		
		#if EQEV_OPTION_DEBUG > 0
		char szBuffer[50];
		
		if (EQEV_TOK_FP == pSymbol->bToken)
		{
			#ifdef __TINYC__
			printf("Token: [FP] Value: %f\r\n", g_k);
			#else
			xprintf_P(PSTR("Token: [FP] Value: %k\r\n"), g_k);
			#endif
		}
		else if (EQEV_E_BADTOKEN != tok)
		{
			j = pSymbol->cbSymbol;
			strncpy_P(szBuffer, pSymbol->psSymbol, j);
			szBuffer[j] = 0;
			xprintf_P(PSTR("Token: [%s]\r\n"), szBuffer);
		}
		#endif // EQEV_OPTION_DEBUG > 0

	}														// End of main loop
	
	if (g_bPriority)
	{
		g_atokCode[0] = EQEV_TOK_END;
//		xprintf_P(PSTR("\r\nError: " EQEV_E_STR_UNBALPAREN "\r\n"));
		return EQEV_E_UNBALPAREN;
	}
	
	while (g_itemp)											// Move temp stack to code stack
	{
		error = eqeval_check_depth(g_atemp[--g_itemp].bToken);		
		if (error)
			return error;
		error = eqeval_push_op(g_atemp[g_itemp].bToken);
		if (error)
			return error;
	}

	if (g_bDepth != 1 && g_atokCode[0] != EQEV_TOK_END)
	{
		g_atokCode[0] = EQEV_TOK_END;
		
		return EQEV_E_BADDEPTH;
	}
	
	error = eqeval_push_op(EQEV_TOK_END);
	if (error)
		return error;
	
	return EQEV_E_OK;
}

/*-------------------------------------------------------------------------------------------------
 * EQEVAL: Get error message for error number
 *-------------------------------------------------------------------------------------------------
 */
const __flash char* eqeval_get_err_msg(uint8_t isz)
{
	if (isz > 0 && isz <= EQEV_E_BADDEPTH)
	{
		return g_aszErrMsg[isz - 1];
	}
	
	if (EQEV_E_BADTOKEN == isz)
	{
		return g_aszErrMsg[EQEV_E_BADDEPTH];				// Err msg for EQEV_E_BADTOKEN
	}

	return 0;
}

/*-------------------------------------------------------------------------------------------------
 * EQEVAL: Evaluate equation for given value of X
 *
 *	Return value:
 *		Value of equation evaluated for X
 *-------------------------------------------------------------------------------------------------
 */
_Accum eqeval_eval(_Accum x)
{
	_Accum			afp[EQEV_SIZE_FPSTACK];
	eqevConvert_t	convert;
	uint8_t 		ifp = 0;
	uint8_t 		i 	= 0;
	
	while (1)
	{
		switch (g_atokCode[i])
		{
			case EQEV_TOK_ADD:
				ifp--;
				afp[ifp-1] += afp[ifp];
				break;

			case EQEV_TOK_SUB:
				ifp--;
				afp[ifp-1] -= afp[ifp];
				break;

			case EQEV_TOK_MUL:
				ifp--;
				afp[ifp-1] *= afp[ifp];
				break;

			case EQEV_TOK_DIV:
				ifp--;
				afp[ifp-1] /= afp[ifp];
				break;

			case EQEV_TOK_POW:
				ifp--;
				afp[ifp-1] = powk(afp[ifp-1], afp[ifp]);
				break;

			case EQEV_TOK_SQRT:
				afp[ifp-1] = sqrtk(afp[ifp-1]);
				break;

			case EQEV_TOK_SIN:
				afp[ifp-1] = sink(afp[ifp-1]);
				break;

			case EQEV_TOK_COS:
				afp[ifp-1] = cosk(afp[ifp-1]);
				break;

			case EQEV_TOK_TAN:
				afp[ifp-1] = tank(afp[ifp-1]);
				break;

			case EQEV_TOK_ATN:
				afp[ifp-1] = atank(afp[ifp-1]);
				break;

			case EQEV_TOK_LN:
				afp[ifp-1] = logk(afp[ifp-1]);
				break;

			case EQEV_TOK_NEG:
				afp[ifp-1] = -afp[ifp-1];
				break;

			case EQEV_TOK_ABS:
				if (afp[ifp-1] < 0)
					afp[ifp-1] = -afp[ifp-1];
				break;

			case EQEV_TOK_EXP:
				afp[ifp-1] = expk(afp[ifp-1]);
				break;

			case EQEV_TOK_FP:
				convert.b[0] = g_atokCode[++i];
				convert.b[1] = g_atokCode[++i];
				convert.b[2] = g_atokCode[++i];
				convert.b[3] = g_atokCode[++i];
				afp[ifp++]   = convert.k;
				break;

			case EQEV_TOK_X:
				afp[ifp++] = x;
				break;

			case EQEV_TOK_END:
				return afp[ifp-1];
				break;
				
			default:
				if (EQEV_T_INT == (EQEV_TOK_MASK_INT & g_atokCode[i]))
				{
					afp[ifp++] = (EQEV_T_INT-1) & g_atokCode[i];
				}
				else
				{
//					xprintf_P(PSTR("\r\nError: " EQEV_E_STR_BADOPCODE "\r\n"));
					return 0;
				}

		}
		
		i++;
	}
}

#ifdef __TINYC__
#include <stdio.h>
#include <string.h>

void dumpCode(void)
{
	uint8_t 		iCode;
	eqevConvert_t	convert;

	printf("\r\n");
	for (iCode = 0; iCode < EQEV_SIZE_CODESTACK; iCode++)
	{
		printf("%03d Token: %02X", iCode, g_atokCode[iCode]);
		if (EQEV_TOK_FP == g_atokCode[iCode])
		{
			convert.b[0] = g_atokCode[++iCode];
			convert.b[1] = g_atokCode[++iCode];
			convert.b[2] = g_atokCode[++iCode];
			convert.b[3] = g_atokCode[++iCode];
			xprintf_P(PSTR(" %f"), convert.k);
		}
		else if (EQEV_TOK_NEG == g_atokCode[iCode])
		{
			printf(" NEG");
		}
		else if (EQEV_TOK_END == g_atokCode[iCode])
		{
			printf(" END\r\n");
			break;
		}
		else if (EQEV_T_INT == (EQEV_TOK_MASK_INT & g_atokCode[iCode]))
		{
			printf(" %d", (EQEV_T_INT-1) & g_atokCode[iCode]);
		}
		else
		{
			for (uint8_t i = 0; i < EQEV_SYMTAB_SIZE; i++)
				if (g_atokCode[iCode] == g_aSymbol[i].bToken)
				{
					printf(" %s", g_aSymbol[i].psSymbol);
					break;
				}
		}
		printf("\r\n");
	}
}

int main(void)
{
	uint8_t error;
	char	sz[EQEV_SIZE_EQ];
	_Accum	x;

	g_atokCode[0] = EQEV_TOK_END;
	
	printf("Test program for the equation evaluator library\r\n");

	while (1)
	{
		printf("\r\nEnter an equation: ");
		fgets(sz, sizeof(sz), stdin);
		printf("\r\n");
		strupr(sz);
		
		if (strncmp_P(sz, PSTR("QUIT"), 4) == 0)
			break;
		
		if (strnicmp("X=", sz, 2) == 0)
		{
			x = atof(sz+2);
			printf("For x = %f equation = %f\r\n", x, eqeval_eval(x));
			continue;
		}
		
		if (strncmp_P(sz, PSTR("TABLE"), 5) == 0)
		{
			xprintf_P(PSTR("\r\n\r\n"), sz);
			for (x = -M_PI; x <= M_PI; x += 0.125)
				xprintf_P(PSTR("  %10f  %10f\r\n"), x, eqeval_eval(x));
			xprintf_P(PSTR("  %10f  %10f\r\n"), M_PI, eqeval_eval(M_PI));
			xprintf_P(PSTR("\r\n"), sz);
			continue;
		}
		
		if (strncmp_P(sz, PSTR("DUMP"), 4) == 0)
		{
			dumpCode();
			continue;
		}

		error = eqeval_parse(sz);
		if (error)
		{
			printf("Error: %s\r\n", eqeval_get_err_msg(error));
			continue;
		}

		if (g_itokCode)
		{
			printf("\r\nBefore Optimization:\r\n");
			dumpCode();
			printf("\r\nAfter Optimization:\r\n");
			eqeval_optimize();
			dumpCode();
		}
	}
	
	return 0;
}
#endif // __TINYC__
