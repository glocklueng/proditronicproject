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

tpgui_screen_s screen_1;
tpgui_screen_s screen_2;
tpgui_screen_s screen_3;
tpgui_screen_s screen_4;


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

tpgui_screen_s screen_1=
	{
	.type=TPGUI_SCREEN,

	.keyOK_action.type= 0,
	.keyEX_action.type= 0,

	.keyUP_action.action.screen= &screen_2,
	.keyUP_action.type= TPGUI_SCREEN,

	.keyDW_action.action.screen= &screen_4,
	.keyDW_action.type= TPGUI_SCREEN,

	.keyOKl_action.type= 0,
	.keyEXl_action.type= 0,
	.keyUPl_action.type= 0,
	.keyDWl_action.type= 0,
	};

tpgui_screen_s screen_2=
	{
	.type=TPGUI_SCREEN,

	.keyOK_action.type= 0,
	.keyEX_action.type= 0,

	.keyUP_action.action.screen= &screen_3,
	.keyUP_action.type= TPGUI_SCREEN,

	.keyDW_action.action.screen= &screen_1,
	.keyDW_action.type= TPGUI_SCREEN,

	.keyOKl_action.type= 0,
	.keyEXl_action.type= 0,
	.keyUPl_action.type= 0,
	.keyDWl_action.type= 0,
	};

tpgui_screen_s screen_3=
	{
	.type=TPGUI_SCREEN,

	.keyOK_action.type= 0,
	.keyEX_action.type= 0,

	.keyUP_action.action.screen= &screen_4,
	.keyUP_action.type= TPGUI_SCREEN,

	.keyDW_action.action.screen= &screen_2,
	.keyDW_action.type= TPGUI_SCREEN,

	.keyOKl_action.type= 0,
	.keyEXl_action.type= 0,
	.keyUPl_action.type= 0,
	.keyDWl_action.type= 0,
	};

tpgui_screen_s screen_4=
	{
	.type=TPGUI_SCREEN,

	.keyOK_action.type= 0,
	.keyEX_action.type= 0,

	.keyUP_action.action.screen= &screen_1,
	.keyUP_action.type= TPGUI_SCREEN,

	.keyDW_action.action.screen= &screen_3,
	.keyDW_action.type= TPGUI_SCREEN,

	.keyOKl_action.type= 0,
	.keyEXl_action.type= 0,
	.keyUPl_action.type= 0,
	.keyDWl_action.type= 0,
	};



//------------------------------------------------------------------------------

tpgui_screen_item_label_s screen_1_label1=
	{
	.type= TPGUI_SI_LABEL,
	.attr= 0x00,
	.col= 0,
	.row= 0,
	.len= 16,
	.text= "Screen #1",
	};

tpgui_screen_item_label_s screen_1_label2=
	{
	.type= TPGUI_SI_LABEL,
	.attr= TPGUI_ITEM_ATTRIB_BLINKING,
	.col= 0,
	.row= 1,
	.len= 16,
	.text= "test",
	};

tpgui_screen_item_variable_s screen_1_time_label=
	{
	.type= TPGUI_SI_VARIABLE,
	.attr= TPGUI_ITEM_ATTRIB_DOUBLESIZE,
	.col= 0,
	.row= 2,
	.len= 8,
	.text= NULL,
	.data_type= TPGUI_VAR_DATATYPE_TIME,
	.data_ptr= NULL,
	.precision= 0,

	};



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

tpgui_screen_item_label_s screen_2_label1=
	{
	.type= TPGUI_SI_LABEL,
	.attr= 0x00,
	.col= 0,
	.row= 0,
	.len= 16,
	.text= "Screen #2",
	};

tpgui_screen_item_label_s screen_2_label2=
	{
	.type= TPGUI_SI_LABEL,
	.attr= TPGUI_ITEM_ATTRIB_BLINKING,
	.col= 0,
	.row= 1,
	.len= 16,
	.text= "test",
	};



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
tpgui_screen_item_label_s screen_3_label1=
	{
	.type= TPGUI_SI_LABEL,
	.attr= 0x00,
	.col= 0,
	.row= 0,
	.len= 16,
	.text= "Screen #3",
	};

tpgui_screen_item_label_s screen_3_label2=
	{
	.type= TPGUI_SI_LABEL,
	.attr= TPGUI_ITEM_ATTRIB_BLINKING,
	.col= 0,
	.row= 1,
	.len= 16,
	.text= "test",
	};



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
tpgui_screen_item_label_s screen_4_label1=
	{
	.type= TPGUI_SI_LABEL,
	.attr= 0x00,
	.col= 0,
	.row= 0,
	.len= 16,
	.text= "Screen #4",
	};

tpgui_screen_item_label_s screen_4_label2=
	{
	.type= TPGUI_SI_LABEL,
	.attr= TPGUI_ITEM_ATTRIB_BLINKING,
	.col= 0,
	.row= 1,
	.len= 16,
	.text= "test",
	};



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------




//------------------------------------------------------------------------------

tpgui_screen_item_label_s main_screen_1_label1=
	{
	.type= TPGUI_SI_LABEL,
	.attr= 0x00,
	.col= 0,
	.row= 0,
	.len= 16,
	.text= "Screen #2",
	};

tpgui_screen_item_variable_s tpgui_screen_1_item_variable1=
	{
	.type= TPGUI_SI_VARIABLE,
	.attr= 0x00,
	.col= 0,
	.row= 1,
	.len= 8,
	.data_type= TPGUI_VAR_DATATYPE_INT,
	.data_ptr= NULL,
	.precision= 0,
	};


tpgui_screen_item_label_s main_screen_1_label2=
	{
	.type= TPGUI_SI_LABEL,
	.attr= TPGUI_ITEM_ATTRIB_INVERT,
	.col= 0,
	.row= 2,
	.len= 6,
	.text= "textlabel #1",
	};

tpgui_screen_item_label_s main_screen_1_label3=
	{
	.type= TPGUI_SI_LABEL,
	.attr= TPGUI_ITEM_ATTRIB_INVERT | TPGUI_ITEM_ATTRIB_BLINKING,
	.col= 0,
	.row= 3,
	.len= 6,
	.text= "ABCDEF",
	};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
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
