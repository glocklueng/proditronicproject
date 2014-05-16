#ifndef __TPGUI_H__
#define __TPGUI_H__

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "wdlist.h"


//------------------------------------------------------------------------------

#define TPGUI_SCREEN_REFRESH_PERIOD		((portTickType)50 / portTICK_RATE_MS)	// [ms]
#define TOGUI_SCREEN_BLINKING_PERIOD	1000 									// [ms]

#define TPGUI_SCREEN		0x01
#define TPGUI_MENU			0x02
#define TPGUI_FUNCTION		0x10



#define TPGUI_KEY_PRESS_OK		0x01
#define TPGUI_KEY_PRESS_EXIT	0x02
#define TPGUI_KEY_PRESS_UP		0x04
#define TPGUI_KEY_PRESS_DOWN	0x08




enum TPGUI_SCREEN_ITEM
	{
	TPGUI_SI_LABEL,


	};


enum TPGUI_MENU_ITEM
	{
	TPGUI_MI_LABEL,


	};



//------------------------------------------------------------------------------

typedef struct
	{
	unsigned char type;


	} tpgui_action_s;


typedef struct
	{
	unsigned char type;
	wdlist_s item_list;
	
	tpgui_action_s *keyOK_action;
	tpgui_action_s *keyEX_action;
	tpgui_action_s *keyUP_action;
	tpgui_action_s *keyDW_action;
	
	tpgui_action_s *keyOKl_action;
	tpgui_action_s *keyEXl_action;
	tpgui_action_s *keyUPl_action;
	tpgui_action_s *keyDWl_action;
	
	} tpgui_screen_s;


typedef struct
	{
	unsigned char type;
	wdlist_s item_list;
	
	bool changed;


	} tpgui_menu_s;


//------------------------------------------------------------------------------

typedef struct
	{
	TPGUI_SCREEN_ITEM type;
	bool changed;
	unsigned char attr;

	} tpgui_screen_item_s;


typedef struct
	{
	TPGUI_MENU_ITEM type;


	} tpgui_menu_item_s;



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// SCREEN ITEM

typedef struct
	{
	TPGUI_SCREEN_ITEM type;
	bool changed;
	unsigned char attr;

	unsigned char col;
	unsigned char row;
	unsigned char len;		// rozmiar (w znakach)
	
	unsigned char *text;


	} tpgui_screen_item_label_s;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// MENU ITEM

typedef struct
	{
	TPGUI_MENU_ITEM type;
	
	
	} tpgui_menu_item_label_s;



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------




void tpgui_run(void *start_screen);


void tpgui_screen_init(tpgui_screen_s *screen);
void tpgui_screen_item_add(tpgui_screen_s *screen, tpgui_screen_item_s *item);


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#endif // __TPMENU_H__
