// Created by flarn2006 <flarn2006@gmail.com>
// Compile with libmatheval (http://www.gnu.org/software/libmatheval/)
// Also requires ncurses (Google it.)
// To compile without libmatheval, use -DNOLIBMATHEVAL.
// (Edit defaultFunction to change which function to graph then.)
// Source: https://github.com/flarn2006/MiscPrograms/blob/master/graph.c
//
// Modified by David Harmon 2016 to support mcurses
//

#include "../source/lib_mcurses/mcurses.h"
#include "../source/lib_gccfix/gccfix.h"
#include "../source/lib_xio/xio.h"
#include "graph_form.h"
#include "graph.h"
#include <string.h>

#define XMIN -2*PIk
#define XMAX 2*PIk
#define YMIN -PIk
#define YMAX PIk
#define XSCL 1
#define YSCL 1

void defaultKeyHandler(WINDOW* win, int key, khdata *data);

int enableSlopeChars = 1;
key_handler handleKey = defaultKeyHandler;

_Accum defaultFunction(_Accum x)
{
	return eqeval_eval(x);
}

_Accum estimateSlope(yfunction func, _Accum x, _Accum accuracy)
{
	_Accum y1 = func(x - accuracy);
	_Accum y2 = func(x + accuracy);
	return (y2 - y1) / (2 * accuracy);
}

_Accum scale(_Accum value, _Accum omin, _Accum omax, _Accum nmin, _Accum nmax)
{
	/* Useful function to scale a value in one range to a different range.
	   omin/omax - old range
	   nmin/nmax - new range
	*/
	_Accum x = (value - omin) / (omax - omin);
	return x * (nmax - nmin) + nmin;
}

void plotPoint(WINDOW* win, const viewwin *view, _Accum x, _Accum y, char ch, int *scrY, int *scrX)
{
	/* Displays a point on the screen at a location determined by graph coordinates.
	   win       - ncurses window for drawing (can be NULL to only set scrY and scrX w/o drawing)
	   view      - view parameters structure
	   x/y       - graph coordinates for point
	   ch        - character to display
	   scrY/scrX - screen coordinates where point was drawn are saved here if not NULL
	*/
	int xm, ym; getmaxyx(win, ym, xm);
	
	if (y < view->ymin || y > view->ymax)
		return;
	
	int xp = scale(x, view->xmin, view->xmax, 0, xm);
	int yp = scale(y, view->ymin, view->ymax, ym, 0);
	
	if (scrX) *scrX = xp;
	if (scrY) *scrY = yp;

	if (win) 
		if (wmove(win, yp, xp) == OK)
			waddch(win, ch);
}

char slopeChar(_Accum slope)
{
	// Gets the character to display at a point in the graph with a given slope.
	
	_Accum a = absk(slope);
	if (a < 0.5)        return '=';
	else if (a < 1.5)   return slope>0 ? '/' : '\\';
	else                return '|';
}

int editViewWindow(WINDOW* win, viewwin *view)
{
	uint8_t		bFormOptions	= FORM_OPTION_HAS_BORDER | FORM_OPTION_LEFT_JUSTIFY;
	uint16_t	wAttributes		= F_WHITE | B_BLACK;

	graph_view = view;
	wattrset(win, wAttributes);
	init_form(form, FID_VIEW, 5, 5, 10, 21, bFormOptions, wAttributes);
	set_form_window(form, win);
	while (form_driver(form) == FORM_DRIVER_CONTINUE)
		;

	return 0;
}

int helpWindow(WINDOW* win)
{
	uint8_t		bFormOptions	= FORM_OPTION_HAS_BORDER | FORM_OPTION_LEFT_JUSTIFY;
	uint16_t	wAttributes		= F_WHITE | B_BLACK;

	wattrset(win, wAttributes);
	init_form(form, FID_HELP, 5, 5, 14, 31, bFormOptions, wAttributes);
	set_form_window(form, win);
	while (form_driver(form) == FORM_DRIVER_CONTINUE)
		;

	return 0;
}

void getViewStep(WINDOW* win, const viewwin *view, _Accum *xstep, _Accum *ystep)
{
	// Gets the 'value' of one character on either or both axes.

	int xm, ym; getmaxyx(win, ym, xm);
	if (xstep) *xstep = (view->xmax - view->xmin) / (xm + 1);
	if (ystep) *ystep = (view->ymax - view->ymin) / (ym + 1);
}

static _Accum mod(_Accum x, _Accum y)
{
	_Accum div = x/y;

	union
	{
		long	l;
		_Accum	k;
	} val;

	val.k = div;
	val.l &= ~((1L << xioFRACBITS_K) - 1);

	div = div - val.k;
	
	return y * div;
}

void drawAxes(WINDOW* win, const viewwin *view)
{
	// This function is what draws the axes on the screen.
	
	int i;
	int xm, ym; getmaxyx(win, ym, xm);
	_Accum x0 = scale(0, view->xmin, view->xmax, 0, xm);
	_Accum y0 = scale(0, view->ymin, view->ymax, ym, 0);

	_Accum xstep, ystep; getViewStep(win, view, &xstep, &ystep);

	for (i=0; i<=xm; i++) 
	{
		_Accum plotx = view->xmin + xstep * i;
		int tick = absk(mod(plotx, view->xscl)) < xstep;
		if (wmove(win, y0, i) == OK)
			waddch(win, tick ? '+':'-');
	}
	for (i=0; i<=ym; i++) 
	{
		_Accum ploty = view->ymin + ystep * i;
		int tick = absk(mod(ploty, view->yscl)) < ystep;
		if (wmove(win, i, x0) == OK)
			waddch(win, tick ? '+':'|');
	}
	
	if (wmove(win, y0, x0) == OK)
		waddch(win, '+');
}

void drawGraph(WINDOW* win, const viewwin *view, yfunction yfunc, int enableSlopeChars)
{
	/* Draws a graph on the screen without axes.
	   win              - ncurses window for drawing
	   view             - view parameters structure
	   yfunc            - function to graph (function pointer)
	   enableSlopeChars - whether or not to call slopeChar to determine characters
	*/
	_Accum step; getViewStep(win, view, &step, NULL);
	_Accum x; for (x = view->xmin; x <= view->xmax; x += step)
	{
		_Accum y = yfunc(x);
		_Accum d = estimateSlope(yfunc, x, step/2);
		plotPoint(win, view, x, y, enableSlopeChars ? slopeChar(d):'#', NULL, NULL);
	}
}

void traceKeyHandler(WINDOW* win, int key, khdata *data)
{
	// Keyboard handling function for trace mode.

	char		acBuff[20];
	uint8_t		cbBuff = sizeof(acBuff);
	GetnstrState_t gs;
	
	_Accum step; getViewStep(win, data->view, &step, NULL);

	switch (key) 
	{
		case KEY_LEFT:  *data->trace -= 4.0*step; break;
		case KEY_RIGHT: *data->trace += 4.0*step; break;
	}

	if (*data->trace < data->view->xmin) *data->trace = data->view->xmin;
	if (*data->trace > data->view->xmax) *data->trace = data->view->xmax;

	if (key == 't') 
	{
		handleKey = defaultKeyHandler;
		*data->mode = MODE_GRAPH;
	}

	if (key == KEY_CR) 
	{
		wmove(win, 3, 0);
		wclrtoeol(win);
		wattrset(win, F_YELLOW | B_BLACK);
		waddstr_P(win, PSTR("Enter X value: "));
		wrefresh(win);
		curs_set(win, 1);
		gs.state  = MC_SM_START;
		acBuff[0] = 0;
		while (MC_SM_FINISHED != wgetnstr_field_sm(win, &gs, acBuff, cbBuff, MC_T_FP | MC_TB_CL))
			;
		if (strlen(acBuff))
		{
			char* pcBuff = acBuff;
			union {long l; _Accum k;} val;
			if(xatoq(&pcBuff, &val.l, xioFRACBITS_K, 0))
				*data->trace = val.k;
		}
		curs_set(win, 0);// noecho();
	}
}

void defaultKeyHandler(WINDOW* win, int key, khdata *data)
{
	// Default keyboard handling function.
	
	viewwin *view = data->view;
	_Accum xshift = 0, yshift = 0;
	
	switch (key) 
	{
		case KEY_UP:	yshift = 1; break;
		case KEY_DOWN:	yshift = -1; break;
		case KEY_LEFT:	xshift = -1; break;
		case KEY_RIGHT:	xshift = 1; break;
	}
	
	xshift *= (view->xmax - view->xmin) / 8;
	yshift *= (view->ymax - view->ymin) / 8;
	
	view->xmin += xshift; view->xmax += xshift;
	view->ymin += yshift; view->ymax += yshift;

	if (key == '-')
	 {
		view->xmin *= 1.5; view->xmax *= 1.5;
		view->ymin *= 1.5; view->ymax *= 1.5;
	}

	if (key == '=') 
	{
		view->xmin /= 1.5; view->xmax /= 1.5;
		view->ymin /= 1.5; view->ymax /= 1.5;
	}

	if (key == 's') enableSlopeChars = !enableSlopeChars;

	if (key == 't')
	 {
		handleKey = traceKeyHandler;
		*data->mode = MODE_TRACE;
	}

	if (key == 'w') editViewWindow(win, view);

	if (key == 'h') helpWindow(win);

	if (key == 'r') 
	{
		view->xmin = XMIN; view->xmax = XMAX;
		view->ymin = YMIN; view->ymax = YMAX;
		view->xscl = XSCL; view->yscl = YSCL;
	}
}

void drawTrace(WINDOW* win, viewwin *view, yfunction yfunc, _Accum x)
{
	/* Draws the trace cursor on the screen.
	   win   - ncurses window for drawing
	   view  - view parameters structure
	   yfunc - function to call to determine y coordinate
	   x     - graph coordinate for x position
	*/
	_Accum y = yfunc(x);
	int yp, xp;
	wattrset(win, F_YELLOW | B_BLACK);
	plotPoint(win, view, x, y, '+', &yp, &xp);
	if (wmove(win, yp-1, xp) == OK)
		waddch(win, '|');
	if (wmove(win, yp+1, xp) == OK)
		waddch(win, '|');
	if (wmove(win, yp, xp-2) == OK)
		waddch(win, '-');
	if (wmove(win, yp, xp-1) == OK)
		waddch(win, '-');
	if (wmove(win, yp, xp+1) == OK)
		waddch(win, '-');
	if (wmove(win, yp, xp+2) == OK)
		waddch(win, '-');
	wmove(win, 0, 0);
	xfprintf_P(win->scr->term->putByte, PSTR("X: %.5k"), x);
	wmove(win, 1, 0);
	xfprintf_P(win->scr->term->putByte, PSTR("Y: %.5k"), y);
//	mvprintw(win, 0, 0, "X: %.5lf", x);
//	mvprintw(win, 1, 0, "Y: %.5lf", y);
}

viewwin		view;
yfunction	yfunc;
oper_mode	mode;
_Accum		trace;

void draw(WINDOW* win)
{
	wclear(win);
	
	// perform drawing
	wattrset(win, F_GREEN | B_BLACK);
	drawAxes(win, &view);
	wattrset(win, F_YELLOW | B_BLACK);
	drawGraph(win, &view, yfunc, enableSlopeChars);
	if (mode == MODE_TRACE)
		drawTrace(win, &view, yfunc, trace);
}

uint8_t graph(WINDOW* win, uint8_t bState)
{
	uint8_t			key;
	static khdata	khd;

	if (0 == bState)
	{
		key   = 0;
		yfunc = defaultFunction;
		mode  = MODE_GRAPH;
		trace = 0.0;

		wattrset(win, F_GREEN | B_BLACK);
		wclear(win);

		view.xmin = XMIN; view.xmax = XMAX;
		view.ymin = YMIN; view.ymax = YMAX;
		view.xscl = XSCL; view.yscl = YSCL;

		khd.view  = &view;
		khd.mode  = &mode;
		khd.trace = &trace;

		draw(win);
	}
	else
	{
		key = wgetch(win);
		if (ERR == key)
			return 0;
			
		if ('q' == key)
			return 1;

		handleKey(win, key, &khd);
		draw(win);
	}

	// ncurses initialization
//	curs_set(win, 0);
//	nodelay(win, 0);
// 	while (key != 'q') 
// 	{	
// 
// 		do 
// 		{
// 			key = wgetch(win);
// 		} while (ERR == key);
// 	}
//	nodelay(win, 1);
// 	wattrset(win, F_GREEN | B_BLACK);
// 	wclear(win);

	return 0;
}
