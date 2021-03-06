#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include "stm32f10x.h"

#include "tpgui.h"
#include "wdlist.h"
#include "keyboard.h"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#define KEYB_SAMPLING_PERIOD				20		// [ms]
#define KEYB_SHORT_PUSH_TIME				20		// [ms]
#define KEYB_LONG_PUSH_TIME					500
#define KEYB_TIME_BEFORE_REPEATEDLY			500
#define KEYB_PUSH_REPEATEDLY_TIME			500



#define BUTTON_STATE_RELEASED				0x00
#define BUTTON_STATE_PUSHED					0x01
#define BUTTON_STATE_BEFORE_REPEATEDLY		0x02
#define BUTTON_STATE_PUSHED_REPEATEDLY		0x03


//------------------------------------------------------------------------------


typedef struct
	{
	uint8_t key_code;

	uint32_t peripheral_addr;
	uint16_t key_pin;

	uint8_t state;
	uint8_t cntr;

	} keydef_s;

//------------------------------------------------------------------------------


wdlist_s key_list;
xQueueHandle key_queue;
key_s key_event;

struct tm utime_tm;
time_t czas, czas_prev;
extern tpgui_screen_item_variable_s screen_1_time_label;


//------------------------------------------------------------------------------

void keyboard_thread(void *params);





//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void keyboard_init()
	{
    key_queue= xQueueCreate(1, sizeof(key_s));
	wdlist_init(&key_list);
	}

//------------------------------------------------------------------------------

void keyboard_run()
	{
    xTaskCreate(keyboard_thread, "key", 512, NULL, tskIDLE_PRIORITY, NULL);
	}

//------------------------------------------------------------------------------

void keyboard_key_add(uint8_t key_code, uint32_t peripheral_addr, uint16_t key_pin)
	{
	GPIO_InitTypeDef GPIO_InitStructure;
	keydef_s *key;

	key= (keydef_s *)malloc(sizeof(keydef_s));

	key->key_code= key_code;
	key->peripheral_addr= peripheral_addr;
	key->key_pin= key_pin;

	key->state= BUTTON_STATE_RELEASED;
	key->cntr= 0;


	wdlist_append(&key_list, (void *)key);

	// setup
	GPIO_InitStructure.GPIO_Pin= key_pin;
	GPIO_InitStructure.GPIO_Speed= GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode= GPIO_Mode_IPU;
	GPIO_Init((GPIO_TypeDef *)peripheral_addr, &GPIO_InitStructure);
	GPIO_SetBits((GPIO_TypeDef *)peripheral_addr, key_pin);

	}

//------------------------------------------------------------------------------

bool keyboard_key_get(key_s *key)
	{
	if (!key)
		return false;

	return xQueueReceive(key_queue, key, 0);
	}

//------------------------------------------------------------------------------

void keyboard_thread(void *params)
	{
    wdlist_entry_s *entry;
	keydef_s *key;
	GPIO_TypeDef *gpio;
	bool pushing;

	vTaskDelay(1000); // czekam na podci�gni�cie wej�� do plusa

	czas_prev= -1;

	while (1)
		{

        vTaskDelay(KEYB_SAMPLING_PERIOD);


        czas= time(0);

        if (czas != czas_prev)
        	{

            printf("czas: %d\n", czas);

        	localtime_r(&czas, &utime_tm);
        	tpgui_screen_item_change_notify(&screen_1_time_label);

        	czas_prev= czas;
        	}


        //localtime_r(&czas, &utime_tm);




		entry= key_list.first_entry;
		while (entry)
			{
			key= (keydef_s *)entry->data;
			gpio= (GPIO_TypeDef *)key->peripheral_addr;

			pushing= (gpio->IDR & key->key_pin) ? 0 : 1; // 1 - push


			if (pushing)
				{
				// pushing

				key->cntr+= 1;

				switch (key->state)
					{

					case BUTTON_STATE_RELEASED:
						{
						key->state= BUTTON_STATE_PUSHED;
						key->cntr= 0;
						break;
						}

					case BUTTON_STATE_PUSHED:
						{

						if (key->cntr >= (KEYB_LONG_PUSH_TIME / KEYB_SAMPLING_PERIOD))
							{
							key->state= BUTTON_STATE_BEFORE_REPEATEDLY;
							key->cntr= 0;
							
							key_event.key_code= key->key_code;
							key_event.key_action= KEY_ACTION_LONG_PRESSED;
                            xQueueSend(key_queue, &key_event, 0);

							printf("PUSHED: KEY_ACTION_LONG_PRESSED\n");
							}

						break;
						}

					case BUTTON_STATE_BEFORE_REPEATEDLY:
						{

						if (key->cntr >= (KEYB_TIME_BEFORE_REPEATEDLY / KEYB_SAMPLING_PERIOD))
							{
							key->state= BUTTON_STATE_PUSHED_REPEATEDLY;
							key->cntr= 0;
							}

						break;
						}

					case BUTTON_STATE_PUSHED_REPEATEDLY:
						{

						if (key->cntr >= (KEYB_PUSH_REPEATEDLY_TIME / KEYB_SAMPLING_PERIOD))
							{
							key->cntr= 0;

							key_event.key_code= key->key_code;
							key_event.key_action= KEY_ACTION_REPEATEDLY_PRESSED;
							xQueueSend(key_queue, &key_event, 0);

							printf("PUSHED: KEY_ACTION_REPEATEDLY_PRESSED\n");
							}

						break;
						}

					default:
						{
						key->state= BUTTON_STATE_RELEASED;
						key->cntr= 0;
						break;
						}

					} // switch (key->state)

				} // pushing
			else
				{
				// releasing

				switch (key->state)
					{

					case BUTTON_STATE_PUSHED:
						{

						if (key->cntr >= (KEYB_LONG_PUSH_TIME / KEYB_SAMPLING_PERIOD))
							{
							key_event.key_code= key->key_code;
							key_event.key_action= KEY_ACTION_LONG_PRESSED;
							xQueueSend(key_queue, &key_event, 0);


							printf("RELEASED: KEY_ACTION_LONG_PRESSED\n");
							}
						else
						if (key->cntr >= (KEYB_SHORT_PUSH_TIME / KEYB_SAMPLING_PERIOD))
							{
							key_event.key_code= key->key_code;
							key_event.key_action= KEY_ACTION_SHORT_PRESSED;
							xQueueSend(key_queue, &key_event, 0);

							printf("RELEASED: KEY_ACTION_SHORT_PRESSED\n");
							}

						break;
						}

					case BUTTON_STATE_PUSHED_REPEATEDLY:
						{

						if (key->cntr >= (KEYB_PUSH_REPEATEDLY_TIME / KEYB_SAMPLING_PERIOD))
							{
							key_event.key_code= key->key_code;
							key_event.key_action= KEY_ACTION_REPEATEDLY_PRESSED;
							xQueueSend(key_queue, &key_event, 0);

							printf("RELEASED: KEY_ACTION_REPEATEDLY_PRESSED\n");
							}

						break;
						}

					default:
						{
						key->state= BUTTON_STATE_RELEASED;
						key->cntr= 0;
						break;
						}

					} // switch (key->state)


				key->state= BUTTON_STATE_RELEASED;
				key->cntr= 0;

				} // releasing


			entry= entry->next;
			} // while (entry)


		} // while (1)


	}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

