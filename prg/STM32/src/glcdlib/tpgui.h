#ifndef __TPGUI_H__
#define __TPGUI_H__

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include <stdbool.h>

#include "wdlist.h"


//------------------------------------------------------------------------------

#define TPGUI_SCREEN_REFRESH_PERIOD		((portTickType)50 / portTICK_RATE_MS)	// [ms]
#define TPGUI_SCREEN_BLINKING_PERIOD	1000 									// [ms]
#define TPGUI_SCREEN_ANIMATION_PERIOD	1000 									// [ms]


#define TPGUI_SCREEN		0x01
#define TPGUI_MENU			0x02
#define TPGUI_FUNCTION		0x10



#define TPGUI_KEY_PRESS_OK			0x13
#define TPGUI_KEY_PRESS_EXIT		0x10
#define TPGUI_KEY_PRESS_UP			0x12
#define TPGUI_KEY_PRESS_DOWN		0x11

#define TPGUI_VAR_DATATYPE_INT		0x01
#define TPGUI_VAR_DATATYPE_FLOAT	0x02
#define TPGUI_VAR_DATATYPE_TIME		0x03
#define TPGUI_VAR_DATATYPE_DATE		0x04
#define TPGUI_VAR_DATATYPE_WDAY		0x05



enum TPGUI_SCREEN_ITEM
	{
	TPGUI_SI_LABEL=4,
	TPGUI_SI_VARIABLE,
	TPGUI_SI_BITMAP,


	};


enum TPGUI_MENU_ITEM
	{
	TPGUI_MI_LABEL,


	};



//------------------------------------------------------------------------------

typedef struct
	{
	unsigned char type;

	union
		{
		struct tpgui_screen_s *screen;
		struct tpgui_menu_s *menu;
		void (*function)(struct tpgui_screen_s *screen);

		} action;

	} tpgui_action_s;


typedef struct
	{
	unsigned char type;
	wdlist_s item_list;

	bool repeatedly_action_permitted;
	
	tpgui_action_s keyOK_action;
	tpgui_action_s keyEX_action;
	tpgui_action_s keyUP_action;
	tpgui_action_s keyDW_action;
	
	tpgui_action_s keyOKl_action;
	tpgui_action_s keyEXl_action;
	tpgui_action_s keyUPl_action;
	tpgui_action_s keyDWl_action;
	
	} tpgui_screen_s;


typedef struct
	{
	unsigned char type;
	wdlist_s item_list;

	bool changed;

	tpgui_action_s *up_menu;

	} tpgui_menu_s;


//------------------------------------------------------------------------------

typedef struct
	{
	enum TPGUI_SCREEN_ITEM type;
	bool changed;
	unsigned char attr;

	} tpgui_screen_item_s;


typedef struct
	{
	enum TPGUI_MENU_ITEM type;

	} tpgui_menu_item_s;



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// SCREEN ITEM

typedef struct
	{
	enum TPGUI_SCREEN_ITEM type;	// TPGUI_SI_LABEL
	bool changed;
	unsigned char attr;

	unsigned char col;
	unsigned char row;
	unsigned char len;		// rozmiar (w znakach)
	
	char *text;

	} tpgui_screen_item_label_s;

typedef struct
	{
	enum TPGUI_SCREEN_ITEM type;	// TPGUI_SI_VARIABLE
	bool changed;
	unsigned char attr;

	unsigned char col;
	unsigned char row;
	unsigned char len;		// rozmiar (w znakach)
	
	char *text;				// inicjowane w funkcji tpgui_screen_item_add

	//----------------------- powy¿ej jak tpgui_screen_item_label_s

	unsigned char data_type;
	void *data_ptr;
	char precision;


	} tpgui_screen_item_variable_s;

typedef struct
	{
	enum TPGUI_SCREEN_ITEM type;
	bool changed;	// not used
	unsigned char attr;

	unsigned char col;
	unsigned char row;
	unsigned char width;
	unsigned char height;

	unsigned char *data;
	unsigned char npage;
	unsigned char curr_page;


	} tpgui_screen_item_bmp_s;


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// MENU ITEM

typedef struct
	{
	enum TPGUI_MENU_ITEM type;
	unsigned char attr;

	unsigned char *text;
	tpgui_action_s *action;
	
	
	} tpgui_menu_item_label_s;



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------




void tpgui_run(void *start_screen);


void tpgui_screen_init(tpgui_screen_s *screen);

void tpgui_screen_item_add(tpgui_screen_s *screen, tpgui_screen_item_s *item);
void tpgui_screen_item_change_notify(tpgui_screen_item_s *item);


void tpgui_menu_init(tpgui_menu_s *menu);
void tpgui_menu_item_add(tpgui_menu_s *menu, tpgui_menu_item_s *item);



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#endif // __TPMENU_H__
