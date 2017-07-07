/*
 * graph_menu.h
 *
 * Created: 5/4/2017 7:36:50 PM
 *  Author: David
 */ 


#ifndef GRAPH_MENU_H_
#define GRAPH_MENU_H_

#include "../source/lib_mcurses/menu.h"

#define MENU_MAXDEPTH							2

#define MID_MAIN								0
#define MID_PLOT								1

#define IID_PLOT								0
#define IID_ABOUT								1
#define IID_PLOT_DRAW							1
#define IID_PLOT_EQUATIONS						2
#define IID_PLOT_SETTINGS						3
#define IID_PLOT_LOADDEFAULTS					4

extern MENU g_amenu[MENU_MAXDEPTH];

#endif /* GRAPH_MENU_H_ */
