/*
 * display_struct.c
 *
 *  Created on: 18-07-2014
 *      Author: Tomek
 */


#include <lcd_interface.h>
#include <tpgui.h>

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

tpgui_screen_s main_screen=
	{
	.type=TPGUI_SCREEN,
	.keyOK_action= NULL,
	.keyEX_action= NULL,
	.keyUP_action= NULL,
	.keyDW_action= NULL,
	.keyOKl_action= NULL,
	.keyEXl_action= NULL,
	.keyUPl_action= NULL,
	.keyDWl_action= NULL,

	};

//------------------------------------------------------------------------------

tpgui_screen_item_label_s main_screen_label1=
	{
	.type= TPGUI_SI_LABEL,
	.attr= TPGUI_ITEM_ATTRIB_BLINKING,
	.col= 0,
	.row= 0,
	.len= 6,
	.text= "textlabel #1",
	};

tpgui_screen_item_label_s main_screen_label2=
	{
	.type= TPGUI_SI_LABEL,
	.attr= TPGUI_ITEM_ATTRIB_DOUBLESIZE | TPGUI_ITEM_ATTRIB_BLINKING | TPGUI_ITEM_ATTRIB_INVERT,
	.col= 0,
	.row= 1,
	.len= 9,
	.text= "textlabel #2",
	};
/*
tpgui_screen_item_label_s main_screen_label3=
	{
	.type= TPGUI_SI_LABEL,
	.attr= 0x00,
	.col= 0,
	.row= 3,
	.len= 12,
	.text= "textlabel #3",
	};
*/

tpgui_screen_item_variable_s tpgui_screen_item_variable1=
	{
	.type= TPGUI_SI_VARIABLE,
	.attr= 0x00,
	.col= 0,
	.row= 3,
	.len= 8,
	.data_type= TPGUI_VAR_DATATYPE_INT,
	.data_ptr= NULL,
	.precision= 0,
	};

tpgui_screen_item_variable_s tpgui_screen_item_variable2=
	{
	.type= TPGUI_SI_VARIABLE,
	.attr= 0x00,
	.col= 0,
	.row= 3,
	.len= 8,
	.data_type= TPGUI_VAR_DATATYPE_FLOAT,
	.data_ptr= NULL,
	.precision= 3,
	};

tpgui_screen_item_variable_s tpgui_screen_item_variable3=
	{
	.type= TPGUI_SI_VARIABLE,
	.attr= 0x00,
	.col= 0,
	.row= 3,
	.len= 8,
	.data_type= TPGUI_VAR_DATATYPE_TIME,
	.data_ptr= NULL,
	.precision= 0,
	};

tpgui_screen_item_variable_s tpgui_screen_item_variable4=
	{
	.type= TPGUI_SI_VARIABLE,
	.attr= 0x00,
	.col= 0,
	.row= 3,
	.len= 10,
	.data_type= TPGUI_VAR_DATATYPE_DATE,
	.data_ptr= NULL,
	.precision= 0,
	};


tpgui_screen_item_label_s main_screen_label4=
	{
	.type= TPGUI_SI_LABEL,
	.attr= TPGUI_ITEM_ATTRIB_INVERT,
	.col= 6,
	.row= 0,
	.len= 6,
	.text= "textlabel #1",
	};

tpgui_screen_item_label_s main_screen_label5=
	{
	.type= TPGUI_SI_LABEL,
	.attr= TPGUI_ITEM_ATTRIB_INVERT | TPGUI_ITEM_ATTRIB_BLINKING,
	.col= 10,
	.row= 3,
	.len= 6,
	.text= "ABCDEF",
	};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

tpgui_menu_s main_menu=
	{
	.type=TPGUI_MENU,

	.up_menu= NULL,

	};

//------------------------------------------------------------------------------

tpgui_menu_item_label_s tpgui_menu_item1=
	{
	.type= TPGUI_MI_LABEL,
	.attr= 0x00,
	.text= "MENU1",
	.action= NULL,
	};

tpgui_menu_item_label_s tpgui_menu_item2=
	{
	.type= TPGUI_MI_LABEL,
	.attr= 0x00,
	.text= "MENU2",
	.action= NULL,
	};

tpgui_menu_item_label_s tpgui_menu_item3=
	{
	.type= TPGUI_MI_LABEL,
	.attr= 0x00,
	.text= "MENU3",
	.action= NULL,
	};

tpgui_menu_item_label_s tpgui_menu_item4=
	{
	.type= TPGUI_MI_LABEL,
	.attr= 0x00,
	.text= "MENU4",
	.action= NULL,
	};

tpgui_menu_item_label_s tpgui_menu_item5=
	{
	.type= TPGUI_MI_LABEL,
	.attr= 0x00,
	.text= "MENU5",
	.action= NULL,
	};

tpgui_menu_item_label_s tpgui_menu_item6=
	{
	.type= TPGUI_MI_LABEL,
	.attr= 0x00,
	.text= "MENU6",
	.action= NULL,
	};

tpgui_menu_item_label_s tpgui_menu_item7=
	{
	.type= TPGUI_MI_LABEL,
	.attr= 0x00,
	.text= "MENU7",
	.action= NULL,
	};

tpgui_menu_item_label_s tpgui_menu_item8=
	{
	.type= TPGUI_MI_LABEL,
	.attr= 0x00,
	.text= "MENU8",
	.action= NULL,
	};


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
