/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * statusline.c - mcurses status line lib
 *
 * Copyright (c) 2016 David Harmon
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */


#include "statusline.h"

#include <string.h>

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES STATUS LINE: init_statusline
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void 
init_statusline(STATUSLINE* stat, uint8_t bRow, uint8_t bCol, uint8_t bCols, uint16_t wAttributes)
{
	stat->bRow		= bRow;
	stat->bCol		= bCol;
	stat->bCols		= bCols;
	stat->wAttrs	= wAttributes;
	stat->win		= 0;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES STATUS LINE: set_statusline_window
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void 
set_statusline_window(STATUSLINE* stat, WINDOW* win)
{
	if (0 == stat->win)
		stat->win = win;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES STATUS LINE: post_statusline
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void 
post_statusline(STATUSLINE* stat)
{
	uint16_t wAttrs;
	WINDOW*  win = stat->win;

	if (0 == win)
		return;

	if (0xFF == stat->bRow)
	{
		if (win->bFlags & WIN_FLAG_STATUSLINE)
			return;

		stat->bRow	 = win->bMaxy;
		win->bFlags |= WIN_FLAG_STATUSLINE;
		win->bFlags &= ~WIN_FLAG_FULLWIN;
		win->bMaxy--;

		if (win->bFlags & WIN_FLAG_TERMSCRLRGN)
			mcurses_term_scrreg(win, TRUE);
	}
	
	if (0xFF == stat->bCols)
		stat->bCols = win->bMaxx;
	
	wAttrs = win->wAttrs;
	wattrset(win, stat->wAttrs);
	wmove_raw(win, stat->bRow, stat->bCol);
	wclrtoeol(win);
	wattrset(win, wAttrs);
	wmove_raw(win, win->bCury, win->bCurx);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES STATUS LINE: unpost_statusline
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void 
unpost_statusline(STATUSLINE* stat)
{
	WINDOW* win = stat->win;

	if (0 == win)
		return;

	if (win->bFlags & WIN_FLAG_STATUSLINE && stat->bRow == (win->bMaxy+1))
	{
		wattrset(win, win->wAttrs);
		wmove_raw(win, stat->bRow, stat->bCol);
		wclrtoeol(win);
		wmove_raw(win, win->bCury, win->bCurx);

		win->bFlags &= ~WIN_FLAG_STATUSLINE;
		win->bMaxy++;
		
		if (win->bMaxy == win->scr->bMaxy && win->bMaxx == win->scr->bMaxx)
			win->bFlags |= WIN_FLAG_FULLWIN;

		if (win->bFlags & WIN_FLAG_TERMSCRLRGN)
			mcurses_term_scrreg(win, TRUE);
	}

	stat->bRow = 0;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES STATUS LINE: statusline_draw
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void 
statusline_draw(STATUSLINE* stat, char* pcText, uint8_t bFieldWidth, uint8_t bOffset, uint8_t bOptions)
{
	uint16_t wAttrs;
	uint8_t  bColOffset  = 0;													// Column for start of text
	uint8_t	 bTextLength = strlen(pcText);
	WINDOW*	 win		 = stat->win;

	if (0 == win)
		return;

	if (bFieldWidth && (bFieldWidth <= win->bMaxx - stat->bCol))
	{
		switch (bOptions & SL_OPTION_FIELD_JUSTIFY_MASK)
		{
			case SL_OPTION_FIELD_JUSTIFY_LEFT:
				bColOffset = stat->bCol;
				if (bOptions & SL_OPTION_FIELD_OFFSET)
					bColOffset += bOffset;
				break;
				
			case SL_OPTION_FIELD_JUSTIFY_RIGHT:
				bColOffset = win->bMaxx - bFieldWidth;
				if (bOptions & SL_OPTION_FIELD_OFFSET)
					bColOffset -= bOffset;
				break;
				
			case SL_OPTION_FIELD_JUSTIFY_CENTER:
				bColOffset = ((win->bMaxx - stat->bCol) - bFieldWidth)/2;
				break;
		}
		
		if (bOptions & SL_OPTION_FIELD_OFFSET)									// Offset only used for field or text, not both
			bOptions &= ~SL_OPTION_TEXT_OFFSET;
	}
	else
	{
		bFieldWidth = win->bMaxx - stat->bCol;
		bColOffset 	= stat->bCol;
	}
	
	if (bTextLength > bFieldWidth)
		bTextLength = bFieldWidth;
	
	switch (bOptions & SL_OPTION_TEXT_JUSTIFY_MASK)
	{
		case SL_OPTION_TEXT_JUSTIFY_LEFT:
			if (bOptions & SL_OPTION_TEXT_OFFSET)
			{
				bFieldWidth -= bOffset;
				bColOffset  += bOffset;
			}
			break;
			
		case SL_OPTION_TEXT_JUSTIFY_RIGHT:
			bColOffset += bFieldWidth - bTextLength;
			if (bOptions & SL_OPTION_TEXT_OFFSET)
			{
				if (bOffset > bFieldWidth - bTextLength)
					bOffset = bFieldWidth - bTextLength;

				bFieldWidth -= bOffset;
				bColOffset  -= bOffset;
			}
			break;
			
		case SL_OPTION_TEXT_JUSTIFY_CENTER:
			bColOffset = (bFieldWidth - bTextLength)/2;
			break;
	}

	wAttrs = win->wAttrs;
	wattrset(win, stat->wAttrs);
	if (bOptions & SL_OPTION_CLEAR_FIRST)
		wclrrec(win, stat->bRow, stat->bCol+bColOffset, 1, bFieldWidth);
	mvwaddnstr_raw(win, stat->bRow, stat->bCol+bColOffset, pcText, bFieldWidth);
	wattrset(win, wAttrs);
	wmove_raw(win, win->bCury, win->bCurx);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES STATUS LINE: statusline_draw_raw
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void 
statusline_draw_raw(STATUSLINE* stat, char* pcText, uint8_t cbText)
{
	uint16_t wAttrs;
	WINDOW*	 win =  stat->win;

	if (0 == win)
		return;

	cbText = strlen(pcText);
	if (cbText > win->bMaxx - stat->bCol)
		cbText = win->bMaxx - stat->bCol;
	
	wAttrs = win->wAttrs;
	wattrset(win, stat->wAttrs);
	mvwaddnstr_raw(win, stat->bRow, stat->bCol, pcText, cbText);
	wattrset(win, wAttrs);
	wmove_raw(win, win->bCury, win->bCurx);
}
