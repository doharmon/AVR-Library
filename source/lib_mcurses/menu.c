/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * menu.c - mcurses menu lib
 *
 * Copyright (c) 2016 David Harmon
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */


#include "menu.h"
#include "../lib_xio/xio.h"

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES MENU: init_menu
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void 
init_menu(MENU* menu, uint8_t bMenuID, uint8_t bRow, uint8_t bCol, uint8_t bOptions, uint16_t wAttributes)
{
	uint8_t		bStart;
	uint8_t		bEnd;

	for (bStart = 0; MENU_ENTRY_END != g_aMenuItem[bStart].bMenuID; bStart++)
		if (bMenuID == g_aMenuItem[bStart].bMenuID)
			break;
			
	for (bEnd = bStart+1; MENU_ENTRY_END != g_aMenuItem[bEnd].bMenuID; bEnd++)
		if (bMenuID != g_aMenuItem[bEnd].bMenuID)
			break;

	bEnd--;

	menu->bState	= MENU_SM_PROCESS;
	menu->bMenuID	= bMenuID;
	menu->bStart	= bStart;
	menu->bSelected	= bStart;
	menu->bFirst	= bStart;
	menu->bEnd		= bEnd;
	menu->bLast		= bEnd;
	menu->bRow		= bRow;
	menu->bCol		= bCol;
	menu->bOptions  = bOptions;
	menu->wAttrs	= wAttributes;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES MENU: set_menu_window
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void 
set_menu_window(MENU* menu, WINDOW* win)
{
	menu->win = win;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES MENU: post_menu
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void 
post_menu(MENU* menu)
{
	WINDOW*  win = menu->win;

	if (0 == win)
		return;

	if (win->bFlags & WIN_FLAG_MENUBAR)
		return;

	win->bFlags |= WIN_FLAG_MENUBAR;
	win->bFlags &= ~WIN_FLAG_FULLWIN;
	win->bBegy++;
	win->bMaxy--;
	
	menu->bRow--;

	if (win->bFlags & WIN_FLAG_TERMSCRLRGN)
		mcurses_term_scrreg(win, TRUE);

	menu_draw_menubar(menu);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES MENU: unpost_menu
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void 
unpost_menu(MENU* menu)
{
	WINDOW* win = menu->win;

	if (0 == win)
		return;

	if (win->bFlags & WIN_FLAG_MENUBAR)
	{
		wattrset(win, win->wAttrs);
		wmove_raw(win, menu->bRow, menu->bCol);
		wclrtoeol(win);
		wmove_raw(win, win->bCury, win->bCurx);

		win->bFlags &= ~WIN_FLAG_MENUBAR;
		win->bBegy--;
		win->bMaxy++;

		if (win->bMaxy == win->scr->bMaxy && win->bMaxx == win->scr->bMaxx)
			win->bFlags |= WIN_FLAG_FULLWIN;

		if (win->bFlags & WIN_FLAG_TERMSCRLRGN)
			mcurses_term_scrreg(win, TRUE);
	}

	menu->bRow = 0;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES MENU: menu_draw_menubar
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void 
menu_draw_menubar(MENU* menu)
{
	uint8_t		i;
	uint8_t		cbWidth;
	uint8_t		cbSpace;
	uint8_t		cbLen;
	uint8_t		bRadioButton;
	uint16_t	wAttr;
	WINDOW*		win = menu->win;

    if (win->scr->bFlags & SCR_FLAG_CHARSET)
    {
#if MCURSES_ENABLE_TODO > 0
#warning "Should charset be switched back to G1 at end of function?"
#endif
	    waddch_raw (win, '\017');												// switch to G0 set
	    win->scr->bFlags &= CHARSET_G0;
    }

	curs_set(win, 0);
	cbWidth = (win->bMaxx - menu->bCol) - 1;									// Decrease width by one. Don't print on last column.
	
	if (menu->bSelected < menu->bFirst)
		menu->bFirst = menu->bSelected;		

	if (menu->bSelected > menu->bLast)
	{
		if (menu->bSelected == menu->bEnd)
			cbSpace = 0;
		else
			cbSpace = 2;														// Include space for < and >
		
		for (i = menu->bSelected; i >= menu->bStart; i--)						// Trace back to find first menu item to display
		{
			cbSpace += strlen_P(g_aMenuItem[i].pcText) + 2;						// Space between menu items is 2
			if (cbSpace > cbWidth)
			{
				i++;
				break;
			}
 		}
			
		menu->bFirst = i;
	}

	wmove_raw(win, menu->bRow, menu->bCol);
	wAttr = win->wAttrs;
	wattrset(win, A_NORMAL | menu->wAttrs);

	cbSpace = 0;
	if (menu->bStart != menu->bFirst)
	{
		waddch_raw(win, '<');
		cbSpace++;
	}

	menu->bLast = menu->bFirst;
	for (i = menu->bFirst; i <= menu->bEnd; i++)								// Display menu items from first to last
	{
		bRadioButton = 0;
		if (MENU_ITEM_RADIO_BUTTON == g_aMenuItem[i].bOptions && g_aMenuItem[i].pFunc)
		{
			if (MENU_FUNC_RB_SELECTED == g_aMenuItem[i].pFunc(g_aMenuItem[i].bItemID, &menu->bData, MENU_FUNC_OPTION_PREDRAW))
				bRadioButton = 1;
		}
		cbLen = strlen_P(g_aMenuItem[i].pcText) + ((i == menu->bFirst)?bRadioButton:2);
		if (cbSpace + cbLen < cbWidth)
		{
			menu->bLast = i;
			if (i != menu->bFirst)
			{
				waddch_raw(win, ' ');
				if (!bRadioButton)
					waddch_raw(win, ' ');
			}
			if (i == menu->bSelected)
				wattrset(win, A_REVERSE | menu->wAttrs);
			if (bRadioButton)
				waddch_graphic_raw(win, ACS_DIAMOND);
			waddstr_P_raw(win, g_aMenuItem[i].pcText);
			if (i == menu->bSelected)
				wattrset(win, A_NORMAL | menu->wAttrs);

			cbSpace += cbLen;
		}
		else
			break;
	}
	if (menu->bLast != menu->bEnd)
		waddch_raw(win, '>');
	wclrtoeol(win);
	wattrset(win, wAttr);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES MENU INTERN: menu_process_menubar
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint8_t 
menu_process_menubar(MENU* menu)
{
	uint8_t bOptions = menu->bOptions;
	WINDOW* win		 = menu->win;

	switch (wgetch(win))
	{
		case KEY_LEFT:
			if (menu->bSelected == menu->bStart)
			{
				if (!(bOptions & MENU_OPTION_NONCYCLIC))
					menu->bSelected = menu->bEnd;
			}
			else
				menu->bSelected--;
			break;
			
		case KEY_RIGHT:
			if (menu->bSelected == menu->bEnd)
			{
				if (!(bOptions & MENU_OPTION_NONCYCLIC))
					menu->bSelected = menu->bStart;
			}
			else
				menu->bSelected++;
			break;
			
		case KEY_HOME:
			menu->bSelected = menu->bStart;
			break;
			
		case KEY_END:
			menu->bSelected = menu->bEnd;
			break;
			
		case KEY_PPAGE:
			if (menu->bFirst != menu->bStart)
				menu->bSelected = menu->bFirst-1;
			else
				menu->bSelected = menu->bStart;
			break;
			
		case KEY_NPAGE:
			if (menu->bLast != menu->bEnd)
				menu->bSelected = menu->bLast+1;
			else
				menu->bSelected = menu->bEnd;
			break;
			
		case KEY_CR:
			return MENU_PROCESS_SELECT_ITEM;
			break;

		case KEY_ESCAPE:
			return MENU_PROCESS_FINISHED;
			break;
			
		default:
			return MENU_PROCESS_CONTINUE;
			break;
	}
	
	menu_draw_menubar(menu);
	wmove_raw(win, win->bCury, win->bCurx);

	return MENU_PROCESS_CONTINUE;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES MENU: menu_driver
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
uint8_t 
menu_driver(MENU* amenu, uint8_t cMenuStates, uint8_t* pbMenuStateIndex)
{
	MENU*						menu;
	uint8_t						bReturn;
	uint16_t					wAttr;
	WINDOW*						win;
	const __flash MenuItem_t*	pMI;

	menu = &amenu[*pbMenuStateIndex];
	win  = menu->win;
	pMI  = &g_aMenuItem[menu->bSelected];
	

	if (MENU_SM_CONTINUE == menu->bState)
	{
		menu->bState = MENU_SM_PROCESS;
		bReturn		 = MENU_PROCESS_CONTINUE;
		switch (pMI->pFunc(pMI->bItemID, &menu->bData, MENU_FUNC_OPTION_SELECTED))
		{
			case MENU_FUNC_CLOSE_MENU:
				*pbMenuStateIndex = 0;
				menu_draw_menubar(&amenu[*pbMenuStateIndex]);
				break;
						
			case MENU_FUNC_KEEP_OPEN:
				if (MENU_ITEM_RADIO_BUTTON == pMI->bOptions)
					menu_draw_menubar(menu);
				break;
						
			case MENU_FUNC_EXIT_MENU_SYSTEM:
				*pbMenuStateIndex = 0;
				menu_draw_menubar(&amenu[*pbMenuStateIndex]);
				bReturn = MENU_PROCESS_FINISHED;
				break;
						
			case MENU_FUNC_CONTINUE:
				menu->bState = MENU_SM_CONTINUE;
				break;
		}

		return bReturn;
	}

	bReturn	= MENU_PROCESS_CONTINUE;
	switch(menu_process_menubar(menu))
	{
		case MENU_PROCESS_SELECT_ITEM:
			if (pMI->bSubMenuGroup > 0)
			{
				if (*pbMenuStateIndex < cMenuStates-1)
				{
					(*pbMenuStateIndex)++;
					init_menu(menu+1, pMI->bSubMenuGroup, menu->bRow, menu->bCol, menu->bOptions, menu->wAttrs);
					set_menu_window(menu+1, win);
					if (menu->bOptions & MENU_OPTION_SHOWPARENT)
					{
						wAttr = win->wAttrs;
						wattrset(win, menu->wAttrs);
						wmove_raw(win, menu->bRow, menu->bCol);
						(menu+1)->bCol = menu->bCol + 2 + strlen_P(pMI->pcText);
						waddstr_P_raw(win, pMI->pcText);
						waddstr_P_raw(win, PSTR(": "));
						wattrset(win, wAttr);
					}
					menu_draw_menubar(menu+1);
				}
				else
					bReturn = MENU_PROCESS_ERR_EXCEED_LEVELS;
					
				break;
			}

			if (0 != pMI->pFunc)
			{
				menu->bState = MENU_SM_CONTINUE;
				menu->bData  = 0;
			}
			else
			{
				*pbMenuStateIndex = 0;
				menu = &amenu[*pbMenuStateIndex];
				menu_draw_menubar(menu);
			}
			break;
			
		case MENU_PROCESS_FINISHED:
			menu_draw_menubar(amenu);
			*pbMenuStateIndex = 0;
			break;
	}
	
	return bReturn;
}
