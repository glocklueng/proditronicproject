/*
 * 1wire_thr.c
 *
 *  Created on: 25-06-2013
 *      Author: Tomek
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>


#include <FreeRTOS.h>
#include "task.h"
#include "semphr.h"
#include "queue.h"


#include "wdlist.h"
#include "ksystem.h"

#include "1wire.h"
#include "therm_ds18b20.h"
#include "global.h"
#include "tempctrlpid_thr.h"


#if defined (__STM32__)
	#include "stm32f10x.h"
#endif


#include "1wire_thr.h"

//-----------------------------------------------------------------------------

const char thrname[]= "1wire";


//-----------------------------------------------------------------------------

extern wdlist_s thermometer_list;
extern xSemaphoreHandle thermometer_sem;
extern onewire_handler_s onewire_chnlst[ONEWIRE_NBUS];
extern xQueueHandle temp_pid_queue;
extern wdlist_s heater_list;
extern unsigned char *cmd_inter_strtok_del;

//-----------------------------------------------------------------------------

char debug_txt[17];
char dev_id_str[13];
char dbgtmp[3];
float temp_dbg;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


void prvOneWireTask(void *pvParameters)
	{

    wdlist_entry_s *list_entry= NULL;
	thermometer_s *therm_curr;

    portTickType xLastWakeTime;
	k_uchar owire_resp;
	k_short temp_read;

	float temp_avr_f;
	k_short temp_s, temp_avr;

	int x;
	//char test= 0;


	msleep(1000);



	// 1-wire chn #1
	onewire_chnlst[0].peripheral_addr= (uint32_t)GPIOD;
	onewire_chnlst[0].data_pin= GPIO_Pin_8;
	onewire_chnlst[0].strong_pull_up_enable= 0x01;
	onewire_chnlst[0].strong_pull_up_pin= GPIO_Pin_9;
	onewire_chnlst[0].chn_no= 0;
	onewire_bus_init(&onewire_chnlst[0]);

	// 1-wire chn #2
	onewire_chnlst[1].peripheral_addr= (uint32_t)GPIOD;
	onewire_chnlst[1].data_pin= GPIO_Pin_10;
	onewire_chnlst[1].strong_pull_up_enable= 0x01;
	onewire_chnlst[1].strong_pull_up_pin= GPIO_Pin_11;
	onewire_chnlst[1].chn_no= 1;
	onewire_bus_init(&onewire_chnlst[1]);

	// 1-wire chn #3
	onewire_chnlst[2].peripheral_addr= (uint32_t)GPIOD;
	onewire_chnlst[2].data_pin= GPIO_Pin_12;
	onewire_chnlst[2].strong_pull_up_enable= 0x01;
	onewire_chnlst[2].strong_pull_up_pin= GPIO_Pin_13;
	onewire_chnlst[2].chn_no= 2;
	onewire_bus_init(&onewire_chnlst[2]);

	// 1-wire chn #4
	onewire_chnlst[3].peripheral_addr= (uint32_t)GPIOC;
	onewire_chnlst[3].data_pin= GPIO_Pin_6;
	onewire_chnlst[3].strong_pull_up_enable= 0x01;
	onewire_chnlst[3].strong_pull_up_pin= GPIO_Pin_7;
	onewire_chnlst[3].chn_no= 3;
	onewire_bus_init(&onewire_chnlst[3]);


	list_entry= thermometer_list.first_entry;
	while (list_entry)
		{
		therm_curr= (thermometer_s *)list_entry->data;

		dev_id_str[0]= 0x00;
		for (x=0;x<6;x++)
			{
			sprintf(dbgtmp, "%02X", therm_curr->dev_id[x]);
			strcat(dev_id_str, dbgtmp);
			}

		printf("<TRM> dev[%02d]: 1wire chn[%d] id[%s]\n", therm_curr->indx, therm_curr->onewire_handler->chn_no, dev_id_str);

		list_entry= list_entry->next;
		}

	printf("\n");


	list_entry= heater_list.first_entry;
	while (list_entry)
		{
		heater_s *heater= (heater_s *)list_entry->data;

		printf("<HTR> dev[%02d]: therm[%02d]\n", heater->indx, heater->thermometer->indx);

		list_entry= list_entry->next;
		}

	printf("\n");


	msleep(3000);








	xLastWakeTime= xTaskGetTickCount();

	while (1)
		{


		//msleep(3000);

		if (list_entry == NULL)
			{
			list_entry= thermometer_list.first_entry;

			if (list_entry == NULL)
				break; // brak zdefiniowanych termometr�w, zako�cz w�tek
			}

		therm_curr= (thermometer_s *)list_entry->data;

		dev_id_str[0]= 0x00;
		for (x=0;x<6;x++)
			{
			sprintf(dbgtmp, "%02X", therm_curr->dev_id[x]);
			strcat(dev_id_str, dbgtmp);
			}

		if (therm_curr->onewire_handler == NULL)
			{
			therm_curr->error_code= 0x01;
			therm_curr->temp_read_0x0550_cntr= 0;
			temp_read= 0x0000;
			goto prvOneWireTask_next;
			}

		//---





/*
		owire_resp= therm_ds18b20_id_read(therm_curr->onewire_handler, debug_txt);
		dev_id_str[0]= 0x00;
		for (x=0;x<6;x++)
			{
			sprintf(dbgtmp, "%02X", debug_txt[x]);
			strcat(dev_id_str, dbgtmp);
			}
		printf("1Wire: ID(%d): %s\n", owire_resp, dev_id_str);
*/







		owire_resp= therm_ds18b20_conversion_start(therm_curr->onewire_handler, therm_curr->dev_id);
//		owire_resp= therm_ds18b20_conversion_start(therm_curr->onewire_handler, NULL);

		if (owire_resp != 0x00)
			{
			therm_curr->error_code= (owire_resp == 0xF4) ? 0x07 : 0x02;
			therm_curr->temp_read_0x0550_cntr= 0;
			temp_read= 0x0000;

			if (therm_curr->onewire_handler->strong_pull_up_enable)
				onewire_strong_pullup_enable(therm_curr->onewire_handler, 0x00); // strong pull-up OFF

			goto prvOneWireTask_next;
			}


		msleep(800); // Tconv, max. 750 ms

		if (therm_curr->onewire_handler->strong_pull_up_enable)
			{
			onewire_strong_pullup_enable(therm_curr->onewire_handler, 0x00); // strong pull-up OFF
			msleep(1);
			}


		owire_resp= therm_ds18b20_temperature_read(therm_curr->onewire_handler, therm_curr->dev_id, &temp_read);
//		owire_resp= therm_ds18b20_temperature_read(therm_curr->onewire_handler, NULL, &temp_read);

		if (therm_curr->temp_debug_f)
			temp_read= therm_curr->temp_debug_value;

		if (owire_resp == 0x00)
			{

			if ((temp_read >= (k_short)0xFC90) && (temp_read <= (k_short)0x07D0)) // -55.0 - +125.0
				{

				therm_curr->error_code= 0x00;

				if (temp_read == 0x0550) // power on reset value
					{

					if (therm_curr->temp_read_0x0550_cntr < 10)
						therm_curr->temp_read_0x0550_cntr++;
					else
						{
						// power on reset value
						// termometr nie dzia�a prawid�owo

						therm_curr->error_code= 0x11;
						goto prvOneWireTask_next;
						}

					if (therm_curr->temp_read_0x0550_cntr == 1)
						goto prvOneWireTask_next;

					} // 0x0550
				else
					therm_curr->temp_read_0x0550_cntr= 0;


				// prawid�owy odczyt temperatury

				// oblicz sredni� temperatury

				if (!therm_curr->temp_vaild)
					{
					for (x=0;x<THERMOMETER_READ_CNTR;x++)
						therm_curr->temp_valuex[x]= temp_read;
					}

				temp_s= 0;
				for (x=0;x<THERMOMETER_READ_CNTR;x++)
					{
					temp_s+= therm_curr->temp_valuex[THERMOMETER_READ_CNTR - x - 1];

					if (x < (THERMOMETER_READ_CNTR-1))
						therm_curr->temp_valuex[THERMOMETER_READ_CNTR - x - 1]= therm_curr->temp_valuex[THERMOMETER_READ_CNTR - x - 2];
					}

				therm_curr->temp_valuex[0]= temp_read;
				temp_s+= temp_read;

				temp_avr_f= (float)temp_s;
				temp_avr_f/= (float)(THERMOMETER_READ_CNTR + 1);
				temp_avr_f= round(temp_avr_f);
				temp_avr= (k_short)temp_avr_f;
				temp_avr_f/= 16.0;

				temp_dbg= (float)temp_read;
				temp_dbg/= 16.0;

				// wpis

				xSemaphoreTake(thermometer_sem, 0);

				therm_curr->temp_value= temp_avr;
				therm_curr->temp_vaild= true;
				therm_curr->temp_read_error_cntr= 0;

				xSemaphoreGive(thermometer_sem);

				if (!therm_curr->temp_debug_f)
					printf("<TRM> dev[%02d]: chn[%d] id[%s] t=%f tavrg=%f\n", therm_curr->indx, therm_curr->onewire_handler->chn_no, dev_id_str, temp_dbg, temp_avr_f);
				else
					printf("<TRM> dev[%02d]: chn[%d] id[%s] t=%f tavrg=%f FORCED\n", therm_curr->indx, therm_curr->onewire_handler->chn_no, dev_id_str, temp_dbg, temp_avr_f);

				} // prawid�owy odczyt temperatury
			else
				{
				// b��dna wartosc odczytu tempertury

				switch (temp_read)
					{

					case 0x07FF:
						{
						// problem ze strong pullup
						therm_curr->error_code= 0x12;
						break;
						}

					default:
						{
						// wartosc spoza zakresu
						therm_curr->error_code= 0x13;
						break;
						}

					} // switch (temp_read)

				therm_curr->temp_read_0x0550_cntr= 0;
				goto prvOneWireTask_next;

				} // b��dna wartosc odczytu tempertury



			}
		else
			{

			switch (owire_resp)
				{

				case 0xF0: 	// odebrano ciag 00000...
					therm_curr->error_code= 0x03;
					break;

				case 0xF1: 	// odebrano ciag 11111...
					therm_curr->error_code= 0x04;
					break;

				case 0xF2:	// blad CRC
					therm_curr->error_code= 0x05;
					break;

				case 0xF3:	// blad formatu
					therm_curr->error_code= 0x06;
					break;

				case 0xF4:	// brak odpowiedzi na reset
					therm_curr->error_code= 0x07;
					break;

				default:
					therm_curr->error_code= 0x19;
					break;

				} // switch (owire_resp)


			therm_curr->temp_read_0x0550_cntr= 0;
			temp_read= 0x0000;
			goto prvOneWireTask_next;
			}



	prvOneWireTask_next:


		// obs�uga b��d�w

		if (therm_curr->error_code)
			{

			if (therm_curr->temp_read_error_cntr < (THERMOMETER_READ_ERROR_MAX - 1))
				therm_curr->temp_read_error_cntr++;
			else
			if (therm_curr->temp_vaild)
				{
				xSemaphoreTake(thermometer_sem, 0);
				therm_curr->temp_vaild= false;
				xSemaphoreGive(thermometer_sem);
				}

			printf("<ERR><TRM> dev[%02d]: chn[%d] id[%s] ERR[%02X] valid[%d] RAW[%04X]\n", therm_curr->indx, therm_curr->onewire_handler->chn_no, dev_id_str, therm_curr->error_code, therm_curr->temp_vaild, temp_read);

			} // if (therm_curr->error_code)


		//---


		list_entry= list_entry->next;

		if (list_entry == NULL)
			{
			// zako�czenie cylku odpyta� pomiaru temperatury
			k_uchar temp_queue_dta;

			temp_queue_dta= 0x00;
            xQueueSend(temp_pid_queue, &temp_queue_dta, (portTickType)0);

			//msleep(1000);
			vTaskDelayUntil(&xLastWakeTime, (THERMOMETER_READ_PERIOD * 1000 / portTICK_RATE_MS));
			}

		} // while (1)



	}




//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void cmdline_thermometer_temp_set(unsigned char *param)
	{
	unsigned char *tokptr;
	k_uchar indx= 0;

	thermometer_s *therm= NULL;
	k_uchar cmd= 0xFF;
	k_uchar nparam= 0;

	const k_uchar max_nparam= 3;
	k_ulong paramsval[max_nparam];

	while (param)
		{

		switch (indx)
			{

			case 0:	// indeks termometru
				{
				wdlist_entry_s *entry;
				thermometer_s *therm_tmp;
				k_uchar hindx;

				hindx= atoi(param);

				entry= thermometer_list.first_entry;
				while (entry)
					{
					therm_tmp= (thermometer_s *)entry->data;

					if (therm_tmp->indx == hindx)
						{
						therm= therm_tmp;
						break;
						}

					entry= entry->next;
					}

				break;
				}


			case 1:
				{

				if (!strcmp(param, "temp"))
					cmd= 0x01;


				break;
				}

			default:
				{

				if (nparam < max_nparam)
					{
					paramsval[nparam]= atoi(param);
					nparam++;
					}

				break;
				}

			} // switch (indx)

		indx++;
		param= strtok(NULL, cmd_inter_strtok_del);
		}


	if (therm)
		{

		switch (cmd)
			{

			case 0x01:	// temp
				{

				if (nparam == 1)
					{

					if (paramsval[0] == 0)
						{
						therm->temp_debug_f= false;

						printf("<TRM> dev[%02d] set to: real value\n", therm->indx);
						}
					else
						{
						therm->temp_debug_value= ((paramsval[0] / 1000) << 4) | ((k_short)((float)(paramsval[0] % 1000) / 1000.0 * 16) & 0xF);
						therm->temp_debug_f= true;

						printf("<TRM> dev[%02d] set to value: %2.3f\n", therm->indx, (float)(paramsval[0] / 1000.0));
						}

					}

				break;
				}


			} // switch (cmd)


		} // if (therm)


	}


//-----------------------------------------------------------------------------
