/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * @file test_mcurses.c based on demo.c - demo program using mcurses lib
 *
 * Copyright (c) 2011-2014 Frank Meyer - frank(at)fli4l.de
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
#include "../source/lib_mcurses/display.h"
#include "../source/lib_mcurses/cli.h"
#include "../source/lib_mcurses/statusline.h"

#include "../source/lib_xio/xio.h"

#include "../source/lib_timer/Timer.h"

#include <util/delay.h>
#include <stdlib.h>
#include <string.h>

char    buf[80];

struct
{
	long				i;
	long				ui;
	long				hex;
	_Accum				k;
	unsigned _Accum		uk;
	char				text[41];
	uint8_t				bSelected;
	uint8_t				abSelected[3];
} data;

#define         PAUSE(x)                        { wrefresh(&win); if (!fast) _delay_ms (x); }
#define         myitoa(x,buf)                   itoa ((x), buf, 10)

static uint8_t  fast = 0;

static uint8_t  hanoi_pole_height[3];
static uint8_t  hanoi_number_of_rings;

static uint16_t g_wWinAttr;
static uint8_t  g_bMenuOptions;

static SCREEN scr;
static WINDOW win;
static STATUSLINE stat;

// Menu Text
const __flash char MT_DEMO[]					= "Demo";
const __flash char MT_MENU[]					= "Menu";
const __flash char MT_FORM[]					= "Form Test";
const __flash char MT_GETCH[]					= "Getch";
const __flash char MT_DISPLAY[]					= "Display";
const __flash char MT_EXIT[]					= "Exit";
const __flash char MT_DEMO_CLEAR[]				= "Clear";
const __flash char MT_DEMO_SCREEN[]				= "Screen";
const __flash char MT_DEMO_HANOI[]				= "Hanoi";
const __flash char MT_DEMO_TEMP[]				= "Temp";
const __flash char MT_DEMO_DUMP[]				= "Dump";
const __flash char MT_DEMO_KEYS[]				= "Keys";
const __flash char MT_DEMO_STATE[]				= "State";
const __flash char MT_DEMO_CLI[]				= "Command Line Interface";
const __flash char MT_MENU_CYCLIC[]				= "Cyclic";
const __flash char MT_MENU_PARENT[]				= "Show Parent";
const __flash char MT_MENU_FOREGROUND[]			= "Foreground";
const __flash char MT_MENU_BACKGROUND[]			= "Background";
const __flash char MT_FBGROUND_BLACK[]			= "Black";
const __flash char MT_FBGROUND_RED[]			= "Red";
const __flash char MT_FBGROUND_GREEN[]			= "Green";
const __flash char MT_FBGROUND_YELLOW[]			= "Yellow";
const __flash char MT_FBGROUND_BLUE[]			= "Blue";
const __flash char MT_FBGROUND_MAGENTA[]		= "Magenta";
const __flash char MT_FBGROUND_CYAN[]			= "Cyan";
const __flash char MT_FBGROUND_WHITE[]			= "White";

//const __flash char MT_[]= "";

uint8_t menu_demo(uint8_t bID, uint8_t* pbData, uint8_t bOptions);
uint8_t menu_menu(uint8_t bID, uint8_t* pbData, uint8_t bOptions);
uint8_t menu_color(uint8_t bID, uint8_t* pbData, uint8_t bOptions);
uint8_t menu_form(uint8_t bID, uint8_t* pbData, uint8_t bOptions);
uint8_t menu_getch(uint8_t bID, uint8_t* pbData, uint8_t bOptions);
uint8_t menu_display(uint8_t bID, uint8_t* pbData, uint8_t bOptions);
uint8_t menu_exit(uint8_t bID, uint8_t* pbData, uint8_t bOptions);

const __flash MenuItem_t g_aMenuItem[] =
{
//													 Sub
//	  MenuID,	ItemID, Options,					 MenuID,pFunc,			pcText
	{ 0,		 1,		0,							 1,		NULL,			MT_DEMO },
	{ 0,		 2,		0,							 2,		NULL,			MT_MENU },
	{ 0,		 2,		0,							 0,		menu_form,		MT_FORM },
	{ 0,		 2,		0,							 0,		menu_getch,		MT_GETCH },
	{ 0,		 2,		0,							 0,		menu_display,	MT_DISPLAY },
	{ 0,		99,		0,							 0,		menu_exit,		MT_EXIT },

	{ 1,		13,		0,							 0,		menu_demo,		MT_DEMO_CLEAR },
	{ 1,		14,		0,							 0,		menu_demo,		MT_DEMO_SCREEN },
	{ 1,		15,		0,							 0,		menu_demo,		MT_DEMO_HANOI },
	{ 1,		16,		0,							 0,		menu_demo,		MT_DEMO_TEMP },
	{ 1,		17,		0,							 0,		menu_demo,		MT_DEMO_DUMP },
	{ 1,		18,		0,							 0,		menu_demo,		MT_DEMO_KEYS },
	{ 1,		19,		0,							 0,		menu_demo,		MT_DEMO_STATE },
	{ 1,		20,		0,							 0,		menu_demo,		MT_DEMO_CLI },

	{ 2,		18,		MENU_ITEM_RADIO_BUTTON,		 0,		menu_menu,		MT_MENU_CYCLIC },
	{ 2,		19,		MENU_ITEM_RADIO_BUTTON,		 0,		menu_menu,		MT_MENU_PARENT },
	{ 2,		21,		0,							13,		NULL,			MT_MENU_FOREGROUND },
	{ 2,		22,		0,							14,		NULL,			MT_MENU_BACKGROUND },

	{ 13,		58,		MENU_ITEM_RADIO_BUTTON,		 0,		menu_color,		MT_FBGROUND_BLACK },
	{ 13,		59,		MENU_ITEM_RADIO_BUTTON,		 0,		menu_color,		MT_FBGROUND_RED },
	{ 13,		60,		MENU_ITEM_RADIO_BUTTON,		 0,		menu_color,		MT_FBGROUND_GREEN },
	{ 13,		61,		MENU_ITEM_RADIO_BUTTON,		 0,		menu_color,		MT_FBGROUND_YELLOW },
	{ 13,		62,		MENU_ITEM_RADIO_BUTTON,		 0,		menu_color,		MT_FBGROUND_BLUE },
	{ 13,		63,		MENU_ITEM_RADIO_BUTTON,		 0,		menu_color,		MT_FBGROUND_MAGENTA },
	{ 13,		64,		MENU_ITEM_RADIO_BUTTON,		 0,		menu_color,		MT_FBGROUND_CYAN },
	{ 13,		65,		MENU_ITEM_RADIO_BUTTON,		 0,		menu_color,		MT_FBGROUND_WHITE },

	{ 14,		66,		MENU_ITEM_RADIO_BUTTON,		 0,		menu_color,		MT_FBGROUND_BLACK },
	{ 14,		67,		MENU_ITEM_RADIO_BUTTON,		 0,		menu_color,		MT_FBGROUND_RED },
	{ 14,		68,		MENU_ITEM_RADIO_BUTTON,		 0,		menu_color,		MT_FBGROUND_GREEN },
	{ 14,		69,		MENU_ITEM_RADIO_BUTTON,		 0,		menu_color,		MT_FBGROUND_YELLOW },
	{ 14,		70,		MENU_ITEM_RADIO_BUTTON,		 0,		menu_color,		MT_FBGROUND_BLUE },
	{ 14,		71,		MENU_ITEM_RADIO_BUTTON,		 0,		menu_color,		MT_FBGROUND_MAGENTA },
	{ 14,		72,		MENU_ITEM_RADIO_BUTTON,		 0,		menu_color,		MT_FBGROUND_CYAN },
	{ 14,		73,		MENU_ITEM_RADIO_BUTTON,		 0,		menu_color,		MT_FBGROUND_WHITE },

	{ MENU_ENTRY_END,	0,	0,	0,	0,	0 }
};

MENU g_amenu[3];

// Form Label
const __flash char FL_HEADER[]					= "Test Form";
const __flash char FL_INT[]						= "Integer:";
const __flash char FL_POS_INT[]					= "Pos Int:";
const __flash char FL_HEX[]						= "Hex:";
const __flash char FL_FLOAT[]					= "Float:";
const __flash char FL_POS_FLOAT[]				= "Pos Float:";
const __flash char FL_TEXT[]					= "Text:";
const __flash char FL_RADIO_SMALL[]				= "Small";
const __flash char FL_RADIO_MEDIUM[]			= "Medium";
const __flash char FL_RADIO_LARGE[]				= "Large";
const __flash char FL_CHECK_1[]					= "Check Box 1";
const __flash char FL_CHECK_2[]					= "Check Box 2";
const __flash char FL_CHECK_3[]					= "Check Box 3";
const __flash char FL_OK[]						= " OK ";
const __flash char FL_CANCEL[]					= " Cancel ";

uint8_t form_field(WINDOW* win, uint8_t bID, uint8_t bOptions, char* pcBuff, uint8_t bBuffLen);
uint8_t form_radio(WINDOW* win, uint8_t bID, uint8_t bOptions, char* pcBuff, uint8_t bBuffLen);
uint8_t form_check(WINDOW* win, uint8_t bID, uint8_t bOptions, char* pcBuff, uint8_t bBuffLen);
uint8_t form_button(WINDOW* win, uint8_t bID, uint8_t bOptions, char* pcBuff, uint8_t bBuffLen);

const __flash FormItem_t g_aFormItem[] =
{
	//																			Length/
	//																			Group
	// FormID,	ItemID,				Type,						Row, 	Col,	Param,	Attribute,		pFunc,			pcText
	{  0,		FORM_ITEMID_HEADER,	FORM_ITEM_CENTER,			0,		0,		0,		0,				NULL,			FL_HEADER },
	{  0,		0,					FORM_ITEM_LABEL,			0,		3,		0,		0,				NULL,			FL_INT },
	{  0,		0,					FORM_ITEM_LABEL,			1,		3,		0,		0,				NULL,			FL_POS_INT },
	{  0,		0,					FORM_ITEM_LABEL,			2,		7,		0,		0,				NULL,			FL_HEX },
	{  0,		0,					FORM_ITEM_LABEL,			3,		5,		0,		0,				NULL,			FL_FLOAT },
	{  0,		0,					FORM_ITEM_LABEL,			4,		1,		0,		0,				NULL,			FL_POS_FLOAT },
	{  0,		0,					FORM_ITEM_LABEL,			5,		5,		0,		0,				NULL,			FL_TEXT },
	{  0,		0,					FORM_ITEM_LABEL,			7,		5,		0,		0,				NULL,			FL_RADIO_SMALL },
	{  0,		0,					FORM_ITEM_LABEL,			8,		5,		0,		0,				NULL,			FL_RADIO_MEDIUM },
	{  0,		0,					FORM_ITEM_LABEL,			9,		5,		0,		0,				NULL,			FL_RADIO_LARGE },
	{  0,		0,					FORM_ITEM_LABEL,			7,		31,		0,		0,				NULL,			FL_CHECK_1 },
	{  0,		0,					FORM_ITEM_LABEL,			8,		31,		0,		0,				NULL,			FL_CHECK_2 },
	{  0,		0,					FORM_ITEM_LABEL,			9,		31,		0,		0,				NULL,			FL_CHECK_3 },
	{  0,		0,					FORM_ITEM_FIELD,			0,		12,		4,		A_UNDERLINE,	form_field,		NULL },
	{  0,		1,					FORM_ITEM_FIELD,			1,		12,		4,		A_UNDERLINE,	form_field,		NULL },
	{  0,		2,					FORM_ITEM_FIELD,			2,		12,		4,		A_UNDERLINE,	form_field,		NULL },
	{  0,		3,					FORM_ITEM_FIELD,			3,		12,		10,		A_UNDERLINE,	form_field,		NULL },
	{  0,		4,					FORM_ITEM_FIELD,			4,		12,		10,		A_UNDERLINE,	form_field,		NULL },
	{  0,		5,					FORM_ITEM_FIELD,			5,		12,		40,		A_UNDERLINE,	form_field,		NULL },
	{  0,		0,					FORM_ITEM_RADIO_BUTTON,		7,		1,		0,		0,				form_radio,		NULL },
	{  0,		1,					FORM_ITEM_RADIO_BUTTON,		8,		1,		0,		0,				form_radio,		NULL },
	{  0,		2,					FORM_ITEM_RADIO_BUTTON,		9,		1,		0,		0,				form_radio,		NULL },
	{  0,		0,					FORM_ITEM_CHECK_BOX,		7,		27,		0,		0,				form_check,		NULL },
	{  0,		1,					FORM_ITEM_CHECK_BOX,		8,		27,		0,		0,				form_check,		NULL },
	{  0,		2,					FORM_ITEM_CHECK_BOX,		9,		27,		0,		0,				form_check,		NULL },
	{  0,		0,					FORM_ITEM_BUTTON,			11,		17,		0,		0,				form_button,	FL_OK },
	{  0,		1,					FORM_ITEM_BUTTON,			11,		27,		0,		0,				form_button,	FL_CANCEL },

	{  FORM_ENTRY_END,	0,	0,	0,	0,	0,	0,	0,	0 }
};

// set_position: set position on pole
static void
hanoi_set_position (uint8_t pole, uint8_t ring)
{
    uint8_t column;
    uint8_t line;

    column = pole * 20 - ring - 3;
    line = hanoi_number_of_rings + 5 - hanoi_pole_height[pole - 1];
    wmove (&win, line, column);
}

// erase ring on pole
static void
hanoi_erase_ring (uint8_t ring, uint8_t pole)
{
    uint8_t i;
    hanoi_set_position (pole, ring + 1);
    hanoi_pole_height[pole - 1]--;

    i = ring + 3;

    while (i--)
    {
        waddch (&win, ' ');
    }

    if (hanoi_pole_height[pole - 1] - 1 == hanoi_number_of_rings)
    {
        waddch (&win, ' ');
    }
    else
    {
        waddch (&win, '|');
    }

    i = ring + 3;

    while (i--)
    {
        waddch (&win, ' ');
    }
}

// draw ring on pole
static void
hanoi_draw_ring (uint8_t ring, uint8_t pole)
{
    uint8_t i;

    hanoi_pole_height[pole - 1]++;
    hanoi_set_position (pole, ring);

    i = 2 * ring + 5;

    while (i--)
    {
        waddch (&win, '-');
    }
}


// wmove ring from pole to pole
static void
hanoi_move_ring (uint8_t ring, uint8_t from_pole, uint8_t to_pole)
{
    uint8_t height[3];
    uint8_t h;
    uint8_t i;

    height[0] = hanoi_pole_height[0];
    height[1] = hanoi_pole_height[1];
    height[2] = hanoi_pole_height[2];

    h = hanoi_number_of_rings - height[from_pole - 1] + 2;

    while (h)
    {
        hanoi_erase_ring (ring, from_pole);
        wmove (&win, 3, 0);
        PAUSE (100/10);
        hanoi_pole_height[from_pole-1]++;
        hanoi_draw_ring (ring, from_pole);
        wmove (&win, 3, 0);
        PAUSE (100/10);
        h--;
    }

    if (from_pole > to_pole)
    {
        for (i = 0; i < 20 * (from_pole - to_pole); i++)
        {
            mvwdelch (&win, 3,0);
            PAUSE (25/10);
        }
    }
    else
    {
        for (i = 0; i < 20 * (to_pole - from_pole); i++)
        {
            mvwinsch (&win, 3, 0, ' ');
            PAUSE (25/10);
        }
    }

    i = hanoi_number_of_rings - height[to_pole - 1] + 1;
    hanoi_pole_height[to_pole - 1] = hanoi_number_of_rings + 1;

    while (i)
    {
        hanoi_draw_ring (ring, to_pole);
        wmove (&win, 3, 0);
        PAUSE (100/10);
        hanoi_erase_ring (ring, to_pole);
        wmove (&win, 3, 0);
        PAUSE (100/10);
        hanoi_pole_height[to_pole - 1]--;
        i--;
    }

    hanoi_draw_ring (ring, to_pole);
    wmove (&win, 3, 0);
    PAUSE (100/10);

    hanoi_pole_height[from_pole - 1] = height[from_pole - 1] - 1;
    hanoi_pole_height[to_pole   - 1] = height[to_pole   - 1] + 1;
}

static void
hanoi (uint8_t nrings, uint8_t n1, uint8_t n2, uint8_t n3)
{
    uint8_t n;

    if (nrings == 0)
    {
        return;
    }

    n = nrings - 1;
    hanoi (n, n1, n3, n2);
    hanoi_move_ring (n, n1, n2);
    hanoi (n, n3, n2, n1);
}

static void
hanoi_draw_poles (void)
{
    uint8_t ring;
    uint8_t height_poles;
    uint8_t h;
    uint8_t i;
    uint8_t j;

    ring = hanoi_number_of_rings;
    height_poles = 1 + hanoi_number_of_rings;
    h = height_poles;

    while (h--)
    {
        for (i = 0; i < 3; i++)
        {
            mvwaddch (&win, h + 4, i * 20 + 19, '|');
        }
    }

    wmove (&win, height_poles + 4, 0);

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 19; j++)
        {
            waddch (&win, '-');
        }

        if (i != 3)
        {
            waddch (&win, '^');
        }
    }

    while (ring)
    {
        hanoi_draw_ring (ring, 1);
        ring--;
    }
}

static void
show_top_line_P (const char * str)
{
    int     col;

    wmove (&win, 1, 0);
    wattrset (&win, A_BOLD | F_WHITE | B_BLUE);

    for (col = 0; col < scr.bMaxx; col++)
    {
        waddch (&win, ' ');
    }

    mvwaddstr_P (&win, 1, 2, str);
    wattrset (&win, A_NORMAL);
}

static void
show_bottom_line_P (const char * str)
{
    uint8_t col;

    wmove (&win, scr.bMaxy - 3, 0);
    wattrset (&win, A_BOLD | F_WHITE | B_BLUE);

    for (col = 0; col < scr.bMaxx; col++)
    {
        waddch (&win, ' ');
    }

    mvwaddstr_P (&win, scr.bMaxy - 3, 2, str);
    wattrset (&win, A_NORMAL);
}

static void
message_P (const char * msg)
{
    wmove (&win, scr.bMaxy - 2, 0);
    waddstr_P (&win, msg);
    wclrtoeol (&win);
}

static void
shift_left (uint8_t y, uint8_t x, uint8_t ch)
{
    uint8_t col;

    wmove (&win, y, scr.bMaxx - 2);
    waddch (&win, ch);
    wmove (&win, y, x);

    for (col = scr.bMaxx - 2; col > x; col--)
    {
        PAUSE (5);
        wdelch (&win);
    }
}

static void
shift_left_str (uint8_t y, uint8_t x, const char __flash * str)
{
    const char __flash *  s;
    uint8_t xx = x;

    wattrset (&win, F_RED);

    for (s = str; *s; s++)
    {
        if (*s != ' ')
        {
            shift_left (y, xx, *s);
        }
        xx++;
    }

    wmove (&win, y, x);
    wattrset (&win, A_REVERSE);

    for (s = str; *s; s++)
    {
        waddch (&win, *s);
        PAUSE (25);
    }

    wmove (&win, y, x);
    wattrset (&win, F_BLUE);

    for (s = str; *s; s++)
    {
        waddch (&win, *s);
        PAUSE (25);
    }
}

static void
screen_demo (void)
{
    char    buf[10];
    uint8_t line;
    uint8_t col;

    wclear (&win);

    show_top_line_P (PSTR("TOP LINE 2"));
    show_bottom_line_P (PSTR("BOTTOM LINE 22"));
    wsetscrreg (&win, 2, win.bMaxy - 2);

    if (fast)
    {
        mvwaddstr_P (&win, 10, 20, PSTR("MCURSES LIB DEMO IN FAST MOTION"));
    }
    else
    {
        shift_left_str (10, 20, PSTR("MCURSES LIB DEMO IN SLOW MOTION"));
    }

    for (line = 0; line < 5; line++)
    {
        scroll (&win);
        PAUSE (200);
    }

    wmove (&win, 5, 15);
    for (line = 0; line < 5; line++)
    {
        winsertln (&win);
        PAUSE (200);
    }

    wmove (&win, 10, 18);
    for (col = 0; col < 5; col ++)
    {
        winsch (&win, ' ');
        PAUSE (200);
    }

    wmove (&win, 10, 18);
    for (col = 0; col < 5; col ++)
    {
        wdelch (&win);
        PAUSE (200);
    }

    wclear (&win);

    show_top_line_P (PSTR("TOP LINE 2"));
    show_bottom_line_P (PSTR("BOTTOM LINE 22"));

    message_P (PSTR("line positioning test"));

    for (line = 2; line <= scr.bMaxy - 4; line++)
    {
        wmove (&win, line, 0);
        waddstr (&win, myitoa (line + 1, buf));
    }

    PAUSE (700);

    message_P (PSTR("BOLD attribute test"));
    wattrset (&win, A_BOLD);
    mvwaddstr_P (&win, 10, 10, PSTR("BOLD"));
    wattrset (&win, A_NORMAL);
    PAUSE (700);

    message_P (PSTR("REVERSE attribute test"));
    wattrset (&win, A_REVERSE);
    mvwaddstr_P (&win, 11, 10, PSTR("REVERSE"));
    wattrset (&win, A_NORMAL);
    PAUSE (700);

    message_P (PSTR("insert character test"));
    for (col = 10; col <= 22; col += 2)
    {
        mvwinsch (&win, 11, col, ' ');
    }
    wmove (&win, 11, col + 1);
    PAUSE (700);

    message_P (PSTR("UNDERLINE attribute test"));
    wattrset (&win, A_UNDERLINE);
    mvwaddstr_P (&win, 12, 10, PSTR("UNDERLINE"));
    wattrset (&win, A_NORMAL);
    PAUSE (1000);

    message_P (PSTR("BLINK attribute test"));
    wattrset (&win, A_BLINK);
    mvwaddstr_P (&win, 13, 10, PSTR("BLINK"));
    wattrset (&win, A_NORMAL);
    PAUSE (1000);

    message_P (PSTR("DIM attribute test"));
    wattrset (&win, A_DIM);
    mvwaddstr_P (&win, 14, 10, PSTR("DIM"));
    wattrset (&win, A_NORMAL);
    PAUSE (1000);

    message_P (PSTR("insert line test"));
    wmove (&win, 11, 10);
    winsertln (&win);
    PAUSE (1000);

    waddstr_P (&win, PSTR("Inserted line, will be deleted soon..."));
    PAUSE (1000);

    message_P (PSTR("delete character test"));
    for (col = 10; col <= 16; col += 1)
    {
        mvwdelch (&win, 12, col);
    }
    wmove (&win, 12, 18);
    PAUSE (1000);

    message_P (PSTR("delete line test"));
    wmove (&win, 11, 10);
    wdeleteln (&win);
    PAUSE (1000);

    message_P (PSTR("scroll up line test"));
    for (line = 0; line < scr.bMaxy - 3; line++)
    {
        scroll (&win);
        PAUSE (50);
    }

    wmove (&win,  8, 20); wattrset (&win, A_BOLD | F_BLACK   | B_WHITE); waddstr_P (&win, PSTR("BLACK"));
    wmove (&win,  9, 20); wattrset (&win, A_BOLD | F_RED     | B_WHITE); waddstr_P (&win, PSTR("RED"));
    wmove (&win, 10, 20); wattrset (&win, A_BOLD | F_GREEN   | B_WHITE); waddstr_P (&win, PSTR("GREEN"));
    wmove (&win, 11, 20); wattrset (&win, A_BOLD | F_YELLOW  | B_WHITE); waddstr_P (&win, PSTR("YELLOW"));
    wmove (&win, 12, 20); wattrset (&win, A_BOLD | F_BLUE    | B_WHITE); waddstr_P (&win, PSTR("BLUE"));
    wmove (&win, 13, 20); wattrset (&win, A_BOLD | F_MAGENTA | B_WHITE); waddstr_P (&win, PSTR("MAGENTA"));
    wmove (&win, 14, 20); wattrset (&win, A_BOLD | F_CYAN    | B_WHITE); waddstr_P (&win, PSTR("CYAN"));
    wmove (&win, 15, 20); wattrset (&win, A_BOLD | F_WHITE   | B_BLACK); waddstr_P (&win, PSTR("WHITE"));
    wmove (&win, 16, 20); wattrset (&win, A_NORMAL); waddstr_P (&win, PSTR("normal"));

    wmove (&win,  8, 50); wattrset (&win, A_BOLD | B_BLACK   | F_WHITE); waddstr_P (&win, PSTR("BLACK"));
    wmove (&win,  9, 50); wattrset (&win, A_BOLD | B_RED     | F_WHITE); waddstr_P (&win, PSTR("RED"));
    wmove (&win, 10, 50); wattrset (&win, A_BOLD | B_GREEN   | F_WHITE); waddstr_P (&win, PSTR("GREEN"));
    wmove (&win, 11, 50); wattrset (&win, A_BOLD | B_YELLOW  | F_BLACK); waddstr_P (&win, PSTR("YELLOW"));
    wmove (&win, 12, 50); wattrset (&win, A_BOLD | B_BLUE    | F_WHITE); waddstr_P (&win, PSTR("BLUE"));
    wmove (&win, 13, 50); wattrset (&win, A_BOLD | B_MAGENTA | F_WHITE); waddstr_P (&win, PSTR("MAGENTA"));
    wmove (&win, 14, 50); wattrset (&win, A_BOLD | B_CYAN    | F_WHITE); waddstr_P (&win, PSTR("CYAN"));
    wmove (&win, 15, 50); wattrset (&win, A_BOLD | B_WHITE   | F_BLACK); waddstr_P (&win, PSTR("WHITE"));
    wmove (&win, 16, 50); wattrset (&win, A_NORMAL); waddstr_P (&win, PSTR("normal"));
    PAUSE (2000);
}

static void
temperature ()
{
    uint8_t idx;
    uint8_t t;
    uint8_t x;
    uint8_t loop;
    char    buf[10];
    unsigned char temp[15] = { 0, 8, 15, 21, 26, 30, 32, 35, 32, 30, 26, 21, 15, 8, 0 };

    curs_set (&win, 0);														// set cursor invisible
    wclear (&win);
    show_top_line_P (PSTR("Temperatures in a disk storage"));
    show_bottom_line_P (PSTR(""));

    for (loop = 0; loop < 30; loop++)
    {
        for (idx = 0; idx < 15; idx++)
        {
            if (temp[idx] > 30)
            {
                wattrset (&win, B_RED);
            }
            else if (temp[idx] > 20)
            {
                wattrset (&win, B_BROWN);
            }
            else
            {
                wattrset (&win, B_GREEN);
            }

            mvwaddstr_P (&win, idx + 4, 5, PSTR("Disk "));
            myitoa (idx + 1, buf);

            if (idx + 1 < 10)
            {
                waddch (&win, ' ');
            }

            waddstr (&win, buf);
            waddstr (&win, ": ");
            myitoa (temp[idx] + 20, buf);
            waddstr (&win, buf);
            waddch (&win, ACS_DEGREE);
            wattrset (&win, A_NORMAL);

            wmove (&win, idx + 4, 20);

            waddch (&win, ACS_LTEE);

            for (t = 0; t < temp[idx]; t++)
            {
                waddch (&win, ACS_HLINE);
            }

            wclrtoeol (&win);

            x = rand() & 0x1F;

            if (x == 0x01)
            {
                if (temp[idx] < 55)
                {
                    temp[idx]++;
                }
            }
            else if (x == 0x02)
            {
                if (temp[idx] > 0)
                {
                    temp[idx]--;
                }
            }
        }
        PAUSE (100);
    }

    curs_set (&win, 1);														// set cursor visible (normal)
}

CommandArgumentParser_t cap;

uint8_t cmd_cls(uint8_t bID, uint8_t bSequence, char* pcBuff, uint8_t bBuffLen)
{
	wclear_phys(&win);
	wmove_phys(&win, 0, 0);
	
	return CLI_CFR_FINISHED;
}

uint8_t cmd_demo(uint8_t bID, uint8_t bSequence, char* pcBuff, uint8_t bBuffLen)
{
	curs_set(&win, 0);
	screen_demo();
	curs_set(&win, 1);
	wclear(&win);
	wmove(&win, 0, 0);

	return CLI_CFR_FINISHED;
}

uint8_t cmd_hanoi(uint8_t bID, uint8_t bSequence, char* pcBuff, uint8_t bBuffLen)
{

	wclear (&win);
    curs_set(&win, 0);
	hanoi_number_of_rings = 5;
	hanoi_pole_height[0] = 0;
	hanoi_pole_height[1] = 0;
	hanoi_pole_height[2] = 0;

	hanoi_draw_poles ();
	hanoi (hanoi_number_of_rings, 1, 2, 3);
    PAUSE (1000);
	
    curs_set(&win, 1);
    wclear(&win);
    wmove(&win, 0, 0);

	return CLI_CFR_FINISHED;
}

uint8_t cmd_temp(uint8_t bID, uint8_t bSequence, char* pcBuff, uint8_t bBuffLen)
{
    curs_set(&win, 0);
    temperature();
    wattrset(&win, A_NORMAL | F_WHITE | B_BLACK);
    curs_set(&win, 1);
    wclear(&win);
    wmove(&win, 0, 0);
	
	return CLI_CFR_FINISHED;
}

uint8_t cmd_bad(uint8_t bID, uint8_t bSequence, char* pcBuff, uint8_t bBuffLen)
{
	strcpy_P(pcBuff, PSTR("Returning an incorrect command function return value!"));
	
	return 0xAB;
}

void nibbleToHex(uint8_t n)
{
	if (n < 10)
		waddch(&win, n + '0');
	else
		waddch(&win, n + 0x37);
}

void printHex(uint8_t b)
{
	nibbleToHex((b>>4));
	nibbleToHex(b&0x0F);
}

uint8_t cmd_dump(uint8_t bID, uint8_t bSequence, char* pcBuff, uint8_t bBuffLen)
{
	uint8_t			bRet;
	uint8_t			b;
	const uint8_t*	data  = 0;
	uint16_t		szPos = (bSequence-1)*16;

	bRet =  CLI_CFR_ADDITIONAL_OUTPUT;
	switch (bSequence)
	{
		case 0:
			b = 0;
			wclear_phys(&win);
			wmove_phys(&win, 0, 0);
			for (uint8_t i = 0; i < 30; i++)
				pcBuff[b++] = '-';
			strcpy_P(pcBuff+b, PSTR("Bytes: 2048"));
			b += 11;															// Length of "Bytes: 2048"
			for (uint8_t i = 0; i < 30; i++)
				pcBuff[b++] = '-';
			pcBuff[b] = 0;
			break;

		case 128:
			bRet =  CLI_CFR_OUTPUT;
			// No break

		default:
			xsnprintf_P(pcBuff, bBuffLen, PSTR("%04X: "), szPos);
			b = 6;
			for (uint8_t j = 0; j < 16; j++, szPos++)
			{
				xsnprintf_P(pcBuff+b, bBuffLen-b, PSTR("%02X"), data[szPos]);
				b += 2;
				if (j != 16 - 1)
					pcBuff[b++] = ' ';
			}
			pcBuff[b++] = ' ';
			pcBuff[b++] = ' ';
			szPos -= 16;
			for (uint8_t j=0; j < 16; j++, szPos++)
			{
				if (data[szPos] < 0x20 || data[szPos] > 0x7E)
					pcBuff[b++] =  '.';
				else
					pcBuff[b++] = (char)data[szPos];
			}
			pcBuff[b] = 0;
			break;
	}

	return bRet;
}

uint8_t cmd_keys(uint8_t bID, uint8_t bSequence, char* pcBuff, uint8_t bBuffLen)
{
	char	buf[10];
	uint8_t cnt;
	uint8_t ch;
	
    wclear (&win);

    nodelay (&win, TRUE);     // set nodelay, wgetch() will then return immediately
    cnt = 0;

    wmove (&win, 11, 10);
    waddstr_P (&win, PSTR("Timeout Counter: "));
    waddstr (&win, myitoa(cnt, buf));
    waddstr_P (&win, PSTR("  tenths of a sec"));
    wclrtoeol (&win);

    wmove (&win, 10, 10);
    waddstr_P (&win, PSTR("Press a key (2x ESC or 5 seconds timeout exits): "));

    while (1)
    {
	    wmove (&win, 11, 27);
	    waddstr (&win, myitoa(cnt, buf));
		waddch (&win, ' ');
	    wmove (&win, 10, 59);

	    ch = wgetch (&win);

	    switch (ch)
	    {
		    case '\t':          waddstr_P (&win, PSTR("TAB"));           break;
		    case '\r':          waddstr_P (&win, PSTR("CR"));            break;
		    case KEY_ESCAPE:    waddstr_P (&win, PSTR("KEY_ESCAPE"));    break;
		    case KEY_DOWN:      waddstr_P (&win, PSTR("KEY_DOWN"));      break;
		    case KEY_UP:        waddstr_P (&win, PSTR("KEY_UP"));        break;
		    case KEY_LEFT:      waddstr_P (&win, PSTR("KEY_LEFT"));      break;
		    case KEY_RIGHT:     waddstr_P (&win, PSTR("KEY_RIGHT"));     break;
		    case KEY_HOME:      waddstr_P (&win, PSTR("KEY_HOME"));      break;
		    case KEY_DC:        waddstr_P (&win, PSTR("KEY_DC"));        break;
		    case KEY_IC:        waddstr_P (&win, PSTR("KEY_IC"));        break;
		    case KEY_NPAGE:     waddstr_P (&win, PSTR("KEY_NPAGE"));     break;
		    case KEY_PPAGE:     waddstr_P (&win, PSTR("KEY_PPAGE"));     break;
		    case KEY_END:       waddstr_P (&win, PSTR("KEY_END"));       break;
		    case KEY_BTAB:      waddstr_P (&win, PSTR("KEY_BTAB"));      break;
		    case KEY_BACKSPACE: waddstr_P (&win, PSTR("KEY_BACKSPACE")); break;
		    case KEY_F(1):      waddstr_P (&win, PSTR("KEY_F(1)"));      break;
		    case KEY_F(2):      waddstr_P (&win, PSTR("KEY_F(2)"));      break;
		    case KEY_F(3):      waddstr_P (&win, PSTR("KEY_F(3)"));      break;
		    case KEY_F(4):      waddstr_P (&win, PSTR("KEY_F(4)"));      break;
		    case KEY_F(5):      waddstr_P (&win, PSTR("KEY_F(5)"));      break;
		    case KEY_F(6):      waddstr_P (&win, PSTR("KEY_F(6)"));      break;
		    case KEY_F(7):      waddstr_P (&win, PSTR("KEY_F(7)"));      break;
		    case KEY_F(8):      waddstr_P (&win, PSTR("KEY_F(8)"));      break;
		    case KEY_F(9):      waddstr_P (&win, PSTR("KEY_F(9)"));      break;
		    case KEY_F(10):     waddstr_P (&win, PSTR("KEY_F(10)"));     break;
		    case KEY_F(11):     waddstr_P (&win, PSTR("KEY_F(11)"));     break;
		    case KEY_F(12):     waddstr_P (&win, PSTR("KEY_F(12)"));     break;
		    case ERR:           PAUSE(100); cnt++;						 break;
		    default:            waddch (&win, ch);						 break;
	    }

	    if (ch != ERR)
	    {
		    cnt = 0;
		    wclrtoeol (&win);

		    if (ch == KEY_ESCAPE)
		    {
			    PAUSE(500);
			    break;
		    }
	    }
	    else if (cnt >= 50 + 1)
	    {
		    break;
	    }
    }
	
	waddstr_P(&win, PSTR("\r\n\r\n\r\n"));
	return CLI_CFR_FINISHED;
}

uint8_t cmd_state(uint8_t bID, uint8_t bSequence, char* pcBuff, uint8_t bBuffLen)
{
	switch (bSequence)
	{
		case 0:
			xsnprintf_P(pcBuff, bBuffLen, PSTR("\r\n"));
			return CLI_CFR_ADDITIONAL_OUTPUT;
			break;
			
		case 1:
			xsnprintf_P(pcBuff, bBuffLen, PSTR("MCurses state (values in row, col)"));
			return CLI_CFR_ADDITIONAL_OUTPUT;
			break;
			
		case 2:
			xsnprintf_P(pcBuff, bBuffLen, PSTR("\tTerminal size:    %03d, %03d"),
						scr.bMaxy, scr.bMaxx);
			return CLI_CFR_ADDITIONAL_OUTPUT;
			break;
		
		case 3:
			xsnprintf_P(pcBuff, bBuffLen, PSTR("\tWindow ULC:       %03d, %03d"),
						win.bBegy, win.bBegx);
			return CLI_CFR_ADDITIONAL_OUTPUT;
			break;
		
		case 4:
			xsnprintf_P(pcBuff, bBuffLen, PSTR("\tWindow size:      %03d, %03d"),
						win.bMaxy, win.bMaxx);
			return CLI_CFR_ADDITIONAL_OUTPUT;
			break;
		
		case 5:
			xsnprintf_P(pcBuff, bBuffLen, PSTR("\tCurrent position: %03d, %03d"),
						win.bCury, win.bCurx);
			return CLI_CFR_OUTPUT;
			break;
		
//		case 4:
//			xsnprintf_P(pcBuff, bBuffLen, PSTR("\tScroll start,end: %03d, %03d"),
//						scr.scrl_start, scr.scrl_end);
//			return CLI_CFR_OUTPUT;
//			break;
			
		default:
			return CLI_CFR_FINISHED;
			break;		
	}
}

uint8_t cmd_menu(uint8_t bID, uint8_t bSequence, char* pcBuff, uint8_t bBuffLen)
{
	uint8_t  bMenuStateIndex;
	uint16_t wMenuAttributes = F_BLUE | B_WHITE;
	
	bMenuStateIndex = 0;
	init_menu(g_amenu, 0, 0, 0, g_bMenuOptions, wMenuAttributes);
	set_menu_window(g_amenu, &win);
	post_menu(g_amenu);
	wclear(&win);
	while (menu_driver(g_amenu, sizeof(g_amenu), &bMenuStateIndex) == MENU_PROCESS_CONTINUE)
		;
	unpost_menu(g_amenu);
	wclear_phys(&win);
	wmove(&win, 0, 0);

	return CLI_CFR_FINISHED;
}

uint8_t cmd_exit(uint8_t bID, uint8_t bSequence, char* pcBuff, uint8_t bBuffLen)
{
	wclear(&win);
	
	return CLI_CFR_EXIT;
}

uint8_t cmd_arg(uint8_t bID, uint8_t bSequence, char* pcBuff, uint8_t bBuffLen)
{
	char	  sz[80];
	
	if (0 == bSequence)
	{
		cli_init_arg_parser(&cap, bID);

		while (cli_get_arg(&cap, pcBuff) == CLI_GA_ARGUMENT)
		{
			switch (cap.bArgumentID)
			{
				case 0:
					return cli_detailed_help(&cap, bID, bSequence, pcBuff, bBuffLen);
					break;

				case 1:
					xsnprintf_P(sz, sizeof(sz), PSTR("Text:  %s\r\n"), cap.xValue.pcText);
					waddstr(&win, sz);
					break;

				case 2:
					xsnprintf_P(sz, sizeof(sz), PSTR("Int:   %ld\r\n"), cap.xValue.lValue);
					waddstr(&win, sz);
					break;

				case 3:
					xsnprintf_P(sz, sizeof(sz), PSTR("Fixed: %k\r\n"), cap.xValue.kValue);
					waddstr(&win, sz);
					break;

				case 4:
					waddstr_P(&win, PSTR("Bool enabled\r\n"));
					break;
			}
		}
	}
	else
		return cli_detailed_help(&cap, bID, bSequence, pcBuff, bBuffLen);

	return CLI_CFR_FINISHED;
}

uint8_t cmd_version(uint8_t bID, uint8_t bSequence, char* pcBuff, uint8_t bBuffLen)
{
	strncpy_P(pcBuff, PSTR(CLI_CONFIG_VERSION), bBuffLen);
	
	return CLI_CFR_OUTPUT;
}

uint8_t cmd_cli(uint8_t bID, uint8_t bSequence, char* pcBuff, uint8_t bBuffLen)
{
	GetnstrState_t stateMachine;
	
	stateMachine.state = MC_SM_START;

	curs_set(&win, 1);                                                       // set cursor visible
	wclear(&win);
	wmove(&win, 0, 0);

	waddstr_P(&win, PSTR("MCurses Command Line Interface Demo\r\nFor help type: help\r\n\r\n"));
	while (cli_driver(&win, &stateMachine, 0xFF, buf, sizeof(buf)) != MC_SM_FINISHED)
		;
	
	return CLI_CFR_FINISHED;
}

// Command entries
const __flash char CE_HELP[]					= "help";
const __flash char CE_CLS[]						= "cls";
const __flash char CE_DEMO[]					= "demo";
const __flash char CE_HANOI[]					= "hanoi";
const __flash char CE_TEMP[]					= "temp";
const __flash char CE_BAD[]						= "bad";
const __flash char CE_DUMP[]					= "dump";
const __flash char CE_KEYS[]					= "keys";
const __flash char CE_STATE[]					= "state";
const __flash char CE_MENU[]					= "menu";
const __flash char CE_ARG[]						= "arg";
const __flash char CE_VERSION[]					= "ver";
const __flash char CE_EXIT[]					= "exit";

// Help descriptions
const __flash char HD_HELP[]					= "\tList the available commands with a short description";
const __flash char HD_CLS[]						= "\tClear the screen";
const __flash char HD_DEMO[]					= "\tShow the screen demo";
const __flash char HD_HANOI[]					= "\tDisplay the Tower of Hanoi";
const __flash char HD_TEMP[]					= "\tDisplay the temperature graph";
const __flash char HD_BAD[]						= "\tCommand function with a bad return type";
const __flash char HD_DUMP[]					= "\tDump SRAM";
const __flash char HD_KEYS[]					= "\tDisplay key values";
const __flash char HD_STATE[]					= "\tReturn the state of mcurses";
const __flash char HD_MENU[]					= "\tStart menu interface";
const __flash char HD_ARG[]						= "\tCommand argument example";
const __flash char HD_VERSION[]					= "\tShow command library version";
const __flash char HD_EXIT[]					= "\tExit the command line interface";

const __flash CommandEntry_t g_aCommandEntry[] =
{
//	  CommandID,	Command,	Help,		Function
	{  1,			CE_HELP,	HD_HELP,	cli_help    },
	{  2,			CE_CLS,		HD_CLS,		cmd_cls     },
	{  3,			CE_DEMO,	HD_DEMO,	cmd_demo    },
	{  4,			CE_HANOI,	HD_HANOI,	cmd_hanoi   },
	{  5,			CE_TEMP,	HD_TEMP,	cmd_temp    },
	{  6,			CE_BAD,		HD_BAD,		cmd_bad	    },
	{  7,			CE_DUMP,	HD_DUMP,	cmd_dump    },
	{  8,			CE_KEYS,	HD_KEYS,	cmd_keys    },
	{  9,			CE_STATE,	HD_STATE,	cmd_state   },
	{ 10,			CE_MENU,	HD_MENU,	cmd_menu    },
	{ 11,			CE_ARG,		HD_ARG,		cmd_arg		},
	{ 12,			CE_VERSION,	HD_VERSION,	cmd_version },
	{ 13,			CE_EXIT,	HD_EXIT,	cmd_exit    },

	{ CLI_ENTRY_END,0,			0,			NULL        }
};

// Command arguments
const __flash char CA_HELP[]				= "?";
const __flash char CA_ARG_TEXT[]			= "text";
const __flash char CA_ARG_INT[]				= "int";
const __flash char CA_ARG_FIXED[]			= "fixed";
const __flash char CA_ARG_BOOL[]			= "bool";

// Argument descriptions
const __flash char AD_HELP[]				= "List help for command arguments.";
const __flash char AD_ARG_TEXT[]			= "Enter a text string. Enclose in double quotes for spaces.";
const __flash char AD_ARG_INT[]				= "Enter a positive or negative integer value.";
const __flash char AD_ARG_FIXED[]			= "Enter a positive or negative fixed point value.";
const __flash char AD_ARG_BOOL[]			= "Enable the bool functionality.";
const __flash char AD_ARG_EXAMPLE[]			= "ARG text \"Hello world!\" int -2 fixed 3.1415 bool";

const __flash CommandArgument_t g_aCommandArgument[] =
{
	//	CommandID,	ArgumentID,	Type,				pcArgument,		pcHelp
	{	11,			0,			CLI_CAT_BOOL,		CA_HELP,		AD_HELP			},
	{	11,			1,			CLI_CAT_TEXT,		CA_ARG_TEXT,	AD_ARG_TEXT		},
	{	11,			2,			CLI_CAT_INT,		CA_ARG_INT,		AD_ARG_INT		},
	{	11,			3,			CLI_CAT_FIXED,		CA_ARG_FIXED,	AD_ARG_FIXED	},
	{	11,			4,			CLI_CAT_BOOL,		CA_ARG_BOOL,	AD_ARG_BOOL		},
	{	11,			5,			CLI_CAT_EXAMPLE,	0,				AD_ARG_EXAMPLE	},

	{  CLI_ENTRY_END,	0,	0,	0,	0 }
};

uint8_t menu_demo(uint8_t bID, uint8_t* pbData, uint8_t bOptions)
{
	uint8_t bSequence;
	uint8_t i;
	uint8_t (*pFunc)(uint8_t	bID,
					 uint8_t	bSequence,
					 char*		pcBuff,
					 uint8_t	bBuffLen);
						 
//	if (MENU_FUNC_OPTION_PREDRAW & bOptions)
//		return MENU_FUNC_RADIO_BUTTON;

	switch (bID)
	{
		case 13:
			pFunc = cmd_cls;
			break;
			
		case 14:
			pFunc = cmd_demo;
			break;
			
		case 15:
			pFunc = cmd_hanoi;
			break;
			
		case 16:
			pFunc = cmd_temp;
			break;
			
		case 17:
			pFunc = cmd_dump;
			break;
			
		case 18:
			pFunc = cmd_keys;
			break;
			
		case 19:
			pFunc = cmd_state;
			break;
			
		case 20:
			pFunc = cmd_cli;
			break;
		
		default:
			return 0;
			break;
	}

	bSequence = 0;
	while((i = pFunc(0, bSequence, buf, sizeof(buf))))
	{
		// Act on the command's return value
		switch (i)
		{
			case CLI_CFR_OUTPUT:
			case CLI_CFR_ADDITIONAL_OUTPUT:
				waddstr(&win, buf);
				waddstr_P(&win, PSTR("\r\n"));
				if (CLI_CFR_OUTPUT == i)
					return MENU_FUNC_CLOSE_MENU;
				break;
			
			default:
				return MENU_FUNC_CLOSE_MENU;
				break;
		}
		bSequence++;
	}

	return MENU_FUNC_CLOSE_MENU;
}

uint8_t menu_menu(uint8_t bID, uint8_t* pbData, uint8_t bOptions)
{
	if (MENU_FUNC_OPTION_PREDRAW == bOptions)
	{
		switch (bID)
		{
			case 18:
				if (!(g_bMenuOptions & MENU_OPTION_NONCYCLIC))
					return MENU_FUNC_RB_SELECTED;
				break;
					
			case 19:
				if (g_bMenuOptions & MENU_OPTION_SHOWPARENT)
					return MENU_FUNC_RB_SELECTED;
				break;
		}
	}
	
	if (MENU_FUNC_OPTION_SELECTED == bOptions)
	{
		switch (bID)
		{
			case 18:
				if (g_bMenuOptions & MENU_OPTION_NONCYCLIC)
					g_bMenuOptions &= ~MENU_OPTION_NONCYCLIC;
				else
					g_bMenuOptions |= MENU_OPTION_NONCYCLIC;
				break;
				
			case 19:
				if (g_bMenuOptions & MENU_OPTION_SHOWPARENT)
					g_bMenuOptions &= ~MENU_OPTION_SHOWPARENT;
				else
					g_bMenuOptions |= MENU_OPTION_SHOWPARENT;
				break;
		}
	}

	return MENU_FUNC_KEEP_OPEN;
}

uint8_t menu_color(uint8_t bID, uint8_t* pbData, uint8_t bOptions)
{
	if (MENU_FUNC_OPTION_PREDRAW & bOptions)
	{
		if (bID < 66)
		{
			if (((g_amenu->wAttrs & F_COLOR) >> 8) + 57 == bID)
				return MENU_FUNC_RB_SELECTED;
		}
		else
		{
			if (((g_amenu->wAttrs & B_COLOR) >> 12) + 65 == bID)
				return MENU_FUNC_RB_SELECTED;
		}
	}

	if (MENU_FUNC_OPTION_SELECTED & bOptions)
	{
		if (bID < 66)
		{
			g_amenu->wAttrs = (g_amenu->wAttrs & ~(F_COLOR)) | (uint16_t)((bID - 57) << 8);
		}
		else
		{
			g_amenu->wAttrs = (g_amenu->wAttrs & ~(B_COLOR)) | (uint16_t)((bID - 65) << 12);
		}
	}

	return MENU_FUNC_CLOSE_MENU;
}

uint8_t menu_form(uint8_t bID, uint8_t* pbData, uint8_t bOptions)
{
	uint8_t		bFormOptions	= FORM_OPTION_HAS_BORDER | FORM_OPTION_HCENTER | FORM_OPTION_VCENTER;
	uint16_t	wAttributes		= win.wAttrs;
	FORM		form;

	wclear(&win);
	init_form(&form, 0, 5, 5, 14, 55, bFormOptions, F_WHITE | B_BLACK);
	set_form_window(&form, &win);
	while (form_driver(&form) == FORM_DRIVER_CONTINUE)
		;
	wattrset(&win, wAttributes);
	wclear(&win);

	return MENU_FUNC_CLOSE_MENU;
}

uint8_t menu_getch(uint8_t bID, uint8_t* pbData, uint8_t bOptions)
{
	uint8_t ch;
	uint8_t lastCh = 0;
	
	wclear(&win);
	waddstr_P(&win, PSTR("\r\nPress keys to see escape sequence. Press crtl-[ctrl-[ to exit.\r\n"));
	
	while (1)
	{
		ch = wgetch_raw(&win);
		if (ERR == ch)
			continue;
			
		if ('\033' == ch)
		{
			if ('\033' == lastCh)
				return MENU_FUNC_CLOSE_MENU;
				
			lastCh = '\033';
			waddstr_P(&win, PSTR("\r\n^"));
		}
		else
		{
			lastCh = ch;
			waddch(&win, ch);
		}
	}
	
	return MENU_FUNC_CLOSE_MENU;
}

uint8_t menu_display(uint8_t bID, uint8_t* pbData, uint8_t bOptions)
{
	static uint8_t			bFormOptions;
	static uint16_t			wAttributes;
	static uint16_t			w;
	static FORM				form;
	static GetnstrState_t	stateMachine;
	
	if (0 == *pbData)
	{
		(*pbData)++;
		stateMachine.state = MC_SM_START;

		bFormOptions	= FORM_OPTION_HAS_BORDER | FORM_OPTION_HCENTER;
		wAttributes		= win.wAttrs;
		w				= 0;

		wclear(&win);
		wsetscrreg_phys(&win, 15, win.bMaxy);
		curs_set(&win, 1);
		wmove(&win, 15, 0);

		init_display(&form, 0, 0, 5, 14, 55, bFormOptions, F_WHITE | B_BLACK);
		set_display_window(&form, &win);
	}

	w = (w+1) & 0x07FF;

	if (w & 0x0008)
		data.i++;

	if (w & 0x0010)
		data.ui++;

	if (w & 0x0020)
		data.hex++;

	if (w & 0x0040)
		data.k++;

	if (w & 0x0080)
		data.uk++;

	if (w & 0x0100)
		xsnprintf_P(data.text, 40, PSTR("Counter w: %u"), w);

	if (w & 0x0200)
	{
		if (0 == data.bSelected)
			data.bSelected = 1;
		else if (1 == data.bSelected)
			data.bSelected = 2;
		else if (2 == data.bSelected)
			data.bSelected = 0;
	}

	if (w & 0x0240)
		data.abSelected[0] = (data.abSelected[0]+1) & 0x01;

	if (w & 0x0280)
		data.abSelected[1] = (data.abSelected[1]+1) & 0x01;

	if (w & 0x050)
		data.abSelected[2] = (data.abSelected[2]+1) & 0x01;

	display_driver(&form);

	if (MC_SM_FINISHED == cli_driver(&win, &stateMachine, 0xFF, buf, sizeof(buf)))
	{
		wsetscrreg_phys(&win, 0, 0);
		wattrset(&win, wAttributes);
		wclear(&win);

		return MENU_FUNC_CLOSE_MENU;
	}

	return MENU_FUNC_CONTINUE;
}

uint8_t menu_exit(uint8_t bID, uint8_t* pbData, uint8_t bOptions)
{
	return MENU_FUNC_EXIT_MENU_SYSTEM;
}

uint8_t form_field(WINDOW* win, uint8_t bID, uint8_t bOptions, char* pcBuff, uint8_t bBuffLen)
{
	static GetnstrState_t sm;

	char					szHex[10]		= "0x";
	char*					pszHex			= szHex;
	
	static long				iTemp			= 0;
	static long				uiTemp			= 0;
	static long				hexTemp			= 0;
	static _Accum			kTemp			= 0;
	static unsigned _Accum	ukTemp			= 0;
	static char				textTemp[41]	= "";

	switch (bOptions)
	{
		case FORM_FUNC_OPTION_GETCH:
			switch (bID)
			{
				case 0:
					wgetnstr_field_sm(win, &sm, pcBuff, bBuffLen, MC_T_INT);
					break;
				case 1:
					wgetnstr_field_sm(win, &sm, pcBuff, bBuffLen, MC_T_POS_INT);
					break;
				case 2:
					wgetnstr_field_sm(win, &sm, pcBuff, bBuffLen, MC_T_HEX);
					break;
				case 3:
					wgetnstr_field_sm(win, &sm, pcBuff, bBuffLen, MC_T_FP);
					break;
				case 4:
					wgetnstr_field_sm(win, &sm, pcBuff, bBuffLen, MC_T_POS_FP);
					break;
				default:
					wgetnstr_field_sm(win, &sm, pcBuff, bBuffLen, MC_T_TEXT);
					break;
			}
			return sm.ch;
			break;
		
		case FORM_FUNC_OPTION_INIT:
			switch (bID)
			{
				case 0:
					iTemp = data.i;
					break;
				case 1:
					uiTemp = data.ui;
					break;
				case 2:
					hexTemp = data.hex;
					break;
				case 3:
					kTemp = data.k;
					break;
				case 4:
					ukTemp = data.uk;
					break;
				default:
					strcpy(textTemp, data.text);
					break;
			}
			// No break.
		
		case FORM_FUNC_OPTION_DRAW:
			switch (bID)
			{
				case 0:
					xsnprintf_P(pcBuff, bBuffLen, PSTR("%ld"), iTemp);
					break;
				case 1:
					xsnprintf_P(pcBuff, bBuffLen, PSTR("%ld"), uiTemp);
					break;
				case 2:
					xsnprintf_P(pcBuff, bBuffLen, PSTR("%X"), hexTemp);
					break;
				case 3:
					xsnprintf_P(pcBuff, bBuffLen, PSTR("%k"), kTemp);
					break;
				case 4:
					xsnprintf_P(pcBuff, bBuffLen, PSTR("%K"), ukTemp);
					break;
				default:
					strncpy(pcBuff, textTemp, bBuffLen);
					break;
			}
			break;
		
		case FORM_FUNC_OPTION_HAS_FOCUS:
			sm.state = MC_SM_START;
			break;

		case FORM_FUNC_OPTION_LOSE_FOCUS:
			switch (bID)
			{
				case 0:
					xatoi(&pcBuff, &iTemp);
					break;
				case 1:
					xatoi(&pcBuff, &uiTemp);
					break;
				case 2:
					strncpy(szHex+2, pcBuff, 7);
					xatoi((char**)&pszHex, &hexTemp);
					break;
				case 3:
					xatok(&pcBuff, &kTemp, xioFRACBITS_K, 0);
					break;
				case 4:
					xatouk(&pcBuff, &ukTemp, xioFRACBITS_UK, 0);
					break;
				default:
					strncpy(textTemp, pcBuff, bBuffLen);
					break;
			}
			break;

		case FORM_FUNC_OPTION_COMMIT:
			switch (bID)
			{
				case 0:
					data.i = iTemp;
					break;
				case 1:
					data.ui = uiTemp;
					break;
				case 2:
					data.hex = hexTemp;
					break;
				case 3:
					data.k = kTemp;
					break;
				case 4:
					data.uk = ukTemp;
					break;
				default:
					strcpy(data.text, textTemp);
					break;
			}
			break;

		case FORM_FUNC_OPTION_CANCEL:
			break;

		case FORM_FUNC_OPTION_QUERY_CHANGED:							// Displays use this option
			switch (bID)
			{
				case 0:
					if (iTemp != data.i)
					{
						iTemp = data.i;
						xsnprintf_P(pcBuff, bBuffLen, PSTR("%ld"), iTemp);
						return FORM_FUNC_OK;
					}
					break;
				case 1:
					if (uiTemp != data.ui)
					{
						uiTemp = data.ui;
						xsnprintf_P(pcBuff, bBuffLen, PSTR("%ld"), uiTemp);
						return FORM_FUNC_OK;
					}
					break;
				case 2:
					if (hexTemp != data.hex)
					{
						hexTemp = data.hex;
						xsnprintf_P(pcBuff, bBuffLen, PSTR("%X"), hexTemp);
						return FORM_FUNC_OK;
					}
					break;
				case 3:
					if (kTemp != data.k)
					{
						kTemp = data.k;
						xsnprintf_P(pcBuff, bBuffLen, PSTR("%k"), kTemp);
						return FORM_FUNC_OK;
					}
					break;
				case 4:
					if (ukTemp != data.uk)
					{
						ukTemp = data.uk;
						xsnprintf_P(pcBuff, bBuffLen, PSTR("%K"), ukTemp);
						return FORM_FUNC_OK;
					}
					break;
				default:
					if (strcmp(textTemp, data.text))
					{
						strncpy(textTemp, data.text, 40);
						strncpy(pcBuff, textTemp, bBuffLen);
						return FORM_FUNC_OK;
					}
					break;
			}
			return FORM_FUNC_NO_CHANGE;
			break;
	}

	return FORM_FUNC_OK;
}

uint8_t form_radio(WINDOW* win, uint8_t bID, uint8_t bOptions, char* pcBuff, uint8_t bBuffLen)
{
	static uint8_t bSelectedTemp;
	
	switch (bOptions)
	{
		case FORM_FUNC_OPTION_INIT:
			bSelectedTemp = data.bSelected;
			// No break

		case FORM_FUNC_OPTION_DRAW:
			if (bID == bSelectedTemp)
			{
				return FORM_FUNC_SELECTED;
			}
			break;

		case FORM_FUNC_OPTION_SELECTED:
			bSelectedTemp = bID;
			return FORM_FUNC_SELECTED;
			break;
			
		case FORM_FUNC_OPTION_COMMIT:
			data.bSelected = bSelectedTemp;
			break;

		case FORM_FUNC_OPTION_QUERY_CHANGED:							// Displays use this option
			if (bSelectedTemp != data.bSelected)
			{
				if (2 == bID)
					bSelectedTemp = data.bSelected;
				return FORM_FUNC_OK;
			}
			return FORM_FUNC_NO_CHANGE;
			break;
	}

	return FORM_FUNC_OK;
}

uint8_t form_check(WINDOW* win, uint8_t bID, uint8_t bOptions, char* pcBuff, uint8_t bBuffLen)
{
	static uint8_t abSelectedTemp[3];
	
	switch (bOptions)
	{
		case FORM_FUNC_OPTION_INIT:
			abSelectedTemp[bID] = data.abSelected[bID];
			// No break

		case FORM_FUNC_OPTION_DRAW:
			if (abSelectedTemp[bID])
			{
				return FORM_FUNC_SELECTED;
			}
			break;

		case FORM_FUNC_OPTION_SELECTED:
			abSelectedTemp[bID] = abSelectedTemp[bID] ^ 0x01;
			if (abSelectedTemp[bID])
			{
				return FORM_FUNC_SELECTED;
			}
			break;
		
		case FORM_FUNC_OPTION_COMMIT:
			data.abSelected[bID] = abSelectedTemp[bID];
			break;

		case FORM_FUNC_OPTION_QUERY_CHANGED:							// Displays use this option
			if (abSelectedTemp[bID] != data.abSelected[bID])
			{
				abSelectedTemp[bID] = data.abSelected[bID];
				return FORM_FUNC_OK;
			}
			return FORM_FUNC_NO_CHANGE;
			break;
	}

	return FORM_FUNC_OK;
}

uint8_t form_button(WINDOW* win, uint8_t bID, uint8_t bOptions, char* pcBuff, uint8_t bBuffLen)
{
	switch (bOptions)
	{
		case FORM_FUNC_OPTION_SELECTED:
			if (0 == bID)
				return FORM_FUNC_COMMIT_FORM;
			else
				return FORM_FUNC_CANCEL_FORM;
		break;

		case FORM_FUNC_OPTION_QUERY_CHANGED:							// Displays use this option
			return FORM_FUNC_NO_CHANGE;
			break;
	}

	return FORM_FUNC_OK;
}

uint16_t myvalue(void)
{
	return (0);
}

void nothing(void)
{
	_delay_ms(1);
}

const __flash TERM g_aterm[] =
{
	{	mcurses_phyio_putc, mcurses_phyio_getbyte, myvalue, nothing	},
};

__attribute__((OS_main)) int main()
{
	char		szStatus[80];
	uint8_t		bMenuStateIndex;
	uint8_t		bCury = 0xFF;
	uint8_t		bCurx = 0xFF;

	g_wWinAttr		= F_WHITE| B_BLACK;
	g_bMenuOptions	= MENU_OPTION_MENUBAR | MENU_OPTION_SHOWPARENT;
	
	initTimer();

	GetnstrState_t stateMachine;

	newterm(0, &scr, &win);
	mcurses_term_scrreg(&win, TRUE);

	while(1)
	{	
		stateMachine.state = MC_SM_START;

		mcurses_term_reset(&win);

		while (scr.bFlags & SCR_FLAG_QUERY_SIZE)
			wgetch(&win);

		curs_set(&win, 0);												// set cursor invisible
		wclear(&win);
		wattrset(&win, g_wWinAttr);

		init_statusline(&stat, -1, 0, -1, F_BLUE | B_WHITE);
		set_statusline_window(&stat, &win);
		post_statusline(&stat);

		bMenuStateIndex = 0;
		init_menu(g_amenu, 0, 0, 0, g_bMenuOptions, F_BLUE | B_WHITE);
		set_menu_window(g_amenu, &win);
		post_menu(g_amenu);
		while (menu_driver(g_amenu, sizeof(g_amenu), &bMenuStateIndex) == MENU_PROCESS_CONTINUE)
		{
			if (bCury != win.bCury || bCurx != win.bCurx)
			{
				bCury = win.bCury;
				bCurx = win.bCurx;
				xsnprintf_P(szStatus, sizeof(szStatus), PSTR("R: %3hd C: %3hd Begy: %3hd Rows: %3hd"), bCury, bCurx, win.bBegy, win.bMaxy);
				statusline_draw_raw(&stat, szStatus, sizeof(szStatus));
			}
		}
		unpost_menu(g_amenu);

		curs_set(&win, 1);												// set cursor visible
		wclear(&win);
		wmove(&win, 0, 0);

		waddstr_P(&win, PSTR("MCurses Command Line Interface Demo\r\nFor help type: help\r\n\r\n"));
		while (cli_driver(&win, &stateMachine, 0xFF, buf, sizeof(buf)) != MC_SM_FINISHED)
		{
			if (bCury != win.bCury || bCurx != win.bCurx)
			{
				bCury = win.bCury;
				bCurx = win.bCurx;
				xsnprintf_P(szStatus, sizeof(szStatus), PSTR("R: %3hd C: %3hd Begy: %3hd Rows: %3hd"), bCury, bCurx, win.bBegy, win.bMaxy);
				statusline_draw(&stat, szStatus, 0, 0, SL_OPTION_TEXT_JUSTIFY_LEFT);
			}
		}

		unpost_statusline(&stat);
	}

    endwin (&win);

    return 0;
}
