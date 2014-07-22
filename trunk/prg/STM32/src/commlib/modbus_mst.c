/*-----------------------------------------------------------------------------/
 * modbus_mst.c
 *
 *  Created on: 26-06-2014
 *      Author: Tomasz Przybysz
/-----------------------------------------------------------------------------*/
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"


#include "serial_port.h"

#include "modbus_mst.h"

//-----------------------------------------------------------------------------

#define polling_timeout_ms 		((portTickType) 10000 / portTICK_RATE_MS)

//-----------------------------------------------------------------------------

static void modbus_mst_thread(void *pvParameters);



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

int modbus_mst_run(modbus_channel_s *modbus_channel)
	{

	USART_InitTypeDef USART_InitStructure;


	if (!modbus_channel)
		return -1;


	USART_InitStructure.USART_BaudRate = 57600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;


	serial_port_init(modbus_channel->chno, &USART_InitStructure);
	serial_port_rx_timeout_set(modbus_channel->chno, 10000);

	vSemaphoreCreateBinary(modbus_channel->ctrl_sem);
	xSemaphoreTake(modbus_channel->ctrl_sem, 0);
	


	xTaskCreate(modbus_mst_thread, (signed char *)"mdb", configMINIMAL_STACK_SIZE, (void *)modbus_channel, USART_RX_TASK_PRIORITY, NULL);

	return 0;
	}

//-----------------------------------------------------------------------------

static void modbus_mst_thread(void *pvParameters)
	{
	modbus_channel_s *channel= (modbus_channel_s *)modbus_channel;

	bool event_recv; // zmienic na dobry typ !
	unsigned char poll_indx;
	modbus_ctrl_param_s ctrl_param_pending;


	channel->connected= false;
	channel->state= MODBUS_STATE_IDLE;



	while (1)
		{
		event_recv= false;


		if (channel->state == MODBUS_STATE_IDLE)
			{
			event_recv= xSemaphoreTake(channel->ctrl_sem, polling_timeout_ms);
			if ((event_recv == pdTRUE) && (channel->state != MODBUS_STATE_CTRL_REQ))
				continue;
			}


		if (!event_recv)
			{
			// polling




			}



		if ((channel->state != MODBUS_STATE_CTRL_REQ) && (channel->state != MODBUS_STATE_CTRL_PENDING))
			{

			}



		if (channel->state == MODBUS_STATE_CTRL_REQ)
			{
			// sterowanie

			xSemaphoreTake(channel->ctrl_sem, 0);
			memcpy(&ctrl_param_pending, channel->ctrl_param_waiting, sizeof(modbus_ctrl_param_s));

			


			channel->state= MODBUS_STATE_CTRL_PENDING;

			} // MODBUS_STATE_CTRL_REQ




		if (channel->state == MODBUS_STATE_IDLE)
			{

			if (xSemaphoreTake(channel->ctrl_sem, 10000) == pdTRUE)
				{
				// sterowanie
				memcpy(&ctrl_param_pending, channel->ctrl_param_waiting, sizeof(modbus_ctrl_param_s));
				}
			else
				{
				// timeout, zacznij polling
				poll_indx= 0;
				channel->state= MODBUS_STATE_POLLING;
				}

			} // MODBUS_STATE_IDLE

		





		} // while (1)


	}

//-----------------------------------------------------------------------------

void modbus_ctrl(modbus_channel_s *channel, modbus_ctrl_param_s *ctrl_param)
	{
	unsigned char timeout= 5;

	if (!channel || !ctrl_param)
		return;


	// sprawdz czy polaczony

	if (!channel->connected)
		return;

	// sprawdzam czy zakonczone poprzednie sterowanie

	while (((channel->state == MODBUS_STATE_CTRL_REQ) || (channel->state == MODBUS_STATE_CTRL_PENDING)) && timeout)
		{
		msleep(1000);
		timeout-= 1;
		}

	if ((channel->state == MODBUS_STATE_CTRL_REQ) || (channel->state == MODBUS_STATE_CTRL_PENDING))
		return;


	memcpy(&channel->ctrl_param_waiting, ctrl_param, sizeof(modbus_ctrl_param_s));
	channel->state= MODBUS_STATE_CTRL_REQ;
	xSemaphoreGive(channel->ctrl_sem);

	}

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------


