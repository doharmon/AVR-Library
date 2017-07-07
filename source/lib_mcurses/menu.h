/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * menu.h - Include file for mcurses menu lib
 *
 * Copyright (c) 2016 David Harmon
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */

 
#ifndef MENU_H
#define MENU_H

#include "mcurses.h"
#include <inttypes.h>

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Structure to hold menu items
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
typedef uint8_t (*pMIFunc)(	uint8_t  bItemID,
							uint8_t* pbData, 
							uint8_t  bOptions);
 
typedef struct menuitem
{
	uint8_t				bMenuID;																// Menu's ID
	uint8_t				bItemID;																// Item's ID. Passed to the pFunc callback function.
	uint8_t				bOptions;																// Options for menu item
	uint8_t				bSubMenuGroup;															// Menu ID for sub menu
	pMIFunc				pFunc;																	// Callback function for menu item
	const __flash char*	pcText;																	// Menu text
} MenuItem_t;

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * The global array g_aMenuItem needs to be defined and configured with all the menus that well be used by the application
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
extern const __flash MenuItem_t g_aMenuItem[];

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Special menu item ID.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define MENU_ENTRY_END						255													// ID for last menu entry in MenuItem list

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Menu item options. Stored in the bOptions field of the MenuItem list
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define MENU_ITEM_CALL_FUNC_ON_MENU_DRAW	0x01												// Not implemented
#define MENU_ITEM_RADIO_BUTTON				0x02												// Menu item is a radio button

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Menu function options. Passed to the menu item functions in the bOptions parameter
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define MENU_FUNC_OPTION_PREDRAW			0x01												// Only passed to radio buttons
#define MENU_FUNC_OPTION_SELECTED			0x02												// Menu item was selected

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Return codes for the menu functions. Returned by the pFunc functions in the MenuItem list
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define MENU_FUNC_CLOSE_MENU				0x01												// Close parent menu item
#define MENU_FUNC_KEEP_OPEN					0x02												// Keep parent menu item open
#define MENU_FUNC_EXIT_MENU_SYSTEM			0x04												// Exit the menu
#define MENU_FUNC_CONTINUE					0x08												// Continue calling the menu item's callback function
#define MENU_FUNC_RB_SELECTED				0x10												// Radio button is selected

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Callback function options and expected return codes
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 
	Option			Radio			All Others				Notes
	=============	============	=====================	================================
	PREDRAW			RB_SELECTED		not called		
	SELECTED		CLOSE_MENU,KEEP_OPEN,EXIT_MENU_SYSTEM	All items can return these codes

	not called = This option is not used for this item type

 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Menu options
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define MENU_OPTION_MENUBAR					0x01												// Menu appears in a single row
#define MENU_OPTION_DROPDOWN				0x02												// Menu bar with dropdown sub menus
#define MENU_OPTION_POPUP					0x04												// Menu appears as a dropdown
#define MENU_OPTION_NONCYCLIC				0x08												// Do not cycle between first and last menu items
#define MENU_OPTION_SHOWDESC				0x10												// Not implemented
#define MENU_OPTION_SHOWPARENT				0x20												// Show parent menu for MENU_OPTION_MENUBAR

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Return codes for the menu_process function
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define MENU_PROCESS_FINISHED				0x00												// Stop processing the menu
#define MENU_PROCESS_CONTINUE				0x01												// Continue processing the menu
#define MENU_PROCESS_SELECT_ITEM			0x02												// Current menu item was selected
#define MENU_PROCESS_ERR_EXCEED_LEVELS		0x10												// Nested menu levels exceed menu array

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Structure for MENU
 *
 * Menu Bar Format
 *
 *     Item1  Item2  <Item3  Item4  Item5 Item6  Item7> Item8 Item9
 *     ^              ^             ^            ^            ^
 *     Start          First         Selected     Last         End
 *
 * Menu Bar Format with MENU_OPTION_SHOWPARENT
 *
 *     Parent: SubItem1  SubItem2  <SubItem3  SubItem4  SubItem5 SubItem6> SubItem7
 *             ^                    ^                   ^        ^         ^
 *             Start                First               Selected Last      End
 *
 * Pop Up Format
 *
 *     Item1    < Start
 *     Item2
 *    ^Item3    < First
 *     Item4
 *     Item5    < Selected
 *     Item6
 *    vItem7    < Last
 *     Item8
 *     Item9    < End
 *
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
/*
typedef struct menuitems
{
	uint8_t		bMenuID;																		// ID of the menu structure to display
	uint8_t		bStart;																			// Index to menu's first item in the menu item array
	uint8_t		bEnd;																			// Index to menu's last item in the menu item array
	uint8_t		bFirst;																			// Index to first displayed item in the menu item array
	uint8_t		bLast;																			// Index to last displayed item in the menu item array
	uint8_t		bSelected;																		// Index to menu item with focus
} MENUITEMS;

typedef struct menu
{
	uint8_t		bState;																			// Menu state
	uint8_t		bRow;																			// Zero based row of ULC (upper left corner) of menu
	uint8_t		bCol;																			// Zero based column of ULC of menu
	uint8_t		bData;																			// Data passed to menu command between consecutive calls
	uint8_t		bOptions;																		// Menu options
	uint16_t	wAttrs;																			// Menu attributes
	uint8_t		bSize;																			// amenuitems array size
	MENUITEMS*	amenuitems;																		// Pointer to array of menu items to handle sub menus
	WINDOW*		win;																			// Window containing menu
} MENU;
*/ 
typedef struct menu
{
	uint8_t		bState;																			// Menu state
	uint8_t		bMenuID;																		// ID of the menu structure to display
	uint8_t		bStart;																			// Index to menu's first item in the menu item array
	uint8_t		bEnd;																			// Index to menu's last item in the menu item array
	uint8_t		bFirst;																			// Index to first displayed item in the menu item array
	uint8_t		bLast;																			// Index to last displayed item in the menu item array
	uint8_t		bSelected;																		// Index to menu item with focus
	uint8_t		bRow;																			// Zero based row of ULC (upper left corner) of menu
	uint8_t		bCol;																			// Zero based column of ULC of menu
	uint8_t		bData;																			// Data passed to menu command between consecutive calls
	uint8_t		bOptions;																		// Menu options
	uint16_t	wAttrs;																			// Menu attributes
	WINDOW*		win;																			// Window containing menu
} MENU;

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * States of the menu state machine
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define MENU_SM_PROCESS						0x00												// Process menu
#define MENU_SM_CONTINUE					0x01												// Continue calling the menu item's callback function

void	init_menu(MENU* menu, uint8_t bMenuID, uint8_t bRow, uint8_t bCol, uint8_t bOptions, uint16_t wAttributes);
void	set_menu_window(MENU* menu, WINDOW* win);
void	post_menu(MENU* menu);
void	unpost_menu(MENU* menu);
void 	menu_draw_menubar(MENU* menu);
uint8_t menu_driver(MENU* amenu, uint8_t cMenuStates, uint8_t* pbMenuStateIndex);

#endif // MENU_H
