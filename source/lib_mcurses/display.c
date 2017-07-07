/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * display.c - mcurses form lib
 *
 * Copyright (c) 2016 David Harmon
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */


#include "display.h"

#include <string.h>

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES DISPLAY: Flash string constants defined in form-config.h
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
extern const __flash char FORM_STR_RADIO_CLEAR[];
extern const __flash char FORM_STR_RADIO_CHECKED[];
extern const __flash char FORM_STR_CB_CLEAR[];
extern const __flash char FORM_STR_CB_CHECKED[];

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES DISPLAY INTERNAL: display_draw_input
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint8_t 
display_draw_input(FORM* form, const __flash FormItem_t* pFI, uint8_t bOptions)
{
	uint8_t		bHasBorder;
	uint8_t		bRow, bCol;
	uint16_t	wAttrs;
	WINDOW*		win = form->win;
	
	if (pFI->pFunc)
	{
		if (pFI->pFunc(win, pFI->bItemID, FORM_FUNC_OPTION_QUERY_CHANGED, form->acBuff, sizeof(form->acBuff)) == FORM_FUNC_NO_CHANGE)
			return 0;

		wAttrs = win->wAttrs;
		form_attrset(form, pFI->wAttrs);
		getyx(win, bRow, bCol);
		curs_set(win, 0);
		bHasBorder = ((bOptions & FORM_OPTION_HAS_BORDER)?1:0);
		wmove_raw(win, form->bRow + bHasBorder + pFI->bRow, form->bCol + bHasBorder + pFI->bCol);

		if (pFI->bType & FORM_ITEM_FIELD)
		{
			form->acBuff[pFI->bParam] = 0;
			waddstr_raw(win, form->acBuff);
			for (uint8_t j = strlen(form->acBuff); j < pFI->bParam; j++)
				waddch_raw(win, ' ');
		}
		else if (pFI->bType & (FORM_ITEM_RADIO_BUTTON | FORM_ITEM_CHECK_BOX))
		{
			waddstr_P_graphic_raw(win,
								  (pFI->pFunc(0, pFI->bItemID, FORM_FUNC_OPTION_DRAW, 0, 0) == FORM_FUNC_SELECTED)?
									(pFI->bType & FORM_ITEM_RADIO_BUTTON)?FORM_STR_RADIO_CHECKED:FORM_STR_CB_CHECKED:
									(pFI->bType & FORM_ITEM_RADIO_BUTTON)?FORM_STR_RADIO_CLEAR:FORM_STR_CB_CLEAR);
		}

		wattrset(win, wAttrs);
		wmove_raw(win, bRow, bCol);
		curs_set(win, 1);

		return 1;
	}

	return 0;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES DISPLAY: display_driver
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void 
display_driver(FORM* form)
{
	uint8_t		bRow, bCol;
	uint16_t	wAttrs;
	uint8_t		b;
	uint8_t		bOptions = form->bOptions;
	WINDOW*		win		 = form->win;
	
	switch (form->bState)
	{
		case FORM_SM_NEXT_FOCUS:
			b = form->bHasFocus + 1;
			
			if (b > form->bEnd)
				b = form->bStart;
				
 			if ((FORM_ITEM_INPUT & ~FORM_ITEM_BUTTON) & g_aFormItem[b].bType)	// Do not display buttons
 				display_draw_input(form, &g_aFormItem[b], bOptions);

			form->bHasFocus = b;
			break;

		case FORM_SM_START:
			if (FORM_OPTION_HCENTER & bOptions)
			{
				form->bCol = win->bMaxx/2 - form->bWidth/2;
			}
			if (FORM_OPTION_VCENTER & bOptions)
			{
				form->bRow = win->bMaxy/2 - form->bHeight/2;
			}
			getyx(win, bRow, bCol);
			wAttrs = win->wAttrs;
			curs_set(win, 0);
			post_form(form);
			form->bState = FORM_SM_NEXT_FOCUS;
			wmove_raw(win, bRow, bCol);
			wattrset(win, wAttrs);
			curs_set(win, 1);
			break;
	}
}
