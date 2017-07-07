/*
 * graph.h
 *
 * Created: 5/4/2017 7:59:27 PM
 *  Author: David
 */ 


#ifndef GRAPH_H_
#define GRAPH_H_

typedef struct _viewwin viewwin;
typedef struct _khdata khdata;
typedef _Accum (*yfunction)(_Accum x);
typedef void (*key_handler)(WINDOW* win, int key, khdata *data);
typedef enum {MODE_GRAPH, MODE_TRACE} oper_mode;

struct _viewwin
{
	_Accum xmin, xmax;
	_Accum ymin, ymax;
	_Accum xscl, yscl;
};

struct _khdata
{
	// Struct for pointers to data that key handlers may need to access
	viewwin*	view;
	oper_mode*	mode;
	_Accum*		trace;
};

extern viewwin *graph_view;

#endif /* GRAPH_H_ */