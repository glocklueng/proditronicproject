/*-----------------------------------------------------------------------------/
 * serial_port.cpp
 *
 *  Created on: 01-02-2013
 *      Author: Tomasz Przybysz
/-----------------------------------------------------------------------------*/
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"


#include "serial_port.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#define USART_RX_TASK_PRIORITY			( tskIDLE_PRIORITY + 1 )
#define tx_ready_try_ms					((portTickType) 5 / portTICK_RATE_MS)
#define tx_timeout_ms 					((portTickType) 10000 / portTICK_RATE_MS)
#define rx_interchar_timeout_ms 		((portTickType) 50 / portTICK_RATE_MS)



//-----------------------------------------------------------------------------

typedef struct
	{
	xSemaphoreHandle usart_tx_int_sem;
	xSemaphoreHandle usart_rx_int_sem;
	xSemaphoreHandle usart_rx_usr_sem;

	unsigned char *usart_rx_buffer;
	unsigned char *usart_rx_data_ptr;
	unsigned char usart_rx_state;
	unsigned long usart_rx_timeout;

	} usart_data_s;

//-----------------------------------------------------------------------------

usart_data_s usart1_def;

unsigned char usart1_rx_buffer[USART1_RX_BUFFER_SIZE];


static void USARTx_Rx_Task(void *pvParameters);

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

int serial_port_init(unsigned char port_no, USART_InitTypeDef *USART_InitStructure)
	{
	int result= -1;
	GPIO_InitTypeDef GPIO_InitStructure;

	if (!USART_InitStructure)
		return -1;

	switch (port_no)
		{

		case 1: // USART1
			{

			vSemaphoreCreateBinary(usart1_def.usart_tx_int_sem);
			xSemaphoreTake(usart1_def.usart_tx_int_sem, 0);

			vSemaphoreCreateBinary(usart1_def.usart_rx_int_sem);
			xSemaphoreTake(usart1_def.usart_rx_int_sem, 0);

			vSemaphoreCreateBinary(usart1_def.usart_rx_usr_sem);
			xSemaphoreTake(usart1_def.usart_rx_usr_sem, 0);

			usart1_def.usart_rx_buffer= usart1_rx_buffer;
			usart1_def.usart_rx_data_ptr= usart1_rx_buffer;
			usart1_def.usart_rx_state= 0x00;
			usart1_def.usart_rx_timeout= (portTickType)portMAX_DELAY;


			RCC_APB2PeriphClockCmd(USART1_GPIO_CLK | USART1_CLK, ENABLE);

			GPIO_InitStructure.GPIO_Pin = USART1_TxPin;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(USART1_GPIO, &GPIO_InitStructure);

			GPIO_InitStructure.GPIO_Pin = USART1_RxPin;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(USART1_GPIO, &GPIO_InitStructure);


			USART_InitStructure->USART_HardwareFlowControl = USART_HardwareFlowControl_None;
			USART_InitStructure->USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

			USART_Init(USART1, USART_InitStructure);

			// w¹tek Rx
			xTaskCreate(USARTx_Rx_Task, (signed char *)"Rx", configMINIMAL_STACK_SIZE, (void *)&usart1_def, USART_RX_TASK_PRIORITY, NULL);

			USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); // przerwanie od Rx

			RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

			USART_Cmd(USART1, ENABLE);

			result= 0;
			break;
			} // USART1

		default:
			break;

		} // switch (port_no)

	return result;
	}

//-----------------------------------------------------------------------------

int serial_port_write(unsigned char port_no, char *ptr, int len)
	{
	static int result= -1;
	static DMA_InitTypeDef DMA_InitStructure;


	if (!ptr || (len < 1))
		return -1;

	switch (port_no)
		{

		case 1: // USART1
			{

			while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
				vTaskDelay(tx_ready_try_ms);

			DMA_DeInit(USART1_Tx_DMA_Channel);
			DMA_InitStructure.DMA_PeripheralBaseAddr = USART1_DR_Base;
			DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ptr;
			DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
			DMA_InitStructure.DMA_BufferSize = len;
			DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
			DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
			DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
			DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
			DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
			DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
			DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
			DMA_Init(USART1_Tx_DMA_Channel, &DMA_InitStructure);

			DMA_ClearITPendingBit(DMA1_IT_GL4);
			DMA_ITConfig(USART1_Tx_DMA_Channel, DMA_IT_TC, ENABLE);

			DMA_Cmd(USART1_Tx_DMA_Channel, ENABLE);
			USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);


			// czekam na zakoñczenie DMA

			if (xSemaphoreTake(usart1_def.usart_tx_int_sem, tx_timeout_ms) == pdTRUE)
				result= len;
			else
				result= -1; // timeout

			result= len;
			break;
			} // USART1

		default:
			break;

		} // switch (port_no)

	return result;
	}

//-----------------------------------------------------------------------------

int serial_port_read(unsigned char port_no, char *ptr, int len)
	{
	static unsigned int recvn;
	static usart_data_s *usart_def;

	if (!ptr || (len < 1))
		return -1;

	switch (port_no)
		{
		case 1:
			usart_def= &usart1_def;
			break;

		default:
			usart_def= NULL;
			break;

		} // switch (port_no)

	if (!usart_def)
		return -1;

	xSemaphoreTake(usart_def->usart_rx_usr_sem, usart_def->usart_rx_timeout);

	recvn= usart_def->usart_rx_data_ptr - usart_def->usart_rx_buffer;

	if (recvn == 0)
		return 0; // timeout

	if (len < recvn)
		recvn= len;

	memcpy(ptr, usart_def->usart_rx_buffer, recvn);
	usart_def->usart_rx_data_ptr= usart_def->usart_rx_buffer;
	usart_def->usart_rx_state= 0x00;

	return (int)recvn;
	}

//-----------------------------------------------------------------------------

int serial_port_rx_timeout_set(unsigned char port_no, int timeout)
	{
	int result;

	if (timeout < 1)
		return -1;

	switch (port_no)
		{

		case 1: // USART1
			{

			if ((portTickType)timeout == portMAX_DELAY)
				usart1_def.usart_rx_timeout= (portTickType)portMAX_DELAY;
			else
				usart1_def.usart_rx_timeout= (portTickType)timeout / portTICK_RATE_MS;

			result= 0;
			break;
			} // USART1

		default:
			{
			result= -1;
			break;
			}

		} // switch (port_no)

	return result;
	}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void USART1_Tx_DMA_IRQ_Handler(void)
	{
	static signed portBASE_TYPE xHigherPriorityTaskWoken= pdFALSE;

	DMA_ClearITPendingBit(DMA1_IT_GL4);
	xSemaphoreGiveFromISR(usart1_def.usart_tx_int_sem, &xHigherPriorityTaskWoken);
	}

//-----------------------------------------------------------------------------

void USART1_Rx_IRQ_Handler(void)
	{
	static unsigned char recv_char;
	static signed portBASE_TYPE xHigherPriorityTaskWoken= pdFALSE;

	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
		{
		// odebrano znak
		recv_char= USART_ReceiveData(USART1);

		if ((usart1_def.usart_rx_state == 0x00) && (usart1_def.usart_rx_data_ptr < (usart1_rx_buffer + USART1_RX_BUFFER_SIZE)))
			{
			*usart1_def.usart_rx_data_ptr++= recv_char;
			xSemaphoreGiveFromISR(usart1_def.usart_rx_int_sem, &xHigherPriorityTaskWoken);
			}
		}
	}

//-----------------------------------------------------------------------------

static void USARTx_Rx_Task(void *pvParameters)
	{
	usart_data_s *usart_def= (usart_data_s *)pvParameters;

	while (1)
		{

		if (xSemaphoreTake(usart_def->usart_rx_int_sem, rx_interchar_timeout_ms) == pdTRUE)
			{
			// odebrano znak
			if ((usart1_def.usart_rx_state == 0x00) && (usart_def->usart_rx_data_ptr == (usart_def->usart_rx_buffer + USART1_RX_BUFFER_SIZE)))
				{
				// przekazanie ramki
				usart_def->usart_rx_state= 0x01;
				xSemaphoreGive(usart_def->usart_rx_usr_sem);
				}
			}
		else
		if ((usart1_def.usart_rx_state == 0x00) && (usart_def->usart_rx_data_ptr != usart_def->usart_rx_buffer))
			{
			// timeout przerwy miêdzyznakowej, tylko gdy w buforze >= 1 odebrany znak
			// przekazanie ramki
			usart_def->usart_rx_state= 0x01;
			xSemaphoreGive(usart_def->usart_rx_usr_sem);
			}

		} // while (1)

	}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------





