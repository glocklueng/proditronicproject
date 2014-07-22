

#include "FreeRTOS.h"
#include "task.h"

#include <time.h>

#include "lcd_interface.h"
#include "tpgui.h"
#include "keyboard.h"

#include "font5x7V.h"

//------------------------------------------------------------------------------

#define GLCD_NJU6450A // wywaliæ st¹d, przenieœæ gdzieœ do pliku konfiguracyjnego !!!


#if defined(GLCD_NJU6450A)
	#include "GLCD_NJU6450A.h"
#endif


extern lcd_handler_s lcd;


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------


#define BLINKING_CNTR_MAX	((TPGUI_SCREEN_BLINKING_PERIOD / TPGUI_SCREEN_REFRESH_PERIOD / 2) - 1)
#define ANIMATION_CNTR_MAX	((TPGUI_SCREEN_ANIMATION_PERIOD / TPGUI_SCREEN_REFRESH_PERIOD / 2) - 1)


//------------------------------------------------------------------------------


bool blinking_f;			// 1 - visible, 0 - not visible
bool blinking_change_f;		// state changed
bool animation_change_f;	// state changed

bool screen_update_f;

unsigned char npage;

unsigned char menu_first_visible_item_index;
unsigned char menu_chosen_item_index;

//------------------------------------------------------------------------------

void tpgui_thread(void *params);

void tpgui_screen_draw(tpgui_screen_s *gui_screen);
void tpgui_menu_draw(tpgui_menu_s *gui_menu);

tpgui_action_s *tpgui_menu_action(tpgui_menu_s *gui_menu, unsigned char menu_action);


void tpgui_x_label_draw(tpgui_screen_item_label_s *item);
void tpgui_x_bmp_draw(tpgui_screen_item_bmp_s *item);


void tpgui_prim_text_draw(tpgui_screen_item_label_s *item);;


void tpgui_prim_area_clean();





//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void tpgui_run(void *start_screen)
	{

	if (!start_screen)
		return;

	//tpgui_screen_init(&main_screen);
/*
	main_screen_label1.type= TPGUI_SI_LABEL;
	main_screen_label1.attr= 0x00;
	main_screen_label1.col= 0;
	main_screen_label1.row= 0;
	main_screen_label1.len= 10;
	main_screen_label1.text= "napistestowy";

	tpgui_screen_item_add(&main_screen, (tpgui_screen_item_s *)&main_screen_label1);
*/


	xTaskCreate(tpgui_thread, "gui", 512, (void *)start_screen, tskIDLE_PRIORITY, NULL);

	}

//------------------------------------------------------------------------------

void tpgui_thread(void *params)
	{
	void *current_screen= params;
	bool lcd_reset= true;

	unsigned char blinking_cntr;
	unsigned char animation_cntr;


	unsigned char key_pressed;
	tpgui_action_s *user_action;
	key_s key_info;



	blinking_cntr= BLINKING_CNTR_MAX;
	blinking_f= true;
	blinking_change_f= false;

	animation_cntr= ANIMATION_CNTR_MAX;
	animation_change_f= false;

	screen_update_f= true;






	while (1)
		{

		if (lcd_reset)
			{
			lcd.init();
			lcd.screen_clear();

			lcd.contrast_set(40);
			lcd.backlight_set(40);

			lcd_reset= false;
			}

        vTaskDelay(TPGUI_SCREEN_REFRESH_PERIOD);


		user_action= NULL;




// obs³uga klawiszy

		if (keyboard_key_get(&key_info))
			{

			printf("key: %02X %02X\n", key_info.key_code, key_info.key_action);


			switch (((tpgui_screen_s *)current_screen)->type)
				{

				case TPGUI_SCREEN:
					{
					tpgui_screen_s *screen= (tpgui_screen_s *)current_screen;

					switch (key_info.key_code)
						{

						case TPGUI_KEY_PRESS_EXIT:
							{
							if ((key_info.key_action == KEY_ACTION_SHORT_PRESSED) || (screen->repeatedly_action_permitted && (key_info.key_action == KEY_ACTION_REPEATEDLY_PRESSED)))
								user_action= screen->keyEX_action;
							else
							if (key_info.key_action == KEY_ACTION_LONG_PRESSED)
								user_action= screen->keyEXl_action;
							break;
							}

						case TPGUI_KEY_PRESS_UP:
							{
							if ((key_info.key_action == KEY_ACTION_SHORT_PRESSED) || (screen->repeatedly_action_permitted && (key_info.key_action == KEY_ACTION_REPEATEDLY_PRESSED)))
								user_action= screen->keyUP_action;
							else
							if (key_info.key_action == KEY_ACTION_LONG_PRESSED)
								user_action= screen->keyUPl_action;
							break;
							}

						case TPGUI_KEY_PRESS_DOWN:
							{
							if ((key_info.key_action == KEY_ACTION_SHORT_PRESSED) || (screen->repeatedly_action_permitted && (key_info.key_action == KEY_ACTION_REPEATEDLY_PRESSED)))
								user_action= screen->keyDW_action;
							else
							if (key_info.key_action == KEY_ACTION_LONG_PRESSED)
								user_action= screen->keyDWl_action;
							break;
							}

						case TPGUI_KEY_PRESS_OK:
							{
							if ((key_info.key_action == KEY_ACTION_SHORT_PRESSED) || (screen->repeatedly_action_permitted && (key_info.key_action == KEY_ACTION_REPEATEDLY_PRESSED)))
								user_action= screen->keyOK_action;
							else
							if (key_info.key_action == KEY_ACTION_LONG_PRESSED)
								user_action= screen->keyOKl_action;
							break;
							}

						} // switch (key_info.key_code)

					break;
					} // TPGUI_SCREEN

				case TPGUI_MENU:
					{
					tpgui_menu_s *menu= (tpgui_menu_s *)current_screen;
					unsigned char menu_action= 0x00;

					switch (key_info.key_code)
						{

						case TPGUI_KEY_PRESS_EXIT:
							{
							if (key_info.key_action == KEY_ACTION_SHORT_PRESSED)
								user_action= menu->up_menu;
							else
                            if (key_info.key_action == KEY_ACTION_LONG_PRESSED)
								user_action= NULL; /// main screen
							break;
							}

						case TPGUI_KEY_PRESS_UP:
							{
							if ((key_info.key_action == KEY_ACTION_SHORT_PRESSED) || (key_info.key_action == KEY_ACTION_LONG_PRESSED) || (key_info.key_action == KEY_ACTION_REPEATEDLY_PRESSED))
								menu_action= TPGUI_KEY_PRESS_UP;
							break;
							}

						case TPGUI_KEY_PRESS_DOWN:
							{
							if ((key_info.key_action == KEY_ACTION_SHORT_PRESSED) || (key_info.key_action == KEY_ACTION_LONG_PRESSED) || (key_info.key_action == KEY_ACTION_REPEATEDLY_PRESSED))
								menu_action= TPGUI_KEY_PRESS_DOWN;
							break;
							}

						case TPGUI_KEY_PRESS_OK:
							{
							if (key_info.key_action == KEY_ACTION_SHORT_PRESSED)
								menu_action= TPGUI_KEY_PRESS_OK;
							break;
							}

						} // switch (key_info.key_code)

					if (menu_action)
						user_action= tpgui_menu_action(menu, menu_action);

					break;
					} // TPGUI_MENU

				default:
					break;

				} // (tpgui_screen_s *)current_screen->type

			} // keyboard_key_get



		if (user_action)
			{

			switch (user_action->type)
				{

				case TPGUI_SCREEN:
					{
					// prze³¹czenie ekranu

					if (user_action->action.screen)
						{
						current_screen= (void *)user_action->action.screen;
						screen_update_f= true;
						}

					break;
					} // TPGUI_SCREEN, TPGUI_MENU

				case TPGUI_MENU:
					{
					// prze³¹czenie ekranu

					if (user_action->action.menu)
						{
						current_screen= (void *)user_action->action.menu;
						screen_update_f= true;
						}

					break;
					} // TPGUI_MENU

				case TPGUI_FUNCTION:
					{
					// wywo³anie funkcji

					if (user_action->action.function)
						{
				//		*user_action->action.function(current_screen);
						}

					break;
					} // TPGUI_SCREEN

				} // switch (user_action->type)

			} // if (user_action)



// obs³uga wyœwietlania

		if (screen_update_f)
			{
			blinking_cntr= BLINKING_CNTR_MAX;
			blinking_f= true;
			blinking_change_f= false;

			animation_cntr= ANIMATION_CNTR_MAX;
			animation_change_f= false;

			lcd.screen_clear();
			}


		switch (((tpgui_screen_s *)current_screen)->type)
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


		if (screen_update_f)
			{
			// w³¹cz ekran !!!
			}



		screen_update_f= false;
		blinking_change_f= false;
		

		if (blinking_cntr != 0)
			blinking_cntr-= 1;
		else
			{
			blinking_cntr= BLINKING_CNTR_MAX;
			blinking_f= !blinking_f;
			blinking_change_f= true;
			}

		if (animation_cntr != 0)
			animation_cntr-= 1;
		else
			{
			animation_cntr= ANIMATION_CNTR_MAX;
			animation_change_f= true;
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

	if (!screen || !item)
		return;

	item->changed= true;

	switch (item->type)
		{

		case TPGUI_SI_VARIABLE:
			{
			tpgui_screen_item_variable_s *xitem= (tpgui_screen_item_variable_s *)item;
			xitem->text= (char *)malloc(xitem->len + 1);
			break;
			}

		} // switch (item->type)


	wdlist_append(&screen->item_list, (void *)item);
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

		switch (xitem->type)
			{

			case TPGUI_SI_LABEL:
				{

				if (screen_update_f || xitem->changed || (blinking_change_f && (xitem->attr & TPGUI_ITEM_ATTRIB_BLINKING)))
					{
					tpgui_x_label_draw((tpgui_screen_item_label_s *)xitem);
					xitem->changed= false;
					}

				break;
				} // TPGUI_SI_LABEL


			case TPGUI_SI_VARIABLE:
				{
				tpgui_screen_item_variable_s *item= (tpgui_screen_item_variable_s *)xitem;

				if (screen_update_f || xitem->changed || (blinking_change_f && (xitem->attr & TPGUI_ITEM_ATTRIB_BLINKING)))
					{

					switch (item->data_type)
						{

						case TPGUI_VAR_DATATYPE_INT:
							{
							sprintf(item->text, "%*d", item->len, *(int *)item->data_ptr);
							break;
							}

						case TPGUI_VAR_DATATYPE_FLOAT:
							{
							sprintf(item->text, "%*.*f", item->len, item->precision, *(float *)item->data_ptr);
							break;
							}

						case TPGUI_VAR_DATATYPE_TIME:
							{
							strftime(item->text, item->len + 1, "%H:%M:%S", (struct tm *)item->data_ptr);
							break;
							}

						case TPGUI_VAR_DATATYPE_DATE:
							{
							strftime(item->text, item->len + 1, "%Y-%m-%d", (struct tm *)item->data_ptr);
							break;
							}

						case TPGUI_VAR_DATATYPE_WDAY:
							{
							break;
							}


						} // switch (item->data_type)

					tpgui_x_label_draw((tpgui_screen_item_label_s *)xitem);
					xitem->changed= false;

					} // if (screen_update_f || xitem->changed || (blinking_change_f && (xitem->attr & TPGUI_ITEM_ATTRIB_BLINKING)))

				break;
				} // TPGUI_SI_VARIABLE


			case TPGUI_SI_BITMAP:
				{
				tpgui_screen_item_bmp_s *item= (tpgui_screen_item_bmp_s *)xitem;

				if (screen_update_f || animation_change_f || (blinking_change_f && (xitem->attr & TPGUI_ITEM_ATTRIB_BLINKING)))
					{

					if (screen_update_f)
						item->curr_page= 0;

					tpgui_x_bmp_draw((tpgui_screen_item_bmp_s *)xitem);

					} // if (screen_update_f || xitem->changed)

				break;
				} // TPGUI_SI_BITMAP

			} // switch (xitem->type)
		


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

void tpgui_x_bmp_draw(tpgui_screen_item_bmp_s *item)
	{

	if ((item->npage == 0) || (item->width == 0) || (item->height == 0))
		return;

	if (item->curr_page >= npage)
		item->curr_page= 0;


	if ((item->attr & TPGUI_ITEM_ATTRIB_BLINKING) && !blinking_f)
		{
		// wyczyœæ
		lcd.region_fill(item->col + 1, item->row, item->width, item->height, 0x00);
		}
	else
		{
		unsigned char *page_data= (npage == 1) ? item->data : &item->data[item->curr_page * item->width * item->height];
		lcd.bmp_draw(item->col + 1, item->row, item->width, item->height, item->attr, page_data);

		item->curr_page= (item->curr_page < (item->npage - 1)) ? (item->curr_page + 1) : 0;
		}

	}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void tpgui_menu_init(tpgui_menu_s *menu)
	{

	if (!menu)
		return;

	wdlist_init(&menu->item_list);

	}

//------------------------------------------------------------------------------

void tpgui_menu_item_add(tpgui_menu_s *menu, tpgui_menu_item_s *item)
	{

	if (!menu || !item)
		return;

	wdlist_append(&menu->item_list, (void *)item);
	}

//------------------------------------------------------------------------------

void tpgui_menu_draw(tpgui_menu_s *gui_menu)
	{
	wdlist_entry_s *wdlist_entry;
	unsigned char item_index= 0;
	unsigned char row= 0;
	unsigned char item_drawn= 0;


	if (!gui_menu)
		return;

	if (screen_update_f)
		{
		menu_first_visible_item_index= 0;
		menu_chosen_item_index= 0;
		}

	if (screen_update_f || gui_menu->changed)
		{

		printf("line: %d\n", __LINE__);

		wdlist_entry= gui_menu->item_list.first_entry;
		while (wdlist_entry)
			{

			printf("line: %d\n", __LINE__);

			if ((item_index >= menu_first_visible_item_index) && (item_index < (menu_first_visible_item_index + lcd.rows)))
				{
				tpgui_menu_item_s *xitem= (tpgui_menu_item_s *)wdlist_entry->data;
				bool chosen= (menu_chosen_item_index == item_index);

				item_drawn+= 1;

				printf("line: %d  %d\n", __LINE__, item_drawn);

				switch (xitem->type)
					{

					case TPGUI_MI_LABEL:
						{
						tpgui_menu_item_label_s *item=(tpgui_menu_item_label_s *)xitem;
						tpgui_screen_item_label_s item_tmp;

						item_tmp.attr= chosen ? TPGUI_ITEM_ATTRIB_INVERT : 0x00;
						item_tmp.col= 0;
						item_tmp.row= row;
						item_tmp.text= item->text;
						item_tmp.len= SCREEN_COLS;

						tpgui_x_label_draw(&item_tmp);
						row+= 1;

						break;
						}

					} // switch (xitem->type)

				} // if ((item_index ...
			else
				{
				printf("line: %d\n", __LINE__);
				//break;
				}

			item_index+= 1;
			wdlist_entry= wdlist_entry->next;


			if (item_drawn == lcd.rows)
				{
				printf("line: %d  break\n", __LINE__);
				break;
				}

			} // while (wdlist_entry)

		gui_menu->changed= false;

		} // if (screen_update_f || gui_menu->changed)

	}

//------------------------------------------------------------------------------

tpgui_action_s *tpgui_menu_action(tpgui_menu_s *gui_menu, unsigned char menu_action)
	{
	tpgui_action_s *action= NULL;
	wdlist_entry_s *wdlist_entry;
	unsigned char item_index= 0;
	unsigned char cur_pos; // w zakresie wyœwietlacza

	printf("line: %d\n", __LINE__);

	if (!gui_menu || (gui_menu->item_list.entries_number < 1))
		return NULL;

	printf("line: %d\n", __LINE__);

	if (screen_update_f)
		{
		menu_first_visible_item_index= 0;
		menu_chosen_item_index= 0;
		}


	cur_pos= menu_chosen_item_index - menu_first_visible_item_index;

	switch (menu_action)
		{

		case TPGUI_KEY_PRESS_OK:
			{

			if (menu_chosen_item_index < gui_menu->item_list.entries_number)
				{
				wdlist_entry= gui_menu->item_list.first_entry;
				while (wdlist_entry)
					{

					if (menu_chosen_item_index == item_index)
						{
						tpgui_menu_item_s *xitem= (tpgui_menu_item_s *)wdlist_entry->data;

						switch (xitem->type)
							{

							case TPGUI_MI_LABEL:
								{
								tpgui_menu_item_label_s *tpgui_menu_item_label= (tpgui_menu_item_label_s *)xitem;
								action= tpgui_menu_item_label->action;
								break;
								} // TPGUI_MI_LABEL

							default:
								break;

							} // switch (xitem->type)

						break;
						}

					item_index+= 1;
					wdlist_entry= wdlist_entry->next;
					} // while (wdlist_entry)

				}

			break;
			} // TPGUI_KEY_PRESS_OK

		case TPGUI_KEY_PRESS_UP:
			{

			if (cur_pos != 0)
				{
				// przesuniêcie paska podœwietlenia w górê
				menu_chosen_item_index-= 1;
				gui_menu->changed= true;
				}
			else
			if (menu_chosen_item_index > 0)
				{
				// przesuniêcie listy
				menu_chosen_item_index-= 1;
				menu_first_visible_item_index-= 1;
				gui_menu->changed= true;
				}

			break;
			} // TPGUI_KEY_PRESS_DOWN

		case TPGUI_KEY_PRESS_DOWN:
			{

			printf("line: %d\n", __LINE__);

			if ((cur_pos < (lcd.rows - 1)) && (menu_chosen_item_index < (gui_menu->item_list.entries_number - 1)))
				{
				// przesuniêcie paska podœwietlenia w dó³
				menu_chosen_item_index+= 1;
				gui_menu->changed= true;
				printf("line: %d\n", __LINE__);

				}
			else
			if (menu_chosen_item_index < (gui_menu->item_list.entries_number - 1))
				{
				// przesuniêcie listy
				menu_chosen_item_index+= 1;
				menu_first_visible_item_index+= 1;
				gui_menu->changed= true;
				printf("line: %d\n", __LINE__);

				}

			break;
			} // TPGUI_KEY_PRESS_DOWN

		default:
			break;
		} // switch (menu_action)


	return action;
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

