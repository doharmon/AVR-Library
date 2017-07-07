/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * form.h - Include file for mcurses form lib
 *
 * Copyright (c) 2016 David Harmon
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */

 
#ifndef FORM_H
#define FORM_H

#include "mcurses.h"
#include "form-config.h"
#include <inttypes.h>

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * raw graphics: draw boxes
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define ACS_LRCORNER_RAW					0x6A												// DEC graphic 0x6A: lower right corner
#define ACS_URCORNER_RAW					0x6B												// DEC graphic 0x6B: upper right corner
#define ACS_ULCORNER_RAW					0x6C												// DEC graphic 0x6C: upper left corner
#define ACS_LLCORNER_RAW					0x6D												// DEC graphic 0x6D: lower left corner
#define ACS_PLUS_RAW						0x6E												// DEC graphic 0x6E: crossing lines
#define ACS_HLINE_RAW						0x71												// DEC graphic 0x71: horizontal line
#define ACS_LTEE_RAW						0x74												// DEC graphic 0x74: left tee
#define ACS_RTEE_RAW						0x75												// DEC graphic 0x75: right tee
#define ACS_BTEE_RAW						0x76												// DEC graphic 0x76: bottom tee
#define ACS_TTEE_RAW						0x77												// DEC graphic 0x77: top tee
#define ACS_VLINE_RAW						0x78												// DEC graphic 0x78: vertical line

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Structure to hold form items
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
typedef uint8_t (*pFIFunc)(	WINDOW* win,
							uint8_t bItemID,
							uint8_t bOption, 
							char*   pcBuff, 
							uint8_t bBuffLen);

typedef struct formitem
{
	uint8_t				bFormID;																// Form's ID
	uint8_t				bItemID;																// Item's ID. Passed to the pFunc callback function.
	uint8_t				bType;																	// Item type
	uint8_t				bRow;																	// Item row. Upper left corner is (0,0)
	uint8_t				bCol;																	// Item column
	uint8_t				bParam;																	// Text input field display length (no trailing zero)/Radio group
	uint16_t			wAttrs;																	// Display attributes of item
	pFIFunc				pFunc;																	// Callback function for form item
	const __flash char*	pcText;																	// Display text for label or button. Status line for input or button
} FormItem_t;

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * The global array g_aFormItem needs to be defined and configured with all the forms that well be used by the application
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
extern const __flash FormItem_t g_aFormItem[];

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Special form item IDs.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define FORM_ITEMID_HEADER					254													// ItemID for form entry that is the form header
#define FORM_ENTRY_END						255													// FormID for last entry in FormItem list

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Form item types. Stored in the bType field of the FormItem list
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define FORM_ITEM_LEFT_JUSTIFY				0x00												// Left justify the header in the form's frame
#define FORM_ITEM_CENTER					0x01												// Center the header in the form's frame
#define FORM_ITEM_RIGHT_JUSTIFY				0x02												// Right justify the header in the form's frame
#define FORM_ITEM_LABEL						0x04												// Item is a text label
#define FORM_ITEM_RADIO_BUTTON				0x08												// Item is a radio button
#define FORM_ITEM_CHECK_BOX					0x10												// Item is a check box
#define FORM_ITEM_BUTTON					0x20												// Item is a button
#define FORM_ITEM_FIELD						0x40												// Item is a text input field
//#define FORM_ITEM_WORD_BREAK				0x80

// Short cut for any form item that takes user input. Not to be used in the FormItem_t array.
#define FORM_ITEM_INPUT	(FORM_ITEM_RADIO_BUTTON | FORM_ITEM_CHECK_BOX | FORM_ITEM_BUTTON | FORM_ITEM_FIELD)

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Form callback function options. Passed to the form item callback functions in the bOptions parameter
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define FORM_FUNC_OPTION_INIT				0x01												// First call to callback function before drawing
#define FORM_FUNC_OPTION_DRAW				0x02												// General call to draw item
#define FORM_FUNC_OPTION_HAS_FOCUS			0x03												// Item has received focus
#define FORM_FUNC_OPTION_LOSE_FOCUS			0x04												// Item has lost focus
#define FORM_FUNC_OPTION_COMMIT				0x05												// Form is closing and items should accept changes
#define FORM_FUNC_OPTION_CANCEL				0x06												// Form is canceled and items should discard changes
#define FORM_FUNC_OPTION_GETCH				0x07												// Form field should process a user input key
#define FORM_FUNC_OPTION_SELECTED			0x08												// Button was pressed
#define FORM_FUNC_OPTION_QUERY_CHANGED		0x10												// Used by display to ask if field has changed

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Return codes for the form callback functions. Returned by the pFunc callback functions in the FormItem list
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define FORM_FUNC_OK						0x00												// Callback has accepted or processed the request
#define FORM_FUNC_COMMIT_FORM				0x01												// Close form and have inputs commit changes
#define FORM_FUNC_CANCEL_FORM				0x02												// Cancel form and have inputs discard changes
#define FORM_FUNC_SELECTED					0x04												// Radio button/Check box is selected
#define FORM_FUNC_REFUSE_FOCUS				0x08												// Input has refused to take focus
#define FORM_FUNC_KEEP_FOCUS				0x10												// Input has refused losing focus
#define FORM_FUNC_NO_CHANGE					0x20												// Return for FORM_FUNC_OPTION_DRAW, no changes

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Callback function options and expected return codes
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 
																			Text
	Option			Radio			Check			Button					Field			Notes
	=============	============	============	=======================	============	===========================================
	INIT			ignored			ignored			not called				ignored *		pcBuff is written to window
	DRAW			SELECTED		SELECTED		not called				NO_CHANGE *		pcBuff is written to window
	HAS_FOCUS		REFUSE_FOCUS	REFUSE_FOCUS	REFUSE_FOCUS			REFUSE_FOCUS
	LOSE_FOCUS		KEEP_FOCUS *	KEEP_FOCUS *	KEEP_FOCUS *			KEEP_FOCUS *	Test field should read acBuff to save value
	COMMIT			ignored			ignored			ignored					ignored			Inputs should save any changes
	CANCEL			ignored			ignored			ignored					ignored			Inputs should ignore any changes
	GETCH			not called		not called		not called				char *			pcBuff is buffer for wgetnstr
	SELECTED		SELECTED		ignored			COMMIT_FORM,CANCEL_FORM

	*			= pcBuff and bBuffLen are passed in callback function
	ignored 	= The callback function's return code is ignored
	not called	= This option is not used for this item type
	
	Note: Callback functions should return FORM_FUNC_OK unless returning an expected return code, even if return is ignored.

 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Form options
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define FORM_OPTION_LEFT_JUSTIFY			0x00												// Left justify the form on the screen
#define FORM_OPTION_HCENTER					0x01												// Center the form horizontally on the screen
#define FORM_OPTION_VCENTER					0x02												// Center the form vertically on the screen
#define FORM_OPTION_RIGHT_JUSTIFY			0x04												// Right justify the form on the screen
#define FORM_OPTION_HAS_BORDER				0x08												// Place a border around form and include any header

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Return codes for the form_driver function
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define FORM_DRIVER_FINISHED				0x00												// From has closed
#define FORM_DRIVER_CONTINUE				0x01												// Continue to process the form

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Structure for FORM
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
typedef struct form
{
	uint8_t		bState;																			// Set to FORM_SM_START before calling form API
	uint8_t		bFormID;																		// ID of the current form
	uint8_t		bStart;																			// Index into FormItem_t table of form's first entry
	uint8_t		bEnd;																			// Index into FormItem_t table of form's last entry
	uint8_t		bHasFocus;																		// Index into FormItem_t table of input with focus
	uint8_t		bRow;																			// Row of form's upper left corner (0,0 is top left)
	uint8_t		bCol;																			// Column of form's upper left column
	uint8_t		bWidth;																			// Width of form
	uint8_t		bHeight;																		// Height of form
	uint8_t		bOptions;																		// Frame options
	uint16_t	wAttrs;																			// Default frame attributes
	char		acBuff[FORM_CONFIG_BUFFER_SIZE];												// Utility buffer. Make as large as largest field length
	WINDOW*		win;
} FORM;

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * States of the form state machine
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define FORM_SM_START						0x00												// Form has not yet been drawn
#define FORM_SM_NEXT_FOCUS					0x01												// Cycle forward to next input that accepts focus
#define FORM_SM_PREV_FOCUS					0x02												// Cycle backwards to next input that accepts focus
#define FORM_SM_HAS_FOCUS					0x03												// An input has focus
#define FORM_SM_SELECTED					0x80												// Show text in text field as selected (A_REVERSE)

void	init_form(FORM* form, uint8_t bFormID, uint8_t bRow, uint8_t bCol, uint8_t bHeight, uint8_t bWidth, uint8_t bOptions, uint16_t wAttributes);
void	set_form_window(FORM* form, WINDOW* win);
void	form_attrset(FORM* form, uint16_t wAttributes);
void	post_form(FORM* form);
void	unpost_form(FORM* form);
uint8_t form_driver(FORM* form);

#endif // FORM_H
