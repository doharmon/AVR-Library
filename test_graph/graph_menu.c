/*
 * graph_menu.c
 *
 * Created: 5/4/2017 7:37:01 PM
 *  Author: David
 */ 

#include "graph_menu.h"
#include "graph_form.h"

extern WINDOW* win;

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Menu structures
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */

const __flash char MT_ABOUT[]					= "About";
const __flash char MT_PLOT[]					= "Plot";
const __flash char MT_PLOT_DRAW[]				= "Draw";
const __flash char MT_PLOT_EQUATIONS[]			= "Equation";
const __flash char MT_PLOT_SETTINGS[]			= "Settings";
const __flash char MT_PLOT_LOADDEFAULTS[]		= "Load Defaults";

uint8_t menu_about(uint8_t bID, uint8_t* pbData, uint8_t bOptions);
uint8_t menu_plot(uint8_t bID, uint8_t* pbData, uint8_t bOptions);
uint8_t plot_equation(uint8_t bID, uint8_t* pbData, uint8_t bOptions);

MENU g_amenu[MENU_MAXDEPTH];

const __flash MenuItem_t g_aMenuItem[] =
{
//																			Sub
//	  MenuID,			ItemID,					Options,					MenuID,		pFunc,			pcText
	{ MID_MAIN,			IID_PLOT,				0,							MID_PLOT,	0,				MT_PLOT					},
	{ MID_MAIN,			IID_ABOUT,				0,							0,			menu_about,		MT_ABOUT				},

	{ MID_PLOT,			IID_PLOT_DRAW,			0,							0,			menu_plot,		MT_PLOT_DRAW			},
	{ MID_PLOT,			IID_PLOT_EQUATIONS,		0,							0,			plot_equation,	MT_PLOT_EQUATIONS		},
//	{ MID_PLOT,			IID_PLOT_SETTINGS,		0,							0,			menu_plot,		MT_PLOT_SETTINGS		},
//	{ MID_PLOT,			IID_PLOT_LOADDEFAULTS,	0,							0,			menu_plot,		MT_PLOT_LOADDEFAULTS	},

	{ MENU_ENTRY_END,	0,						0,							0,			0,				0						}
};

uint8_t menu_about(uint8_t bID, uint8_t* pbData, uint8_t bOptions)
{
	static uint8_t b = 0;

	if (0 == b)
	{
		wattrset(win, F_WHITE | B_BLACK);
		init_form(form, FID_ABOUT, 5, 5, 14, 31, FORM_OPTION_HAS_BORDER | FORM_OPTION_HCENTER, F_WHITE | B_BLACK);
		set_form_window(form, win);
		b = 1;

		return MENU_FUNC_CONTINUE;
	}
	else
	{
		if (form_driver(form) == FORM_DRIVER_CONTINUE)
			return MENU_FUNC_CONTINUE;
	}

	b = 0;
	wclear(win);

	return MENU_FUNC_CLOSE_MENU;
}

uint8_t menu_plot(uint8_t bID, uint8_t* pbData, uint8_t bOptions)
{
	switch (bID)
	{
		case IID_PLOT_DRAW:
			return MENU_FUNC_EXIT_MENU_SYSTEM;

	}
	
	return MENU_FUNC_CLOSE_MENU;
}

uint8_t plot_equation(uint8_t bID, uint8_t* pbData, uint8_t bOptions)
{
	static uint8_t b = 0;

	if (0 == b)
	{
		wattrset(win, F_WHITE | B_BLACK);
		init_form(form, FID_EQUATION, 0, 0, 8, 79, FORM_OPTION_HAS_BORDER | FORM_OPTION_HCENTER | FORM_OPTION_VCENTER, F_WHITE | B_BLACK);
		set_form_window(form, win);
		b = 1;

		return MENU_FUNC_CONTINUE;
	}
	else
	{
		if (form_driver(form) == FORM_DRIVER_CONTINUE)
			return MENU_FUNC_CONTINUE;
	}

	b = 0;
	wclear(win);

	return MENU_FUNC_CLOSE_MENU;
}

