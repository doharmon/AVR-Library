/*-------------------------------------------------------------------------------------------------
 * eqeval-config.h - configuration file for equation evaluator lib
 *
 * Copyright (c) 2017 David Harmon
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *-------------------------------------------------------------------------------------------------
 */

#ifndef EQEVAL_CONFIG_H_
#define EQEVAL_CONFIG_H_

#define EQEV_OPTION_DEBUG			0				// Set to nonzero for debug statements

#define EQEV_SIZE_EQ				75				// Max length of input equation, including ending 0
#define EQEV_SIZE_CODESTACK			100				// VM code stack size
#define EQEV_SIZE_FPSTACK			10				// FP stack size

#define EQEV_E_STR_INVARG			"Invalid number of arguments"
#define EQEV_E_STR_TOOCOMPLEX		"Equation exceeds code stack"
#define EQEV_E_STR_FPTOOSMALL		"FP stack is too small"
#define EQEV_E_STR_PARENTOODEEP		"Parentheses level too deep"
#define EQEV_E_STR_UNBALPAREN		"Unbalanced parentheses"
#define EQEV_E_STR_BADOPCODE		"Invalid op code"
#define EQEV_E_STR_OPTIMIZERSP		"Optimizer exceeded code/fp stack"
#define EQEV_E_STR_BADTOKEN			"Invalid token"
#define EQEV_E_STR_BADDEPTH			"FP stack depth should be one"

#ifdef __TINYC__
#define __flash
#define PSTR
#define _Accum 			float
#define strncmp_P		strncmp
#define xprintf_P		printf
#define strncpy_P		strncpy
#define powk			pow
#define sqrtk			sqrt
#define sink			sin
#define cosk			cos
#define tank			tan
#define atank			atan
#define logk			log
#define expk			exp
#endif

#endif // EQEVAL_CONFIG_H_
