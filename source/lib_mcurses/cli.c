/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * cli.c - mcurses command line interface lib
 *
 * Copyright (c) 2016 David Harmon
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */


#include "cli.h"
#include "../lib_xio/xio.h"
#include <string.h>
#include <ctype.h>

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES CLI: Flash string constants defined in the cli-config.h file
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
//const __flash char CLI_STR_VERSION[]	= CLI_CONFIG_VERSION;
const __flash char CLI_STR_CRLF[]		= CLI_CONFIG_CRLF;
const __flash char CLI_STR_PROMPT[]		= CLI_CONFIG_PROMPT;
const __flash char CLI_STR_CMDERR[]		= CLI_CONFIG_CMDERR;
const __flash char CLI_STR_BADCMD[]		= CLI_CONFIG_BADCMD;
const __flash char CLI_STR_HELPTAIL[]	= CLI_CONFIG_HELPTAIL;
const __flash char CLI_STR_ARGS[]		= CLI_CONFIG_ARGS;
const __flash char CLI_STR_DELIM[]		= " ";

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES CLI: cli_init_arg_parser
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void 
cli_init_arg_parser(CommandArgumentParser_t* pCAP, uint8_t bCommandID)
{
	uint8_t bStart;
	uint8_t bEnd;
	
	for (bStart = 0; CLI_ENTRY_END != g_aCommandArgument[bStart].bCommandID; bStart++)
		if (bCommandID == g_aCommandArgument[bStart].bCommandID)
			break;
			
	for (bEnd = bStart+1; CLI_ENTRY_END != g_aCommandArgument[bEnd].bCommandID; bEnd++)
		if (bCommandID != g_aCommandArgument[bEnd].bCommandID)
			break;

	bEnd--;

	pCAP->bStart 		= bStart;
	pCAP->bEnd   		= bEnd;
	pCAP->bArgumentID	= CLI_ENTRY_END;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES CLI: cli_detailed_help
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
uint8_t 
cli_detailed_help(CommandArgumentParser_t* pCAP, uint8_t bCommandID, uint8_t bSequence, char* pcBuff, uint8_t bBuffLen)
{
	uint8_t cb;
	uint8_t cbHelp;
	
	if (bBuffLen < sizeof(CLI_STR_CRLF))
		return CLI_CFR_FINISHED;
	
	if (0 == bSequence)
	{
		for (bSequence = 0; CLI_ENTRY_END != g_aCommandEntry[bSequence].bCommandID; bSequence++)
			if (g_aCommandEntry[bSequence].bCommandID == bCommandID)
				break;
		
		// Format: cmnd:helpText \0
		cb = strlen_P(g_aCommandEntry[bSequence].pcCommand);
		if (cb+1 > bBuffLen)
			cb = bBuffLen-1;

		strncpy_P(pcBuff, g_aCommandEntry[bSequence].pcCommand, cb);
		pcBuff[cb] = ':';

		cbHelp = strlen_P(g_aCommandEntry[bSequence].pcHelp);
		if (cb+cbHelp+2 > bBuffLen)
		{
			cbHelp = bBuffLen-cb-1;
			if (cbHelp > 0)
				cbHelp--;
		}

		if (cbHelp > 0)
			cb++;

		strncpy_P(pcBuff+cb, g_aCommandEntry[bSequence].pcHelp, cbHelp);

		pcBuff[cb+cbHelp] = 0;

		return CLI_CFR_ADDITIONAL_OUTPUT;
	}
	else if (1 == bSequence)
	{
		strncpy_P(pcBuff, CLI_STR_ARGS, bBuffLen);
		return CLI_CFR_ADDITIONAL_OUTPUT;
	}
	
	bSequence += pCAP->bStart - 2;

	if (bSequence > pCAP->bEnd)
		return CLI_CFR_FINISHED;

	if (CLI_CAT_EXAMPLE == g_aCommandArgument[bSequence].bType)
	{
		// Example format: \r\nText
		strncpy_P(pcBuff, CLI_STR_CRLF, bBuffLen);
		strncpy_P(pcBuff+sizeof(CLI_STR_CRLF)-1, g_aCommandArgument[bSequence].pcHelp, bBuffLen-(sizeof(CLI_STR_CRLF)-1));
	}
	else
	{
		// Argument format: ' ' arg \t helpText \0
		cb = strlen_P(g_aCommandArgument[bSequence].pcArgument)+1;								// Add one for leading ' '
		if (cb+1 > bBuffLen)
			cb = bBuffLen-1;

		pcBuff[0] = ' ';
		strncpy_P(pcBuff+1, g_aCommandArgument[bSequence].pcArgument, cb);
		pcBuff[cb] = '\t';

		cbHelp = strlen_P(g_aCommandArgument[bSequence].pcHelp);
		if (cb+cbHelp+2 > bBuffLen)
		{
			cbHelp = bBuffLen-cb-1;
			if (cbHelp > 0)
				cbHelp--;
		}

		if (cbHelp > 0)
			cb++;

		strncpy_P(pcBuff+cb, g_aCommandArgument[bSequence].pcHelp, cbHelp);

		pcBuff[cb+cbHelp] = 0;
	}

	return CLI_CFR_ADDITIONAL_OUTPUT;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES CLI: cli_get_arg
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
uint8_t
cli_get_arg(CommandArgumentParser_t* pCAP, char* pcBuff)
{
	char*	pcToken;
	uint8_t	i;
	
	if (0 == pCAP || 0 == pcBuff)
		return CLI_GA_FINISHED;

	if (CLI_ENTRY_END == pCAP->bArgumentID)
	{
		strtok_rP(pcBuff, CLI_STR_DELIM, &pCAP->pcLast);						// Skip command
		pCAP->bArgumentID = 0;
	}
	
	pcToken = strtok_rP(0, CLI_STR_DELIM, &pCAP->pcLast);						// Get argument

	if ('/' == *pcToken || '-' == *pcToken)										// Ignore any leading / and -
		pcToken++;

	for (i = pCAP->bStart; i <= pCAP->bEnd; i++)
	{
		if (strcasecmp_P(pcToken, g_aCommandArgument[i].pcArgument) == 0)
		{
			pCAP->bArgumentID = g_aCommandArgument[i].bArgumentID;
			
			if (g_aCommandArgument[i].bType & (CLI_CAT_INT | CLI_CAT_FIXED))
				pcToken = strtok_rP(0, CLI_STR_DELIM, &pCAP->pcLast);
			
			switch (g_aCommandArgument[i].bType)
			{
				case CLI_CAT_TEXT:
					for (; 0 != *pCAP->pcLast; pCAP->pcLast++)
						if (' ' != *pCAP->pcLast)
							break;
							
					if ('"' == *pCAP->pcLast)
					{
						pcToken = ++pCAP->pcLast;								// Argument value is a quoted string

						for (; 0 != *pCAP->pcLast; pCAP->pcLast++)
							if ('"' == *pCAP->pcLast)
								break;
								
						if ('"' != *pCAP->pcLast)
							return CLI_GA_BAD_ARGUMENT;
							
						*pCAP->pcLast++ = 0;
					}
					else
						pcToken = strtok_rP(0, CLI_STR_DELIM, &pCAP->pcLast);	// Argument value is single character or word
						
					if (pcToken)
						pCAP->xValue.pcText = pcToken;
					else
						return CLI_GA_BAD_ARGUMENT;
					break;

				case CLI_CAT_INT:
					if (!xatoi(&pcToken, &pCAP->xValue.lValue))
						return CLI_GA_BAD_ARGUMENT;
					break;

				case CLI_CAT_FIXED:
					if (!xatok(&pcToken, &pCAP->xValue.kValue, xioFRACBITS_K, 0))
						return CLI_GA_BAD_ARGUMENT;
					break;

				case CLI_CAT_BOOL:
					break;

				case CLI_CAT_EXAMPLE:
					return CLI_GA_BAD_ARGUMENT;
					break;
			}
			
			return CLI_GA_ARGUMENT;
		}
		
	}
		
	if (i > pCAP->bEnd)
		return CLI_GA_BAD_ARGUMENT;

	return CLI_GA_FINISHED;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES CLI: cli_help
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
uint8_t 
cli_help(uint8_t bID, uint8_t bSequence, char* pcBuff, uint8_t bBuffLen)
{
	uint8_t cb;
	uint8_t cbHelp;

#warning "Test replacing g_aCommandEntry[bSequence] with local variable"
	if (CLI_ENTRY_END == g_aCommandEntry[bSequence].bCommandID)					// Use the sequence number to step through the command entry table
	{
		strncpy_P(pcBuff, CLI_STR_HELPTAIL, sizeof(CLI_STR_HELPTAIL));
		return CLI_CFR_OUTPUT;
	}

																				// Output the table with one line per command using the format:
																				//		cmnd \t helpText \0
	cb = strlen_P(g_aCommandEntry[bSequence].pcCommand);
	if (cb+1 > bBuffLen)
		cb = bBuffLen-1;

	strncpy_P(pcBuff, g_aCommandEntry[bSequence].pcCommand, cb);
	pcBuff[cb] = '\t';

	cbHelp = strlen_P(g_aCommandEntry[bSequence].pcHelp);
	if (cb+cbHelp+2 > bBuffLen)
	{
		cbHelp = bBuffLen-cb-1;
		if (cbHelp > 0)
			cbHelp--;
	}

	if (cbHelp > 0)
		cb++;

	strncpy_P(pcBuff+cb, g_aCommandEntry[bSequence].pcHelp, cbHelp);

	pcBuff[cb+cbHelp] = 0;

	return CLI_CFR_ADDITIONAL_OUTPUT;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES CLI: cli_driver
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
uint8_t 
cli_driver(WINDOW* win, GetnstrState_t* pSM, uint8_t bOptions, char* pcBuff, uint8_t bBuffLen)
{
	uint8_t	i;
	const __flash CommandEntry_t* pCE;
	
	switch (pSM->state)
	{
		case MC_SM_START:
			curs_set(win, 1);													// set cursor visible
			pcBuff[0] = 0;
			wattrset(win, A_BOLD);
			waddstr_P(win, CLI_STR_PROMPT);
			wattrset(win, A_NORMAL);
			// No break

		#if CLI_CONFIG_USE_WGETNSTR_FIELD > 0
		case MC_SM_SELECTED:
		#endif // CLI_CONFIG_USE_WGETNSTR_FIELD > 0
		case MC_SM_GETCH:
			#if CLI_CONFIG_USE_WGETNSTR_FIELD > 0
			if (wgetnstr_field_sm(win, pSM, pcBuff, bBuffLen-sizeof(CLI_STR_PROMPT), MC_T_CL) == MC_SM_FINISHED)
			#else // CLI_CONFIG_USE_WGETNSTR_FIELD > 0
			if (wgetnstr_sm(win, pSM, pcBuff, bBuffLen-sizeof(CLI_STR_PROMPT)) == MC_SM_FINISHED)
			#endif // CLI_CONFIG_USE_WGETNSTR_FIELD > 0
			{
				waddstr_P(win, CLI_STR_CRLF);

				for (i = 0; i < bBuffLen; i++)									// Remove leading spaces from command line
					if (!isspace(pcBuff[i]))
						break;
		
				if (i)
					strncpy(pcBuff, pcBuff+i, bBuffLen-i);
		
				for (i = 0; i < bBuffLen; i++)									// Find first space after command
					if (!isgraph(pcBuff[i]))
						break;
		
				if (0 == i)														// Empty line
				{
					pSM->state = MC_SM_START;
					break;
				}

				pCE = g_aCommandEntry;
				while(CLI_ENTRY_END != pCE->bCommandID)
				{
					if (strlen_P(pCE->pcCommand) == i && strncasecmp_P(pcBuff, pCE->pcCommand, i) == 0)
					{
						if (pCE->pFunc)
						{
							pSM->ch    = 0;										// Use pSM->ch as the sequence number
							pSM->state = CLI_SM_CALL_FUNC;
							((CommandUtility_t*)pSM)->pCE = pCE;
						}
						else
							pSM->state = MC_SM_START;
						break;
					}
					pCE++;
				}

				if (CLI_ENTRY_END == pCE->bCommandID)
				{
					if (CLI_OPTION_BADCMD & bOptions)
						waddstr_P(win, CLI_STR_BADCMD);
					pSM->state = MC_SM_START;
				}
			}
			break;

		case CLI_SM_CALL_FUNC:
			pCE = ((CommandUtility_t*)pSM)->pCE;
			switch (pCE->pFunc(pCE->bCommandID, pSM->ch++, pcBuff, bBuffLen))
			{
				case CLI_CFR_EXIT:
					return MC_SM_FINISHED;
					break;

				case CLI_CFR_OUTPUT:
					pSM->state = MC_SM_START;
					// No break
					
				case CLI_CFR_ADDITIONAL_OUTPUT:
					waddstr(win, pcBuff);
					waddstr_P(win, CLI_STR_CRLF);
					break;

				default:
					if (CLI_OPTION_CMDERR & bOptions)
						waddstr_P(win, CLI_STR_CMDERR);
					// No break

				case CLI_CFR_FINISHED:
					pSM->state = MC_SM_START;
					break;
			}
			break;
	}

	return MC_SM_GETCH;
}
