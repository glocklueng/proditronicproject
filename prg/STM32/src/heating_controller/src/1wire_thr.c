/*
 * 1wire_thr.c
 *
 *  Created on: 25-06-2013
 *      Author: Tomek
 */


#include "task.h"


#include "wdlist.h"
#include "ksystem.h"

#include "1wire.h"
#include "therm_ds18b20.h"


#if defined (__STM32__)
	#include "stm32f10x.h"
#endif


#include "1wire_thr.h"

//-----------------------------------------------------------------------------

thermometer_s thermometer_tab[4];
int nthermometers= 1;

wdlist_s thermometer_list;


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void prvOneWireTask(void *pvParameters)
	{

	onewire_handler_s onewire_chn1;
    wdlist_entry_s *therm_entry= NULL;
	thermometer_s *therm_new;
	thermometer_s *therm_curr;

    portTickType xLastWakeTime;
	k_uchar owire_resp;
	k_ushort temp_read;


	msleep(1000);

	wdlist_init(&thermometer_list);


	// 1-wire chn #1
	onewire_chn1.peripheral_addr= (uint32_t)GPIOB;
	onewire_chn1.data_pin= GPIO_Pin_1;
	onewire_chn1.strong_pull_up_enable= 0x00;
	onewire_bus_init(&onewire_chn1);


	therm_new= (thermometer_s *)malloc(sizeof(thermometer_s));
	therm_new->onewire_handler= &onewire_chn1;
    wdlist_append(&thermometer_list, (void *)therm_new);


	msleep(3000);


	xLastWakeTime= xTaskGetTickCount();

	while (1)
		{


		//msleep(3000);

		
		if (therm_entry == NULL)
			{
			therm_entry= thermometer_list.first_entry;

			if (therm_entry == NULL)
				break; // brak zdefiniowanych termometrów, zakoñcz w¹tek
			}


		therm_curr= (thermometer_s *)therm_entry->data;

		if (therm_curr->onewire_handler == NULL)
			goto prvOneWireTask_next;

		//---


		owire_resp= therm_ds18b20_conversion_start(therm_curr->onewire_handler, therm_entry->dev_id);

		if (owire_resp != 0x00)
			goto prvOneWireTask_next;

		msleep(800); // Tconv, max. 750 ms

		if (therm_curr->onewire_handler->strong_pull_up_enable)
			onewire_strong_pullup_enable(therm_curr->onewire_handler, 0x00); // strong pull-up OFF

		owire_resp= therm_ds18b20_temperature_read(therm_curr->onewire_handler, therm_entry->dev_id, &temp_read);

		if (owire_resp == 0x00)
			{


			}


		//---

prvOneWireTask_next:

		therm_entry= therm_entry->next;

		if (therm_entry == NULL)
			vTaskDelayUntil(&xLastWakeTime, (THERMOMETER_READ_PERIOD * 1000 / portTICK_RATE_MS));

		} // while (1)



	}




//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

