/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * form.c - mcurses form lib
 *
 * Copyright (c) 2016 David Harmon
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */


#include "form.h"

#include <string.h>

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES FORM: Flash string constants defined in form-config.h
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
const __flash char FORM_STR_RADIO_CLEAR[]	= FORM_CONFIG_RADIO_CLEAR;
const __flash char FORM_STR_RADIO_CHECKED[]	= FORM_CONFIG_RADIO_CHECKED;
const __flash char FORM_STR_CB_CLEAR[]		= FORM_CONFIG_CB_CLEAR;
const __flash char FORM_STR_CB_CHECKED[]	= FORM_CONFIG_CB_CHECKED;

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES FORM: form_attrset
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void 
form_attrset(FORM* form, uint16_t wAttributes)
{
	if ((0xFF00 & wAttributes) == 0)
	{
		wAttributes |= (0xFF00 & form->wAttrs);									// Use default form colors
	}

	wattrset(form->win, wAttributes);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES FORM INTERN: form_draw_frame
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void 
form_draw_frame(FORM* form, const __flash char* pcHeader, uint8_t bOptions)
{
	uint8_t bLine;
	uint8_t i;

	uint8_t bRow	= form->bRow;
	uint8_t bCol	= form->bCol;
	uint8_t bWidth  = form->bWidth;
	uint8_t bHeight = form->bHeight;
	WINDOW* win     = form->win;

	wclrrec(win, bRow, bCol, bHeight, bWidth);
	wmove_raw(win, bRow, bCol);
	waddch_raw(win, '\016');												// switch to G1 set
	waddch_raw(win, ACS_ULCORNER_RAW);
	for (i = 0; i < bWidth - 2; i++)
		waddch_raw(win, ACS_HLINE_RAW);
	waddch_raw(win, ACS_URCORNER_RAW);

	if (pcHeader)
	{
		if (FORM_ITEM_CENTER & bOptions)
		{
			bLine = (bWidth/2-1) - strlen_P(pcHeader)/2;
		}
		else if (FORM_ITEM_RIGHT_JUSTIFY & bOptions)
		{
			bLine = (bWidth-3) - strlen_P(pcHeader);
		}
		else																// Left Justify
		{
			bLine = 1;
		}
		wmove_raw(win, bRow, bCol+bLine);
		waddch_raw(win, '\017');											// switch to G0 set
		waddstr_P_raw(win, pcHeader);
		waddch_raw(win, '\016');											// switch to G1 set
	}

	for (bLine = 1; bLine < bHeight - 1; bLine++)
	{
		wmove_raw(win, bLine + bRow, bCol);
		waddch_raw(win, ACS_VLINE_RAW);
		wmove_raw(win, bLine + bRow, bCol + bWidth - 1);
		waddch_raw(win, ACS_VLINE_RAW);
	}

	wmove_raw(win,  bRow + bHeight - 1, bCol);
	waddch_raw(win, ACS_LLCORNER_RAW);
	for (i = 0; i < bWidth - 2; i++)
	{
		waddch_raw(win, ACS_HLINE_RAW);
	}
	waddch_raw(win, ACS_LRCORNER_RAW);
	waddch_raw(win, '\017');												// switch to G0 set
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES FORM INTERN: form_draw_input
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void 
form_draw_input(FORM* form, const __flash FormItem_t* pFI, uint8_t bHasFocus)
{
	uint8_t bOffsetRow;
	uint8_t bOffsetCol;
	uint8_t bCury;
	uint8_t	bCurx;
	WINDOW* win = form->win;

	bOffsetRow = form->bRow + ((form->bOptions & FORM_OPTION_HAS_BORDER)?1:0);
	bOffsetCol = form->bCol + ((form->bOptions & FORM_OPTION_HAS_BORDER)?1:0);

	if (pFI->bType & FORM_ITEM_FIELD)
	{
		if (pFI->pFunc)
		{
			form->acBuff[0] = 0;
			if (pFI->pFunc(0, pFI->bItemID, FORM_FUNC_OPTION_DRAW, form->acBuff, sizeof(form->acBuff)) != FORM_FUNC_NO_CHANGE)
			{
				bCury = bOffsetRow+pFI->bRow;
				bCurx = bOffsetCol+pFI->bCol;
				
				curs_set(win, 0);
				wmove_raw(win, bCury, bCurx);
				if (form->bState & FORM_SM_SELECTED)
					form_attrset(form, pFI->wAttrs | A_REVERSE);
				else
					form_attrset(form, pFI->wAttrs);
				form->acBuff[pFI->bParam] = 0;
				waddstr_raw(win, form->acBuff);
				if (form->bState & FORM_SM_SELECTED)
					form_attrset(form, pFI->wAttrs);
				for (uint8_t j = strlen(form->acBuff); j < pFI->bParam; j++)
					waddch_raw(win, ' ');
				win->bCury = bCury;
				win->bCurx = bCurx;
			}
		}

		if (bHasFocus)
		{
			curs_set(win, 1);
		}
	}
	else if (pFI->bType & (FORM_ITEM_RADIO_BUTTON | FORM_ITEM_CHECK_BOX))
	{
		form_attrset(form, pFI->wAttrs);
		wmove_raw(win, bOffsetRow+pFI->bRow, bOffsetCol+pFI->bCol);

		if (bHasFocus)
			form_attrset(form, pFI->wAttrs | A_REVERSE);
		if (0 == pFI->pFunc)
		{
			waddstr_P_raw(win, (pFI->bType & FORM_ITEM_RADIO_BUTTON)?FORM_STR_RADIO_CLEAR:FORM_STR_CB_CLEAR);
		}
		else
		{
			waddstr_P_graphic_raw(win,
								  (pFI->pFunc(0, pFI->bItemID, FORM_FUNC_OPTION_DRAW, 0, 0) == FORM_FUNC_SELECTED)?
												(pFI->bType & FORM_ITEM_RADIO_BUTTON)?FORM_STR_RADIO_CHECKED:FORM_STR_CB_CHECKED:
												(pFI->bType & FORM_ITEM_RADIO_BUTTON)?FORM_STR_RADIO_CLEAR:FORM_STR_CB_CLEAR);
		}
	}
	else if (pFI->bType & FORM_ITEM_BUTTON)
	{
		form_attrset(form, pFI->wAttrs);
		wmove_raw(win, bOffsetRow+pFI->bRow, bOffsetCol+pFI->bCol);

		if (bHasFocus)
			form_attrset(form, pFI->wAttrs | A_BOLD);
		else
			form_attrset(form, pFI->wAttrs | A_REVERSE);
		waddstr_P_raw(win, pFI->pcText);
	}
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES FORM INTERN: form_draw_radio_buttons
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void 
form_draw_radio_buttons(FORM* form)
{
	uint8_t bOffsetRow;
	uint8_t bOffsetCol;
	uint8_t bParam;
	uint8_t bOptions = form->bOptions;
	WINDOW* win      = form->win;

	bOffsetRow	= form->bRow + ((bOptions & FORM_OPTION_HAS_BORDER)?1:0);
	bOffsetCol	= form->bCol + ((bOptions & FORM_OPTION_HAS_BORDER)?1:0);
	bParam		= g_aFormItem[form->bHasFocus].bParam;

	for (uint8_t i = form->bStart; i <= form->bEnd; i++)
	{
		if (!(FORM_ITEM_RADIO_BUTTON & g_aFormItem[i].bType))
			continue;

		if (bParam != g_aFormItem[i].bParam)
			continue;

		form_attrset(form, g_aFormItem[i].wAttrs);
		wmove_raw(win, bOffsetRow+g_aFormItem[i].bRow, bOffsetCol+g_aFormItem[i].bCol);
		if (form->bHasFocus == i)
			form_attrset(form, g_aFormItem[i].wAttrs | A_REVERSE);
		if (0 == g_aFormItem[i].pFunc)
		{
			waddstr_P_raw(win, FORM_STR_RADIO_CLEAR);
		}
		else
		{
			waddstr_P_graphic_raw(win,
								  (g_aFormItem[i].pFunc(0, g_aFormItem[i].bItemID, FORM_FUNC_OPTION_DRAW, 0, 0))?
									FORM_STR_RADIO_CHECKED:FORM_STR_RADIO_CLEAR);
		}
	}
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES FORM INTERN: form_draw_check_box
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void 
form_draw_check_box(FORM* form, const __flash FormItem_t* pFI)
{
	uint8_t bOffsetRow;
	uint8_t bOffsetCol;
	uint8_t bOptions = form->bOptions;
	WINDOW* win      = form->win;
	
	bOffsetRow = form->bRow + ((bOptions & FORM_OPTION_HAS_BORDER)?1:0);
	bOffsetCol = form->bCol + ((bOptions & FORM_OPTION_HAS_BORDER)?1:0);
	
	form_attrset(form, pFI->wAttrs | A_REVERSE);
	wmove_raw(win, bOffsetRow+pFI->bRow, bOffsetCol+pFI->bCol);
	waddstr_P_graphic_raw(win,
							(pFI->pFunc(0, pFI->bItemID, FORM_FUNC_OPTION_DRAW, 0, 0))?
								FORM_STR_CB_CHECKED:FORM_STR_CB_CLEAR);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES FORM: init_form
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void 
init_form(FORM* form, uint8_t bFormID, uint8_t bRow, uint8_t bCol, uint8_t bHeight, uint8_t bWidth, uint8_t bOptions, uint16_t wAttributes)
{
	uint8_t		bStart;
	uint8_t		bEnd;

	for (bStart = 0; FORM_ENTRY_END != g_aFormItem[bStart].bFormID; bStart++)
		if (bFormID == g_aFormItem[bStart].bFormID)
			break;
			
	for (bEnd = bStart+1; FORM_ENTRY_END != g_aFormItem[bEnd].bFormID; bEnd++)
		if (bFormID != g_aFormItem[bEnd].bFormID)
			break;

	bEnd--;

	form->bState	= FORM_SM_START;
	form->bFormID	= bFormID;
	form->bStart	= bStart;
	form->bHasFocus	= bStart;
	form->bEnd		= bEnd;
	form->bRow		= bRow;
	form->bCol		= bCol;
	form->bWidth	= bWidth;
	form->bHeight	= bHeight;
	form->bOptions	= bOptions;
	form->wAttrs	= wAttributes;
	form->win		= 0;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES FORM: set_form_window
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void 
set_form_window(FORM* form, WINDOW* win)
{
	form->win = win;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES FORM: post_form
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void 
post_form(FORM* form)
{
	uint8_t i;
	uint8_t bOffsetRow;
	uint8_t bOffsetCol;
	uint8_t bOptions = form->bOptions;;
	WINDOW* win	     = form->win;

	if (0 == win)
		return;
	
	wattrset(win, form->wAttrs);

	if (FORM_OPTION_HCENTER & bOptions)
	{
		form->bCol = win->bMaxx/2 - form->bWidth/2;
	}

	if (FORM_OPTION_VCENTER & bOptions)
	{
		form->bRow = win->bMaxy/2 - form->bHeight/2;
	}

	if (bOptions & FORM_OPTION_HAS_BORDER)
	{
		for (i = form->bStart; i <= form->bEnd; i++)
		{
			if (FORM_ITEMID_HEADER == g_aFormItem[i].bItemID)
			{
				form_draw_frame(form, g_aFormItem[i].pcText, g_aFormItem[i].bType);
				i = FORM_ENTRY_END;
				break;
			}
		}
		
		if (FORM_ENTRY_END != i)
			form_draw_frame(form, 0, 0);
	}

	bOffsetRow = form->bRow + ((bOptions & FORM_OPTION_HAS_BORDER)?1:0);
	bOffsetCol = form->bCol + ((bOptions & FORM_OPTION_HAS_BORDER)?1:0);
	for (i = form->bStart; i <= form->bEnd; i++)
	{
		if (FORM_ITEMID_HEADER == g_aFormItem[i].bItemID)
			continue;

		form_attrset(form, g_aFormItem[i].wAttrs);
		wmove_raw(win, bOffsetRow+g_aFormItem[i].bRow, bOffsetCol+g_aFormItem[i].bCol);
		if (g_aFormItem[i].bType & FORM_ITEM_FIELD)
		{
			form->acBuff[0] = 0;
			if (g_aFormItem[i].pFunc)
			{
				g_aFormItem[i].pFunc(0, g_aFormItem[i].bItemID, FORM_FUNC_OPTION_INIT, form->acBuff, sizeof(form->acBuff));
				form->acBuff[g_aFormItem[i].bParam] = 0;
				waddstr_raw(win, form->acBuff);
			}
			for (uint8_t j = strlen(form->acBuff); j < g_aFormItem[i].bParam; j++)
				waddch_raw(win, ' ');
		}
		else if (g_aFormItem[i].bType & (FORM_ITEM_RADIO_BUTTON | FORM_ITEM_CHECK_BOX))
		{
			if (0 == g_aFormItem[i].pFunc)
			{
				waddstr_P_raw(win, (g_aFormItem[i].bType & FORM_ITEM_RADIO_BUTTON)?FORM_STR_RADIO_CLEAR:FORM_STR_CB_CLEAR);
			}
			else
			{
				waddstr_P_graphic_raw(win,
									  (g_aFormItem[i].pFunc(0, g_aFormItem[i].bItemID, FORM_FUNC_OPTION_INIT, 0, 0) == FORM_FUNC_SELECTED)?
										(g_aFormItem[i].bType & FORM_ITEM_RADIO_BUTTON)?FORM_STR_RADIO_CHECKED:FORM_STR_CB_CHECKED:
										(g_aFormItem[i].bType & FORM_ITEM_RADIO_BUTTON)?FORM_STR_RADIO_CLEAR:FORM_STR_CB_CLEAR);
			}
		}
		else if (g_aFormItem[i].bType & FORM_ITEM_BUTTON)
		{
			form_attrset(form, g_aFormItem[i].wAttrs | A_REVERSE);
			waddstr_P_raw(win, g_aFormItem[i].pcText);
		}
		else	// Default type is FORM_ITEM_LABEL
		{
			waddstr_P_raw(win, g_aFormItem[i].pcText);
		}
	}
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES FORM: unpost_form
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void 
unpost_form(FORM* form)
{
	form->win = 0;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES FORM: form_driver
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
uint8_t 
form_driver(FORM* form)
{
	uint8_t bChar;
	uint8_t bReturn;
	WINDOW* win = form->win;
	
	const __flash FormItem_t* pFI = &g_aFormItem[form->bHasFocus];

	switch (form->bState & ~FORM_SM_SELECTED)
	{
		case FORM_SM_HAS_FOCUS:
			if (FORM_ITEM_FIELD & pFI->bType)
			{
				bChar = pFI->pFunc(win, pFI->bItemID, FORM_FUNC_OPTION_GETCH, form->acBuff, pFI->bParam+1);
			}
			else
			{
				bChar = wgetch(win);
			}
			switch (bChar)
			{
				case KEY_CR:
					if (FORM_ITEM_BUTTON & pFI->bType)
					{
						bReturn = pFI->pFunc(0, pFI->bItemID, FORM_FUNC_OPTION_SELECTED, 0, 0);
						if (bReturn & (FORM_FUNC_COMMIT_FORM | FORM_FUNC_CANCEL_FORM))
						{
							bReturn = (bReturn & FORM_FUNC_COMMIT_FORM)?FORM_FUNC_OPTION_COMMIT:FORM_FUNC_OPTION_CANCEL;
							for (uint8_t i = form->bStart; i <= form->bEnd; i++)
								if (g_aFormItem[i].pFunc)
									g_aFormItem[i].pFunc(0, g_aFormItem[i].bItemID, bReturn, 0, 0);
							return FORM_DRIVER_FINISHED;
						}
					}
					#if FORM_CONFIG_USE_CR_AS_TAB > 0
					else if (FORM_FUNC_KEEP_FOCUS != pFI->pFunc(win, pFI->bItemID, FORM_FUNC_OPTION_LOSE_FOCUS, form->acBuff, pFI->bParam+1))
					{
						form->bState = FORM_SM_NEXT_FOCUS;
						form_draw_input(form, pFI, 0);
					}
					#endif // FORM_CONFIG_USE_CR_AS_TAB > 0
					break;					

				case KEY_TAB:
				case KEY_BTAB:
					if (FORM_FUNC_KEEP_FOCUS != pFI->pFunc(win, pFI->bItemID, FORM_FUNC_OPTION_LOSE_FOCUS, form->acBuff, pFI->bParam+1))
					{
						if (KEY_TAB == bChar)
							form->bState = FORM_SM_NEXT_FOCUS;
						else
							form->bState = FORM_SM_PREV_FOCUS;
						form_draw_input(form, pFI, 0);
					}
					break;

				case ' ':
					if (FORM_ITEM_CHECK_BOX & pFI->bType)
					{
						pFI->pFunc(0, pFI->bItemID, FORM_FUNC_OPTION_SELECTED, 0, 0);
						form_draw_check_box(form, pFI);
					}
					else if (FORM_ITEM_RADIO_BUTTON & pFI->bType)
					{
						if (pFI->pFunc(0, pFI->bItemID, FORM_FUNC_OPTION_SELECTED, 0, 0) == FORM_FUNC_SELECTED)
							form_draw_radio_buttons(form);
					}
					break;

				case KEY_ESCAPE:
					for (uint8_t i = form->bStart; i <= form->bEnd; i++)
						if (g_aFormItem[i].pFunc)
							g_aFormItem[i].pFunc(0, g_aFormItem[i].bItemID, FORM_FUNC_OPTION_CANCEL, 0, 0);
 					if (FORM_ITEM_FIELD & pFI->bType)
 						curs_set(win, 0);
					return FORM_DRIVER_FINISHED;
					break;
			}
			break;
		
		case FORM_SM_NEXT_FOCUS:
		case FORM_SM_PREV_FOCUS:
			if (FORM_SM_NEXT_FOCUS == form->bState)
			{
				if (form->bHasFocus == form->bEnd)
					form->bHasFocus = form->bStart;
				else
					form->bHasFocus++;
			}
			else
			{
				if (form->bHasFocus == form->bStart)
					form->bHasFocus = form->bEnd;
				else
					form->bHasFocus--;
			}

			pFI = &g_aFormItem[form->bHasFocus];				
			if (FORM_ITEM_INPUT & pFI->bType)
			{
				if (pFI->pFunc)
					if (FORM_FUNC_REFUSE_FOCUS != pFI->pFunc(0, pFI->bItemID, FORM_FUNC_OPTION_HAS_FOCUS, 0, 0))
					{
						form->bState = FORM_SM_HAS_FOCUS | FORM_SM_SELECTED;
						form_draw_input(form, pFI, 1);
					}
			}
			break;

		case FORM_SM_START:
			post_form(form);
			form->bHasFocus	= form->bEnd;
			form->bState	= FORM_SM_NEXT_FOCUS;
			break;
	}
	
	return FORM_DRIVER_CONTINUE;
}
