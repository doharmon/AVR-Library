/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * form_config.h - Configuration file for mcurses form lib
 *
 * Copyright (c) 2016 David Harmon
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */


#ifndef FORM_CONFIG_H_
#define FORM_CONFIG_H_

#define FORM_CONFIG_RADIO_CLEAR		"( )"
#define FORM_CONFIG_RADIO_CHECKED	"(\x80)"				// ACS_DIAMOND

#define FORM_CONFIG_CB_CLEAR		"[ ]"
#define FORM_CONFIG_CB_CHECKED		"[\x80]"				// ACS_DIAMOND

#define FORM_CONFIG_BUFFER_SIZE		(80)					// Needs to be one more than largest text field length (bParam in FormItem_t array)

#define FORM_CONFIG_USE_CR_AS_TAB	0						// A carriage return behaves like a tab

#endif /* FORM_CONFIG_H_ */
