/*
 * graph_form.c
 *
 * Created: 5/4/2017 7:35:59 PM
 *  Author: David
 */ 

#include "../source/lib_eqeval/eqeval.h"
#include "../source/lib_xio/xio.h"
#include "../source/lib_mcurses/statusline.h"
#include "graph_form.h"
#include "graph.h"
#include <string.h>

extern char			g_sz[];
extern uint8_t		g_cbField;
extern STATUSLINE*	stat;

viewwin*			graph_view;
const __flash char	GT_K[] = "%k";
GetnstrState_t		sm;
FORM				Form, *form = &Form;
char				g_szEquation[EQEV_SIZE_EQ] = "SIN X";

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Form structures
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */

// View Form Text
const __flash char FT_VWHEADER[]				= "Graph View";
const __flash char FT_XMIN[]					= "Xmin =";
const __flash char FT_XMAX[]					= "Xmax =";
const __flash char FT_XSCL[]					= "Xscl =";
const __flash char FT_YMIN[]					= "Ymin =";
const __flash char FT_YMAX[]					= "Ymax =";
const __flash char FT_YSCL[]					= "Yscl =";
const __flash char FT_VWOK[]					= " OK ";
const __flash char FT_VWCANCEL[]				= " Cancel ";

// Help Form Text
const __flash char FT_HWHEADER[]				= "Help";
const __flash char FT_MINUS[]					= "-: Zoom Out";
const __flash char FT_EQUAL[]					= "=: Zoom In";
const __flash char FT_S[]						= "s: Slope Char";
const __flash char FT_T[]						= "t: Trace";
const __flash char FT_W[]						= "w: View Window";
const __flash char FT_R[]						= "r: Reset";
const __flash char FT_H[]						= "h: Help Window";
const __flash char FT_Q[]						= "q: Quite";
const __flash char FT_ENTER[]					= "Enter: In trace to change x";
const __flash char FT_CURSOR[]					= "Cursor keys move graph";

// About Form Text
const __flash char FT_ABOUT[]					= "About";
const __flash char FT_EXAMPLE[]					= "MCURSES Example Display";
const __flash char FT_YEAR[]					= "2017 David Harmon";

// Equation Form Text
const __flash char FT_EQUATION[]				= "Enter Equation";

uint8_t graph_field(WINDOW* win, uint8_t bID, uint8_t bOptions, char* pcBuff, uint8_t bBuffLen);
uint8_t graph_button(WINDOW* win, uint8_t bID, uint8_t bOptions, char* pcBuff, uint8_t bBuffLen);
uint8_t graph_equation(WINDOW* win, uint8_t bID, uint8_t bOptions, char* pcBuff, uint8_t bBuffLen);

const __flash FormItem_t g_aFormItem[] =
{
	//
	// FormID,			ItemID,				Type,						Row, 	Col,	Param,			Attrs,			pFunc,			pcText
	{  FID_VIEW,		FORM_ITEMID_HEADER,	FORM_ITEM_CENTER,			0,		0,		0,				0,				0,				FT_VWHEADER	},
	{  FID_VIEW,		0,					0,							0,		1,		0,				0,				0,				FT_XMIN		},
	{  FID_VIEW,		0,					0,							1,		1,		0,				0,				0,				FT_XMAX		},
	{  FID_VIEW,		0,					0,							2,		1,		0,				0,				0,				FT_XSCL		},
	{  FID_VIEW,		0,					0,							3,		1,		0,				0,				0,				FT_YMIN		},
	{  FID_VIEW,		0,					0,							4,		1,		0,				0,				0,				FT_YMAX		},
	{  FID_VIEW,		0,					0,							5,		1,		0,				0,				0,				FT_YSCL		},
	{  FID_VIEW,		IID_VIEW_XMIN,		FORM_ITEM_FIELD,			0,		8,		10,				A_UNDERLINE,	graph_field,	0			},
	{  FID_VIEW,		IID_VIEW_XMAX,		FORM_ITEM_FIELD,			1,		8,		10,				A_UNDERLINE,	graph_field,	0			},
	{  FID_VIEW,		IID_VIEW_XSCL,		FORM_ITEM_FIELD,			2,		8,		10,				A_UNDERLINE,	graph_field,	0			},
	{  FID_VIEW,		IID_VIEW_YMIN,		FORM_ITEM_FIELD,			3,		8,		10,				A_UNDERLINE,	graph_field,	0			},
	{  FID_VIEW,		IID_VIEW_YMAX,		FORM_ITEM_FIELD,			4,		8,		10,				A_UNDERLINE,	graph_field,	0			},
	{  FID_VIEW,		IID_VIEW_YSCL,		FORM_ITEM_FIELD,			5,		8,		10,				A_UNDERLINE,	graph_field,	0			},
	{  FID_VIEW,		IID_OK,				FORM_ITEM_BUTTON,			7,		2,		0,				0,				graph_button,	FT_VWOK		},
	{  FID_VIEW,		IID_CANCEL,			FORM_ITEM_BUTTON,			7,		9,		0,				0,				graph_button,	FT_VWCANCEL },

	{  FID_HELP,		FORM_ITEMID_HEADER,	FORM_ITEM_CENTER,			0,		0,		0,				0,				0,				FT_HWHEADER	},
	{  FID_HELP,		0,					0,							0,		1,		0,				0,				0,				FT_MINUS	},
	{  FID_HELP,		0,					0,							1,		1,		0,				0,				0,				FT_EQUAL	},
	{  FID_HELP,		0,					0,							2,		1,		0,				0,				0,				FT_S		},
	{  FID_HELP,		0,					0,							3,		1,		0,				0,				0,				FT_T		},
	{  FID_HELP,		0,					0,							4,		1,		0,				0,				0,				FT_W		},
	{  FID_HELP,		0,					0,							5,		1,		0,				0,				0,				FT_R		},
	{  FID_HELP,		0,					0,							6,		1,		0,				0,				0,				FT_H		},
	{  FID_HELP,		0,					0,							7,		1,		0,				0,				0,				FT_Q		},
	{  FID_HELP,		0,					0,							8,		1,		0,				0,				0,				FT_ENTER	},
	{  FID_HELP,		0,					0,							9,		1,		0,				0,				0,				FT_CURSOR	},
	{  FID_HELP,		IID_OK,				FORM_ITEM_BUTTON,			11,		12,		0,				0,				graph_button,	FT_VWOK		},

	{  FID_ABOUT,		FORM_ITEMID_HEADER,	FORM_ITEM_CENTER,			0,		0,		0,				0,				0,				FT_ABOUT	},
	{  FID_ABOUT,		0,					0,							4,		3,		0,				0,				0,				FT_EXAMPLE	},
	{  FID_ABOUT,		0,					0,							6,		6,		0,				0,				0,				FT_YEAR		},
	{  FID_ABOUT,		IID_OK,				FORM_ITEM_BUTTON,			11,		12,		0,				0,				graph_button,	FT_VWOK		},

	{  FID_EQUATION,	FORM_ITEMID_HEADER,	FORM_ITEM_CENTER,			0,		0,		0,				0,				0,				FT_EQUATION	},
	{  FID_EQUATION,	IID_EQUATION,		FORM_ITEM_FIELD,			1,		1,		EQEV_SIZE_EQ,	A_UNDERLINE,	graph_equation,	0			},
	{  FID_EQUATION,	IID_OK,				FORM_ITEM_BUTTON,			4,		30,		0,				0,				graph_button,	FT_VWOK		},
	{  FID_EQUATION,	IID_CANCEL,			FORM_ITEM_BUTTON,			4,		37,		0,				0,				graph_button,	FT_VWCANCEL },

	{  FORM_ENTRY_END,	0,					0,							0,		0,		0,				0,				0,				0			}
};

uint8_t graph_field(WINDOW* win, uint8_t bID, uint8_t bOptions, char* pcBuff, uint8_t bBuffLen)
{
	static _Accum xmin;
	static _Accum xmax;
	static _Accum xscl;
	static _Accum ymin;
	static _Accum ymax;
	static _Accum yscl;
	
	_Accum k;

	switch (bOptions)
	{
		case FORM_FUNC_OPTION_GETCH:
			wgetnstr_field_sm(win, &sm, pcBuff, bBuffLen, MC_T_FP);
			return sm.ch;
			break;
		
		case FORM_FUNC_OPTION_INIT:
			switch (bID)
			{
				case IID_VIEW_XMIN:
					xmin = graph_view->xmin;
					break;
				case IID_VIEW_XMAX:
					xmax = graph_view->xmax;
					break;
				case IID_VIEW_XSCL:
					xscl = graph_view->xscl;
					break;
				case IID_VIEW_YMIN:
					ymin = graph_view->ymin;
					break;
				case IID_VIEW_YMAX:
					ymax = graph_view->ymax;
					break;
				case IID_VIEW_YSCL:
					yscl = graph_view->yscl;
					break;
			}
			// No break.
		
		case FORM_FUNC_OPTION_DRAW:
			switch (bID)
			{
				case IID_VIEW_XMIN:
					k = xmin;
					break;
				case IID_VIEW_XMAX:
					k = xmax;
					break;
				case IID_VIEW_XSCL:
					k = xscl;
					break;
				case IID_VIEW_YMIN:
					k = ymin;
					break;
				case IID_VIEW_YMAX:
					k = ymax;
					break;
				case IID_VIEW_YSCL:
					k = yscl;
					break;
			}
			xsnprintf_P(pcBuff, bBuffLen, GT_K, k);
			break;
		
		case FORM_FUNC_OPTION_HAS_FOCUS:
			sm.state = MC_SM_START;
			break;

		case FORM_FUNC_OPTION_LOSE_FOCUS:
			xatok(&pcBuff, &k, xioFRACBITS_K, 0);
			switch (bID)
			{
				case IID_VIEW_XMIN:
					xmin = k;
					break;
				case IID_VIEW_XMAX:
					xmax = k;
					break;
				case IID_VIEW_XSCL:
					xscl = k;
					break;
				case IID_VIEW_YMIN:
					ymin = k;
					break;
				case IID_VIEW_YMAX:
					ymax = k;
					break;
				case IID_VIEW_YSCL:
					yscl = k;
					break;
			}
			break;

		case FORM_FUNC_OPTION_COMMIT:
			switch (bID)
			{
				case IID_VIEW_XMIN:
					graph_view->xmin = xmin;
					break;
				case IID_VIEW_XMAX:
					graph_view->xmax = xmax;
					break;
				case IID_VIEW_XSCL:
					graph_view->xscl = xscl;
					break;
				case IID_VIEW_YMIN:
					graph_view->ymin = ymin;
					break;
				case IID_VIEW_YMAX:
					graph_view->ymax = ymax;
					break;
				case IID_VIEW_YSCL:
					graph_view->yscl = yscl;
					break;
			}
			break;

		case FORM_FUNC_OPTION_CANCEL:
			break;
	}

	return FORM_FUNC_OK;
}

uint8_t graph_button(WINDOW* win, uint8_t bID, uint8_t bOptions, char* pcBuff, uint8_t bBuffLen)
{
	switch (bOptions)
	{
		case FORM_FUNC_OPTION_SELECTED:
			if (IID_OK == bID)
				return FORM_FUNC_COMMIT_FORM;
			else
				return FORM_FUNC_CANCEL_FORM;
	}

	return FORM_FUNC_OK;
}

uint8_t graph_equation(WINDOW* win, uint8_t bID, uint8_t bOptions, char* pcBuff, uint8_t bBuffLen)
{
	uint8_t		err;
	static char sz[EQEV_SIZE_EQ];
	
	switch (bOptions)
	{
		case FORM_FUNC_OPTION_GETCH:
			wgetnstr_field_sm(win, &sm, pcBuff, bBuffLen, MC_T_TEXT);
			return sm.ch;
			break;
		
		case FORM_FUNC_OPTION_INIT:
			switch (bID)
			{
				case IID_EQUATION:
					strncpy(sz, g_szEquation, EQEV_SIZE_EQ);
					break;
			}
			// No break.
		
		case FORM_FUNC_OPTION_DRAW:
			switch (bID)
			{
				case IID_EQUATION:
					strncpy(pcBuff, sz, bBuffLen);
					break;
			}
			break;
		
		case FORM_FUNC_OPTION_HAS_FOCUS:
			sm.state = MC_SM_START;
			break;

		case FORM_FUNC_OPTION_LOSE_FOCUS:
			switch (bID)
			{
				case IID_EQUATION:
					err = eqeval_parse(pcBuff);
					if (err != EQEV_E_OK)
					{
						strncpy_P(g_sz, eqeval_get_err_msg(err), g_cbField);
						statusline_draw(stat,
										g_sz, 
										g_cbField, 
										1, 
										SL_OPTION_FIELD_JUSTIFY_LEFT | SL_OPTION_FIELD_OFFSET| SL_OPTION_CLEAR_FIRST);

						return FORM_FUNC_KEEP_FOCUS;
					}
					g_sz[0] = '\0';
					statusline_draw(stat,
									g_sz, 
									g_cbField, 
									1, 
									SL_OPTION_FIELD_JUSTIFY_LEFT | SL_OPTION_FIELD_OFFSET | SL_OPTION_CLEAR_FIRST);
					strncpy(sz, pcBuff, EQEV_SIZE_EQ);
					break;
			}
			break;

		case FORM_FUNC_OPTION_COMMIT:
			switch (bID)
			{
				case IID_EQUATION:
					strcpy(g_szEquation, sz);
					break;
			}
			break;

		case FORM_FUNC_OPTION_CANCEL:
			eqeval_parse(g_szEquation);
			break;
	}

	return FORM_FUNC_OK;
}
