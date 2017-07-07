/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * display.h - Include file for mcurses display
 *
 * Copyright (c) 2016 David Harmon
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */

 
#ifndef DISPLAY_H
#define DISPLAY_H

#include "mcurses.h"
#include "form.h"
//#include "display-config.h"
#include <inttypes.h>

#define init_display		init_form
#define set_display_window	set_form_window
void	display_driver(FORM* form);

#endif // DISPLAY_H
