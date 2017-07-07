/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * @file test_graph.c
 *
 * Copyright (c) 2017 David Harmon
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */

#include "../source/lib_mcurses/mcurses.h"
#include "../source/lib_mcurses/menu.h"
#include "../source/lib_mcurses/form.h"
#include "../source/lib_mcurses/statusline.h"
#include "../source/lib_xio/xio.h"
#include "../source/lib_gccfix/gccfix.h"
#include "../source/lib_timer/Timer.h"

#include "graph_menu.h"
#include "graph_form.h"

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Defines and global variables
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define UPTIME_WIDTH			(21)
#define MENU_ATTRIBUTES			(F_BLUE | B_WHITE)

char		g_sz[100];
uint8_t		g_cbField;
uint16_t	g_dwAttrs =			(F_WHITE | B_BLACK);

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Mcurses structures
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
SCREEN		SCR,  *scr  = &SCR;
WINDOW		WIN,  *win  = &WIN;
STATUSLINE	STAT, *stat = &STAT;

const __flash TERM g_aterm[] =
{
	{	mcurses_phyio_putc, mcurses_phyio_getbyte, mcurses_phyio_available, mcurses_phyio_flushoutput	}
};

void updateStatus(void)
{
	static unsigned long ulMinutes;
	static unsigned long ulPrev    = -1;

	ulMinutes = millis()/60000;
	if (ulPrev != ulMinutes)
	{
		ulPrev = ulMinutes;
		xsnprintf_P(g_sz, sizeof(g_sz), PSTR("Uptime (min): %7ld"), ulMinutes);
		statusline_draw(stat,
						g_sz, 
						UPTIME_WIDTH, 
						0, 
						SL_OPTION_FIELD_JUSTIFY_RIGHT);
	}
}

uint8_t graph(WINDOW* win, uint8_t bState);

__attribute__((OS_main)) int main(void)
{
	uint8_t	err;
	uint8_t imenu = 0;

	sei();
	initTimer();
	err = eqeval_parse(g_szEquation);
	if (err != EQEV_E_OK)
	{
		wmove(win, 10, 0);
		wprintw_P(win, PSTR("Internal error with eqeval library: %S"), eqeval_get_err_msg(err));
		while (1)
			;
	}

	newterm(0, scr, win);
	mcurses_term_reset(win);

	curs_set(win, FALSE);
	wprintw_P(win, PSTR("Waiting for response from terminal\r\nMake sure terminal TX is connected to chip"));
	while (scr->bFlags & SCR_FLAG_QUERY_SIZE)
		wgetch(win);

	mcurses_term_scrreg(win, TRUE);
	wattrset(win, g_dwAttrs);
	wclear(win);
	
	if (win->bMaxx < sizeof(g_sz))
		g_cbField = win->bMaxx - UPTIME_WIDTH - 2;
	else
		g_cbField = sizeof(g_sz) - UPTIME_WIDTH - 2;
	

	init_menu(g_amenu, MID_MAIN, 0, 0, MENU_OPTION_MENUBAR | MENU_OPTION_SHOWPARENT, MENU_ATTRIBUTES);
	set_menu_window(g_amenu, win);
	post_menu(g_amenu);

	init_statusline(stat, -1, 0, -1, MENU_ATTRIBUTES);
	set_statusline_window(stat, win);
	post_statusline(stat);

	while(1)
	{
		while (menu_driver(g_amenu, MENU_MAXDEPTH, &imenu) == MENU_PROCESS_CONTINUE)
		{
			updateStatus();
		}
		
		strncpy_P(g_sz, PSTR(" h:Help q:Quite w:View Window t:Trace r:reset s:Slope Char -/=:Zoom in/out"), g_cbField);
		statusline_draw(stat,
						g_sz, 
						g_cbField, 
						0, 
						SL_OPTION_FIELD_JUSTIFY_LEFT);
		graph(win, 0);

		while (1)
		{
			updateStatus();
			if (graph(win, 1))
				break;
		}

		wattrset(win, MENU_ATTRIBUTES);
		wclrrec(win, win->bMaxy, 0, 1, g_cbField);
		wattrset(win, g_dwAttrs);
		wclear(win);
	}
}
