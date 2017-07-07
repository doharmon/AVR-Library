/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * cli.h - Include file for mcurses command line interface lib
 *
 * Copyright (c) 2016 David Harmon
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */


#ifndef CLI_H_
#define CLI_H_

#include "mcurses.h"
#include <inttypes.h>

#include "cli-config.h"

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Defines for command line interface options. Passed to cli_driver.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define CLI_OPTION_CMDERR						0x01							// Print error message CLI_CONFIG_CMDERR if a command returns an invalid code
#define CLI_OPTION_BADCMD						0x02							// Print error message CLI_CONFIG_BADCMD if an invalid command is entered

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Defines for command function return codes
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define CLI_CFR_FINISHED						0x00							// Command has finished
#define CLI_CFR_EXIT							0x01							// Exit the command line interface
#define CLI_CFR_OUTPUT							0x10							// Print a line of output
#define CLI_CFR_ADDITIONAL_OUTPUT				0x20							// Print a line of output. Call function again for additional lines.
																				// Also used as a state for GetnstrState_t

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Defines for reserved states in GetnstrState_t
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define CLI_SM_CALL_FUNC						0x08							// Call the command's callback function

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Last entry in the command entries needs to be CLI_ENTRY_END
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define CLI_ENTRY_END							255

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Structure for command entries
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
typedef uint8_t	(*pCEFunc)(	uint8_t	bCommandID,
							uint8_t	bSequence,
							char*	pcBuff,
							uint8_t	bBuffLen);
 
typedef struct commandentry
{
	uint8_t				bCommandID;
	const __flash char*	pcCommand;
	const __flash char*	pcHelp;
	pCEFunc				pFunc;
} CommandEntry_t;

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * The global array g_aCommandEntry needs to be defined and configured with all the commands that well be used by the application
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
extern const __flash CommandEntry_t g_aCommandEntry[];

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Command entry structure should include the built in help function
 *---------------------------------------------------------------------------------------------------------------------------------------------------

	// Command entries
	const __flash char CE_HELP[]					= "help";
	const __flash char CE_XXXX[]					= "xxxx";

	// Help descriptions
	const __flash char HD_HELP[]					= "List the available commands with a short description.";
	const __flash char HD_XXXX[]					= "Description of command";

	const __flash CommandEntry_t g_aCommandEntry[] =
	{
	//	  CommandID,		Command,	Help,		Function
		{ 1,				CE_HELP,	HD_HELP,	cli_help },
		{ 2,				CE_XXXX,	HD_XXXX,	cmd_func },
		.
		.
		.
		{ CLI_ENTRY_END,	0,			0,			NULL }
	};

 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Structure to hold command argument entries
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
typedef struct commandargument
{
	uint8_t				bCommandID;
	uint8_t				bArgumentID;
	uint8_t				bType;
	const __flash char*	pcArgument;
	const __flash char*	pcHelp;
} CommandArgument_t;

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * The global array g_aCommandArgument needs to be defined and configured with all the command arguments that well be used by the application
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
extern const __flash CommandArgument_t g_aCommandArgument[];

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Defines for command argument types
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define CLI_CAT_TEXT							0x01							// Text string. Surround with " to include spaces.
#define CLI_CAT_INT								0x02							// Long integer. 0###: Octal 0x###:Hex
#define CLI_CAT_FIXED							0x04							// Fixed point number
#define CLI_CAT_BOOL							0x08							// Returns 1 if argument on command line
#define CLI_CAT_EXAMPLE							0x10							// Printed as part of detailed help. Not an argument. Last in list.

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Defines for cli_get_arg function return
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define CLI_GA_FINISHED							0x00
#define CLI_GA_ARGUMENT							0x01
#define CLI_GA_BAD_ARGUMENT						0x02

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Structure for command argument parser
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
typedef struct commandargumentparser
{
	uint8_t bStart;
	uint8_t bEnd;
	uint8_t bArgumentID;
	char*	pcLast;
	union
	{
		char*	pcText;
		long	lValue;
		_Accum	kValue;
	} xValue;
} CommandArgumentParser_t;

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Structure to save command entry into the GetnstrState_t structure
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
typedef struct commandutility
{
	uint8_t						  bDummy;
	const __flash CommandEntry_t* pCE;
} CommandUtility_t;

uint8_t cli_detailed_help(CommandArgumentParser_t* pCAP, uint8_t bCommandID, uint8_t bSequence, char* pcBuff, uint8_t bBuffLen);
void	cli_init_arg_parser(CommandArgumentParser_t* pCAP, uint8_t bCommandID);
uint8_t cli_get_arg(CommandArgumentParser_t* pCAP, char* pcBuff);
uint8_t cli_help(uint8_t bID, uint8_t bSequence, char* pcBuff, uint8_t bBuffLen);
uint8_t cli_driver(WINDOW* win, GetnstrState_t* pSM, uint8_t bOptions, char* pcBuff, uint8_t bBuffLen);

#endif /* CLI_H_ */
