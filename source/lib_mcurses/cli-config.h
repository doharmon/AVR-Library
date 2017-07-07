/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * cli_config.h - Configuration file for mcurses command line interface lib
 *
 * Copyright (c) 2016 David Harmon
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */


#ifndef CLI_CONFIG_H_
#define CLI_CONFIG_H_

#define CLI_CONFIG_VERSION	"MCURSES CLI Version 1.0"
#define CLI_CONFIG_CRLF		"\r\n"
#define CLI_CONFIG_PROMPT	"cli> "
#define CLI_CONFIG_CMDERR	"Internal Error: Invalid command function return value\r\n"
#define CLI_CONFIG_BADCMD	"Error: Invalid command\r\n"
#define CLI_CONFIG_HELPTAIL	"Some commands support /? to give additional help"
#define CLI_CONFIG_ARGS		"Arguments"

#define CLI_CONFIG_USE_WGETNSTR_FIELD	0	// Use wgetnstr_field_sm instead of wgetnstr_sm. Saves space if only wgetnstr_field_sm is used by app.

#endif /* CLI_CONFIG_H_ */
