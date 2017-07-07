/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * form.h - Include file for mcurses statusline lib
 *
 * Copyright (c) 2016 David Harmon
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */

 
#ifndef STATUSLINE_H
#define STATUSLINE_H

#include "mcurses.h"

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Status line options
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define SL_OPTION_FIELD_JUSTIFY_LEFT		0x00												// Left justify field
#define SL_OPTION_FIELD_JUSTIFY_RIGHT		0x01												// Right justify field
#define SL_OPTION_FIELD_JUSTIFY_CENTER		0x02												// Center the field
#define SL_OPTION_FIELD_JUSTIFY_MASK		0x03												// Mask for field justify
#define SL_OPTION_FIELD_OFFSET				0x04												// Offset the field
#define SL_OPTION_TEXT_JUSTIFY_LEFT			0x00												// Left justify text within field
#define SL_OPTION_TEXT_JUSTIFY_RIGHT		0x10												// Right justify text within field
#define SL_OPTION_TEXT_JUSTIFY_CENTER		0x20												// Center the text within field
#define SL_OPTION_TEXT_JUSTIFY_MASK			0x30												// Mask for text justify
#define SL_OPTION_TEXT_OFFSET				0x40												// Offset the text within field
#define SL_OPTION_CLEAR_FIRST				0x80												// Clear field before printing text

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Structure for status line state
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
typedef struct statusline
{
	uint8_t		bRow;
	uint8_t		bCol;
	uint8_t		bCols;
	uint16_t	wAttrs;
	WINDOW*		win;
} STATUSLINE;

void init_statusline(STATUSLINE* stat, uint8_t bRow, uint8_t bCol, uint8_t bCols, uint16_t wAttributes);
void set_statusline_window(STATUSLINE* stat, WINDOW* win);
void post_statusline(STATUSLINE* stat);
void unpost_statusline(STATUSLINE* stat);
void statusline_draw(STATUSLINE* stat, char* pcText, uint8_t bFieldWidth, uint8_t bOffset, uint8_t bOptions);
void statusline_draw_raw(STATUSLINE* stat, char* pcText, uint8_t cbText);

#endif // STATUSLINE_H
