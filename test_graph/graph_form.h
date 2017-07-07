/*
 * graph_form.h
 *
 * Created: 5/4/2017 7:36:28 PM
 *  Author: David
 */ 


#ifndef GRAPH_FORM_H_
#define GRAPH_FORM_H_

#include "../source/lib_mcurses/form.h"
#include "../source/lib_eqeval/eqeval.h"

#define FID_VIEW								0
#define FID_HELP								1
#define FID_ABOUT								2
#define FID_EQUATION							3

#define IID_OK									0
#define IID_CANCEL								1

#define IID_VIEW_XMIN							0
#define IID_VIEW_XMAX							1
#define IID_VIEW_XSCL							2
#define IID_VIEW_YMIN							3
#define IID_VIEW_YMAX							4
#define IID_VIEW_YSCL							5

#define IID_EQUATION							0

extern char	g_szEquation[EQEV_SIZE_EQ];

extern FORM* form;

#endif /* GRAPH_FORM_H_ */
