/*
 * 1wire_thr.c
 *
 *  Created on: 25-06-2013
 *      Author: Tomek
 */

#include <stdlib.h>
#include <string.h>


#include <FreeRTOS.h>
#include "task.h"
#include "semphr.h"


#include "wdlist.h"
#include "ksystem.h"

#include "1wire.h"
#include "therm_ds18b20.h"


#if defined (__STM32__)
	#include "stm32f10x.h"
#endif


#include "1wire_thr.h"

//-----------------------------------------------------------------------------


extern wdlist_s thermometer_list;
extern xSemaphoreHandle thermometer_sem;
extern onewire_handler_s onewire_chnlst[ONEWIRE_NBUS];

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void prvOneWireTask(void *pvParameters)
	{

    wdlist_entry_s *therm_entry= NULL;
	thermometer_s *therm_curr;

    portTickType xLastWakeTime;
	k_uchar owire_resp;
	k_ushort temp_read;


	msleep(1000);


	// 1-wire chn #1
	onewire_chnlst[0].peripheral_addr= (uint32_t)GPIOB;
	onewire_chnlst[0].data_pin= GPIO_Pin_1;
	onewire_chnlst[0].strong_pull_up_enable= 0x00;
	onewire_bus_init(&onewire_chnlst[0]);




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


		owire_resp= therm_ds18b20_conversion_start(therm_curr->onewire_handler, therm_curr->dev_id);

//		if (owire_resp != 0x00)
//			goto prvOneWireTask_next;

		msleep(800); // Tconv, max. 750 ms

		if (therm_curr->onewire_handler->strong_pull_up_enable)
			onewire_strong_pullup_enable(therm_curr->onewire_handler, 0x00); // strong pull-up OFF

		owire_resp= therm_ds18b20_temperature_read(therm_curr->onewire_handler, therm_curr->dev_id, &temp_read);

		if (owire_resp == 0x00)
			{
			xSemaphoreTake(thermometer_sem, 0);

			therm_curr->temp_value= temp_read;
			therm_curr->temp_vaild= true;
			therm_curr->temp_read_error_cntr= 0;

			xSemaphoreGive(thermometer_sem);
			}
		else
			{

			if (therm_curr->temp_read_error_cntr < (THERMOMETER_READ_ERROR_MAX - 1))
				therm_curr->temp_read_error_cntr++;
			else
			if (!therm_curr->temp_vaild)
				{
				xSemaphoreTake(thermometer_sem, 0);

				therm_curr->temp_vaild= false;

				xSemaphoreGive(thermometer_sem);
				}

			}


		//---

prvOneWireTask_next:

		therm_entry= therm_entry->next;

		if (therm_entry == NULL)
			{
			// zakoñczenie cylku odpytañ pomiaru temperatury
			k_uchar temp_queue_dta;


			temp_queue_dta= 0x00;
            xQueueSend(temp_pid_queue, &temp_queue_dta);

			msleep(5000);
			//vTaskDelayUntil(&xLastWakeTime, (THERMOMETER_READ_PERIOD * 1000 / portTICK_RATE_MS));
			}

		} // while (1)



	}




//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

