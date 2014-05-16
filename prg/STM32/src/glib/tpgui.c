

#include "FreeRTOS.h"
#include "task.h"



#include "lcd_interface.h"
#include "tpgui.h"


//------------------------------------------------------------------------------

#define GLCD_NJU6450A // wywaliæ st¹d, przenieœæ gdzieœ do pliku konfiguracyjnego !!!


#if defined(GLCD_NJU6450A)
	#include "GLCD_NJU6450A.h"
#endif


extern lcd_handler_s lcd;


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#define BLINKING_CNTR_MAX	((TOGUI_SCREEN_BLINKING_PERIOD / TPGUI_SCREEN_REFRESH_PERIOD / 2) - 1)


//------------------------------------------------------------------------------


bool blinking_f;			// 1 - visible, 0 - not visible
bool blinking_change_f;		// state changed

bool screen_update_f;
unsigned char menu_chosen_item_index;

//------------------------------------------------------------------------------

void tpgui_thread(void *params);

void tpgui_screen_draw(tpgui_screen_s *gui_screen);

void tpgui_menu_draw(tpgui_menu_s *gui_menu);
tpgui_action_s *tpgui_menu_action(tpgui_menu_s *gui_menu, unsigned char action);

void tpgui_x_label_draw(tpgui_screen_item_label_s *item);



void tpgui_prim_text_draw(tpgui_screen_item_label_s *item);


void tpgui_prim_area_clean();




//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void tpgui_run(void *start_screen)
	{

	if (!start_screen)
		return;

	lcd.init();
	lcd.screen_clear();


    xTaskCreate(tpgui_thread, "gui", STACK_SIZE, (void *)start_screen, tskIDLE_PRIORITY, NULL);

	}

//------------------------------------------------------------------------------

void tpgui_thread(void *params)
	{
	void *current_screen= params;
	unsigned char blinking_cntr;

	unsigned char key_pressed;
	unsigned char key_long_pressed;
	tpgui_action_s *user_action;

	blinking_cntr= BLINKING_CNTR_MAX;
	blinking_f= true;
	blinking_change_f= false;

	screen_update_f= true;



	while (1)
		{

        vTaskDelay(TPGUI_SCREEN_REFRESH_PERIOD);

		user_action= NULL;


// obs³uga klawiszy

		key_pressed= tpgui_key_pressed();

		if (key_pressed)
			{
			key_long_pressed= tpgui_key_long_pressed();

			switch ((tpgui_screen_s *)current_screen->type)
				{

				case TPGUI_SCREEN:
					{
					tpgui_screen_s *screen= (tpgui_screen_s *)current_screen;

					if (key_pressed & TPGUI_KEY_PRESS_EXIT)
						user_action= (key_long_pressed & TPGUI_KEY_PRESS_EXIT) ? screen->keyEXl_action : screen->keyEX_action;
					else
					if (key_pressed & TPGUI_KEY_PRESS_UP)
						user_action= (key_long_pressed & TPGUI_KEY_PRESS_UP) ? screen->keyUPl_action : screen->keyUP_action;
					else
					if (key_pressed & TPGUI_KEY_PRESS_DOWN)
						user_action= (key_long_pressed & TPGUI_KEY_PRESS_DOWN) ? screen->keyDWl_action : screen->keyDW_action;
					else
					if (key_pressed & TPGUI_KEY_PRESS_OK)
						user_action= (key_long_pressed & TPGUI_KEY_PRESS_OK) ? screen->keyOKl_action : screen->keyOK_action;

					break;
					} // TPGUI_SCREEN

				case TPGUI_MENU:
					{
					tpgui_menu_s *menu= (tpgui_menu_s *)current_screen;
					unsigned char menu_action= 0x00;

					if (key_pressed & TPGUI_KEY_PRESS_EXIT)
						menu_action= TPGUI_KEY_PRESS_EXIT;
					else
					if (key_pressed & TPGUI_KEY_PRESS_UP)
						menu_action= TPGUI_KEY_PRESS_UP;
					else
					if (key_pressed & TPGUI_KEY_PRESS_DOWN)
						menu_action= TPGUI_KEY_PRESS_DOWN;
					else
					if (key_pressed & TPGUI_KEY_PRESS_OK)
						menu_action= TPGUI_KEY_PRESS_OK;

					if (menu_action)
						user_action= tpgui_menu_action(menu, menu_action);

					break;
					} // TPGUI_MENU

				default:
					break;

				} // (tpgui_screen_s *)current_screen->type

			} // if (key_pressed)


		if (user_action)
			{

			switch ()
				{
				}

			} // if (user_action)


// obs³uga wyœwietlania

		switch ((tpgui_screen_s *)current_screen->type)
			{

			case TPGUI_SCREEN:
				{
				tpgui_screen_draw((tpgui_screen_s *)current_screen);
				break;
				} // TPGUI_SCREEN


			case TPGUI_MENU:
				{
				tpgui_menu_draw((tpgui_menu_s *)current_screen);
				break;
				} // TPGUI_MENU
		

			} // switch ((tpgui_screen_s *)current_screen->type)


		screen_update_f= false;
		blinking_change_f= false;


		

		// jeœli zmiana ekranu, wymuœ !!!:
		// screen_update_f= true;



		if (!blinking_cntr)
			blinking_cntr-= 1;
		else
			{
			blinking_cntr= BLINKING_CNTR_MAX;
			blinking_f= !blinking_f;
			blinking_change_f= true;
			}


		} // while (1)


	}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void tpgui_screen_init(tpgui_screen_s *screen)
	{

	if (!screen)
		return;

	wdlist_init(&screen->item_list);

	}

//------------------------------------------------------------------------------

void tpgui_screen_item_add(tpgui_screen_s *screen, tpgui_screen_item_s *item)
	{
	wdlist_entry_s *wdlist_entry;

	if (!screen || !item)
		return;

	item->changed= true;

	wdlist_entry= (wdlist_entry_s *)malloc(sizeof(wdlist_entry_s));
	wdlist_entry->data= (void *)item;
	wdlist_append(&screen->item_list, wdlist_entry);

	}

//------------------------------------------------------------------------------

void tpgui_screen_draw(tpgui_screen_s *gui_screen)
	{
	wdlist_entry_s *wdlist_entry;

	if (!gui_screen)
		return;

	wdlist_entry= gui_screen->item_list.first_entry;
	while (wdlist_entry)
		{
		tpgui_screen_item_s *xitem= (tpgui_screen_item_s *)wdlist_entry->data;

		if (screen_update_f || xitem->changed || (blinking_change_f && (xitem->attr & TPGUI_ITEM_ATTRIB_BLINKING)))
			{

			switch (xitem->type)
				{

				case TPGUI_SI_LABEL:
					{
					tpgui_x_label_draw((tpgui_screen_item_label_s *)xitem);
					break;
					} // TPGUI_SI_LABEL



				} // switch (xitem->type)

			xitem->changed= false;

			} // if (screen_update_f ...)

		wdlist_entry= wdlist_entry->next;
		} // while (wdlist_entry)

	}

//------------------------------------------------------------------------------

void tpgui_x_label_draw(tpgui_screen_item_label_s *item)
	{

	if ((item->attr & TPGUI_ITEM_ATTRIB_BLINKING) && !blinking_f)
		{
		// wyczyœæ

		if (!(item->attr & TPGUI_ITEM_ATTRIB_DOUBLESIZE))
			lcd.region_fill(item->col * FONT_CHAR_WIDTH + 1, item->row, item->len * FONT_CHAR_WIDTH, 1, 0x00);
		else
            lcd.region_fill(item->col * FONT_CHAR_WIDTH + 1, item->row, item->len * FONT_CHAR_WIDTH * 2, 2, 0x00);
		}
	else
		tpgui_prim_text_draw(item); // rysuj

	}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void tpgui_menu_draw(tpgui_menu_s *gui_menu)
	{
	wdlist_entry_s *wdlist_entry;
	unsigned char item_index= 0;

	if (!gui_menu)
		return;

	if (screen_update_f)
		menu_chosen_item_index= 0;

	if (screen_update_f || gui_menu->changed)
		{

		wdlist_entry= gui_menu->item_list.first_entry;
		while (wdlist_entry)
			{
			tpgui_menu_item_s *xitem= (tpgui_menu_item_s *)wdlist_entry->data;
			bool chosen= (menu_chosen_item_index == item_index);
	
			switch (xitem->type)
				{

				case TPGUI_MI_LABEL:
					{


					break;
					}


				
				} // switch (xitem->type)

			item_index+= 1;
			wdlist_entry= wdlist_entry->next;

			} // while (wdlist_entry)

		gui_menu->changed= false;

		} // if (screen_update_f || gui_menu->changed)

	}

//------------------------------------------------------------------------------

tpgui_action_s *tpgui_menu_action(tpgui_menu_s *gui_menu, unsigned char action)
	{

	return NULL;
	}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void tpgui_prim_text_draw(tpgui_screen_item_label_s *item)
	{
	unsigned char x, len;
	unsigned char col;

	unsigned char *charptr;
	bool clear= false;

	if (!item->text)
		return;
    
	charptr= item->text;
	col= item->col;
    
	for (x=0;x<item->len;x++)
		{
		if (*charptr == 0x00)
			clear= true;

		if (!clear)
			lcd.char_draw(*charptr++, col, item->row, item->attr);
		else
			lcd.char_draw(0x20, col, item->row, item->attr); // space

		col+= (item->attr & TPGUI_ITEM_ATTRIB_DOUBLESIZE) ? 2 : 1;
		}

	}

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------

void tpgui_prim_area_clean()
	{


	}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

