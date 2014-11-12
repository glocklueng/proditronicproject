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
#include "tpgui.h"
#include "ksystem.h"

#include "modbus_mst.h"

//-----------------------------------------------------------------------------

#define MODBUS_BUFFER_SIZE	(USART1_RX_BUFFER_SIZE)

//#define polling_period_ms 		((portTickType) 10000 / portTICK_RATE_MS)
#define polling_delay_ms 		((portTickType) 1000 / portTICK_RATE_MS)


//-----------------------------------------------------------------------------

/*
modbus_data_s modbus_data_tab[]=
	{



	};

*/




//-----------------------------------------------------------------------------

static void modbus_mst_thread(void *pvParameters);



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

int modbus_mst_run(modbus_mst_channel_s *modbus_channel)
	{/*

	USART_InitTypeDef USART_InitStructure;


	if (!modbus_channel)
		return -1;


	USART_InitStructure.USART_BaudRate = 57600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;


	serial_port_init(modbus_channel->chno, &USART_InitStructure);
	serial_port_rx_timeout_set(modbus_channel->chno, 10000);
*/


	modbus_channel->ctrl_state= MODBUS_STATE_CTRL_IDLE;
	vSemaphoreCreateBinary(modbus_channel->ctrl_sem);
	xSemaphoreTake(modbus_channel->ctrl_sem, 0);



	xTaskCreate(modbus_mst_thread, "mdb", 512, (void *)modbus_channel, tskIDLE_PRIORITY, NULL);

	return 0;
	}

//-----------------------------------------------------------------------------

static void modbus_mst_thread(void *pvParameters)
	{
	modbus_mst_channel_s *channel= (modbus_mst_channel_s *)pvParameters;
	uint8_t trbuffer[MODBUS_BUFFER_SIZE];
	uint8_t recvd;

	modbus_ctrl_param_s ctrl_param_pending;

	uint16_t *modbus_addr_poll;
	modbus_data_s *modbus_data_poll;

	uint8_t slv_addr, func_code, rxerr, exp_recv_len, exp_data_size;

	int dpi;
	uint16_t recv_reg_val= 0;

	uint8_t poll_timer= 15;

	uint8_t resp_code= 0x00;


	//channel->connected= false;
	//channel->state= MODBUS_STATE_IDLE;


	modbus_addr_poll= channel->modbus_addr_poll;


	for (dpi=0;dpi<8;dpi++)
		trbuffer[dpi]= 0x40 + dpi;




	while (1)
		{

		serial_port_rx_timeout_set(channel->chno, 10);
		serial_port_read(channel->chno, trbuffer, MODBUS_BUFFER_SIZE);
		serial_port_rx_timeout_set(channel->chno, 1000);



		if (xSemaphoreTake(channel->ctrl_sem, polling_delay_ms) == pdTRUE)
			{
			// sterowanie

			if (channel->ctrl_state == MODBUS_STATE_CTRL_REQ)
				{
				channel->ctrl_state= MODBUS_STATE_CTRL_PENDING;
				memcpy(&ctrl_param_pending, &channel->ctrl_param_waiting, sizeof(modbus_ctrl_param_s));
				}

			xSemaphoreGive(channel->ctrl_sem);
			}
		else
			{

			if (poll_timer)
				poll_timer-= 1;

			printf("poll_timer: %d\n", poll_timer);
			}



		if (channel->ctrl_state == MODBUS_STATE_CTRL_PENDING)
			{
			// sterowanie

			channel->ctrl_state= MODBUS_STATE_CTRL_IDLE;




			resp_code= 0x10;
			goto modbus_mst_resp;

			} // sterowanie


		if (poll_timer == 0)
			{
			// polling

			if (*modbus_addr_poll == NULL)
				modbus_addr_poll= channel->modbus_addr_poll;

			printf("modbus_addr_poll REQ %08X %04X\n", modbus_addr_poll, *modbus_addr_poll);

			trbuffer[0]= 0x01;
			trbuffer[1]= 0x03;
			*(uint16_t *)&trbuffer[2]= htons(*modbus_addr_poll);
			*(uint16_t *)&trbuffer[4]= htons(0x0001);
			*(uint16_t *)&trbuffer[6]= htons(0x1234);

			serial_port_write(channel->chno, trbuffer, 8);

			exp_recv_len= 5;
			exp_data_size= 2;

			resp_code= 0x01;
			goto modbus_mst_resp;

			} // polling


		continue;



modbus_mst_resp:

		slv_addr= trbuffer[0];
		func_code= trbuffer[1];

		recvd= serial_port_read(channel->chno, trbuffer, MODBUS_BUFFER_SIZE);
		printf("recvd: %d\n", recvd);

		if (recvd == 0)
			{
			// timeout

			}
		else
			{
			// frame recvd

			rxerr= 0x00;

			if (trbuffer[0] != slv_addr)
				{
				printf("[ERR] modbus recv: bad address: %02X\n", trbuffer[0]);
				rxerr= 0x01;
				goto modbus_mst_resp_err;
				}

			if (trbuffer[1] != func_code)
				{
				printf("[ERR] modbus recv: bad function: %02X\n", trbuffer[1]);
				rxerr= 0x01;
				goto modbus_mst_resp_err;
				}

			if (recvd != (exp_recv_len + 2))
				{
				printf("[ERR] modbus recv: bad frame size: %d\n", recvd);
				rxerr= 0x01;
				goto modbus_mst_resp_err;
				}

			if (0) // crc
				{
				printf("[ERR] modbus recv: bad CRC\n");
				rxerr= 0x01;
				goto modbus_mst_resp_err;
				}


			// analiza odpowiedzi

			switch (resp_code)
				{

				case 0x01: // polling
					{

					if (trbuffer[2] != exp_data_size)
						{
						printf("[ERR] modbus recv: bad data size: %d\n", trbuffer[2]);
						rxerr= 0x01;
						break;
						}

					printf("modbus_addr_poll RESP %08X %04X\n", modbus_addr_poll, *modbus_addr_poll);

					recv_reg_val= ntohs(*(uint16_t *)&trbuffer[3]);

					dpi= 0;
					while (modbus_data_poll= channel->modbus_data_poll[dpi])
						{

						if (modbus_data_poll->modbus_addr == *modbus_addr_poll)
							{
							printf("  modbus_data_poll: %04X %02X\n", modbus_data_poll->modbus_addr, modbus_data_poll->data_type);

							if (modbus_data_poll->dataptr)
								{

								switch (((tpgui_screen_item_s *)modbus_data_poll->dataptr)->type)
									{

									case TPGUI_SI_VARIABLE:
										{
										tpgui_screen_item_variable_s *item= (tpgui_screen_item_variable_s *)modbus_data_poll->dataptr;

										switch (item->data_type)
											{

											case TPGUI_VAR_DATATYPE_FLOAT:
												{
												float *data= (float *)item->data_ptr;

												switch (modbus_data_poll->data_type)
													{

													case MODBUS_DATA_TYPE_INT16:
														{
														*data= (float)((int16_t)recv_reg_val) / 16.0;
														break;
														}



													} // switch (modbus_data_poll->data_type)

												tpgui_screen_item_change_notify(item);

												break;
												}



											} // switch (item->data_type)

										break;
										} // TPGUI_SI_VARIABLE

									} // switch(type)


								} // if (modbus_data_poll->dataptr)


							} // modbus_addr

						dpi++;
						} // while (modbus_data_poll)

					modbus_addr_poll++;

					if (*modbus_addr_poll == NULL)
						{
						modbus_addr_poll= channel->modbus_addr_poll;
						poll_timer= 15;
						}

					break;
					} // polling


				case 0x10: // sterowanie
					{



					break;
					} // sterowanie


				} // switch (resp_code)




modbus_mst_resp_err:

			if (rxerr)
				msleep(1000);

			} // // frame recvd



		} // while (1)


	}

//-----------------------------------------------------------------------------

void modbus_ctrl(modbus_mst_channel_s *channel, modbus_ctrl_param_s *ctrl_param)
	{
	unsigned char timeout= 5;

	if (!channel || !ctrl_param)
		return;

	// sprawdz czy polaczony
	if (!channel->connected)
		return;

	// sprawdzam czy zakonczone poprzednie sterowanie

	while ((channel->ctrl_state != MODBUS_STATE_CTRL_IDLE) && timeout)
		{
		msleep(1000);
		timeout-= 1;
		}

	if (channel->ctrl_state != MODBUS_STATE_CTRL_IDLE)
		return;

	memcpy(&channel->ctrl_param_waiting, ctrl_param, sizeof(modbus_ctrl_param_s));
	channel->ctrl_state= MODBUS_STATE_CTRL_REQ;
	xSemaphoreGive(channel->ctrl_sem);

	}

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------


