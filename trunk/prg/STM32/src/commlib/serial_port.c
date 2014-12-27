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
#include "timers.h"

#include <time.h>

#include "ksystem.h"
#include "serial_port.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#define USART_RX_TASK_PRIORITY			( tskIDLE_PRIORITY + 1 )
#define tx_ready_try_ms					((portTickType) 5 / portTICK_RATE_MS)
#define tx_timeout_ms 					((portTickType) 10000 / portTICK_RATE_MS)
#define rx_interchar_timeout_ms 		((portTickType) 10 / portTICK_RATE_MS)


#define USART1_TIMER_ID					0xC8000001
#define USART2_TIMER_ID					0xC8000002

//-----------------------------------------------------------------------------

typedef struct
	{
	xSemaphoreHandle usart_tx_int_sem;
	xSemaphoreHandle usart_rx_int_sem;
	xSemaphoreHandle usart_tx_usr_sem;
	xSemaphoreHandle usart_rx_usr_sem;

	unsigned char *usart_rx_buffer;
	unsigned char *usart_rx_data_ptr;
	unsigned char usart_rx_state;
	unsigned long usart_rx_timeout;
	unsigned char usart_rx_buffer_size;

	xTimerHandle rx_timer;

	} usart_data_s;

//-----------------------------------------------------------------------------

usart_data_s usart1_def;
usart_data_s usart2_def;

unsigned char usart1_rx_buffer[USART1_RX_BUFFER_SIZE];
unsigned char usart2_rx_buffer[USART2_RX_BUFFER_SIZE];

void vUSARTx_Rx_TimerCallback(xTimerHandle pxTimer);

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

			vSemaphoreCreateBinary(usart1_def.usart_tx_usr_sem);

			vSemaphoreCreateBinary(usart1_def.usart_rx_usr_sem);
			xSemaphoreTake(usart1_def.usart_rx_usr_sem, 0);

			usart1_def.usart_rx_buffer= usart1_rx_buffer;
			usart1_def.usart_rx_data_ptr= usart1_rx_buffer;
			usart1_def.usart_rx_buffer_size= USART1_RX_BUFFER_SIZE;
			usart1_def.usart_rx_state= 0x00;
			usart1_def.usart_rx_timeout= (portTickType)portMAX_DELAY;


#if !defined(USART1_RX485)
			RCC_APB2PeriphClockCmd(USART1_GPIO_CLK | USART1_CLK, ENABLE);
#else
			RCC_APB2PeriphClockCmd(USART1_GPIO_CLK | USART1_CLK | USART1_TxEnaPin_GPIO_CLK, ENABLE);
#endif
			GPIO_InitStructure.GPIO_Pin = USART1_TxPin;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(USART1_GPIO, &GPIO_InitStructure);

			GPIO_InitStructure.GPIO_Pin = USART1_RxPin;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(USART1_GPIO, &GPIO_InitStructure);

#if defined(USART1_RX485)
			GPIO_InitStructure.GPIO_Pin = USART1_TxEnaPin;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(USART1_TxEnaPin_GPIO, &GPIO_InitStructure);
			GPIO_ResetBits(USART1_TxEnaPin_GPIO, USART1_TxEnaPin); // kierunek odbiór
#endif

			USART_InitStructure->USART_HardwareFlowControl = USART_HardwareFlowControl_None;
			USART_InitStructure->USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

			USART_Init(USART1, USART_InitStructure);

			usart1_def.rx_timer= xTimerCreate("TIM", rx_interchar_timeout_ms, 0, (void *)USART1_TIMER_ID, vUSARTx_Rx_TimerCallback);

			USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); // przerwanie od Rx

			RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

			USART_Cmd(USART1, ENABLE);

			result= 0;
			break;
			} // USART1


		case 2: // USART2
			{

			vSemaphoreCreateBinary(usart2_def.usart_tx_int_sem);
			xSemaphoreTake(usart2_def.usart_tx_int_sem, 0);

			vSemaphoreCreateBinary(usart2_def.usart_rx_int_sem);
			xSemaphoreTake(usart2_def.usart_rx_int_sem, 0);

			vSemaphoreCreateBinary(usart2_def.usart_tx_usr_sem);

			vSemaphoreCreateBinary(usart2_def.usart_rx_usr_sem);
			xSemaphoreTake(usart2_def.usart_rx_usr_sem, 0);

			usart2_def.usart_rx_buffer= usart2_rx_buffer;
			usart2_def.usart_rx_data_ptr= usart2_rx_buffer;
			usart2_def.usart_rx_buffer_size= USART2_RX_BUFFER_SIZE;
			usart2_def.usart_rx_state= 0x00;
			usart2_def.usart_rx_timeout= (portTickType)portMAX_DELAY;

#if !defined(USART2_RX485)
			RCC_APB2PeriphClockCmd(USART2_GPIO_CLK, ENABLE);
			RCC_APB1PeriphClockCmd(USART2_CLK, ENABLE);
#else
			RCC_APB2PeriphClockCmd(USART2_GPIO_CLK | USART2_TxEnaPin_GPIO_CLK, ENABLE);
			RCC_APB1PeriphClockCmd(USART2_CLK, ENABLE);
#endif
			GPIO_InitStructure.GPIO_Pin = USART2_TxPin;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(USART2_GPIO, &GPIO_InitStructure);

			GPIO_InitStructure.GPIO_Pin = USART2_RxPin;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(USART2_GPIO, &GPIO_InitStructure);

#if defined(USART2_RX485)
			GPIO_InitStructure.GPIO_Pin = USART2_TxEnaPin;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(USART2_TxEnaPin_GPIO, &GPIO_InitStructure);
			GPIO_ResetBits(USART2_TxEnaPin_GPIO, USART2_TxEnaPin); // kierunek odbiór
#endif

			USART_InitStructure->USART_HardwareFlowControl = USART_HardwareFlowControl_None;
			USART_InitStructure->USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

			USART_Init(USART2, USART_InitStructure);

			usart2_def.rx_timer= xTimerCreate("TIM", rx_interchar_timeout_ms, 0, (void *)USART2_TIMER_ID, vUSARTx_Rx_TimerCallback);

			USART_ITConfig(USART2, USART_IT_RXNE, ENABLE); // przerwanie od Rx

			RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

			USART_Cmd(USART2, ENABLE);

			result= 0;
			break;
			} // USART2


		default:
			break;

		} // switch (port_no)

	return result;
	}

//-----------------------------------------------------------------------------

int serial_port_write(unsigned char port_no, char *ptr, int len)
	{
	int result= -1;
	DMA_InitTypeDef DMA_InitStructure;


	if (!ptr || (len < 1))
		return -1;

	switch (port_no)
		{

		case 1: // USART1
			{

			xSemaphoreTake(usart1_def.usart_tx_usr_sem, portMAX_DELAY);

			while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
				vTaskDelay(tx_ready_try_ms);

#if defined(USART1_RX485)
			USART1->CR1&= ~USART_Mode_Rx; // wy³¹czam odbiornik, brak powoduje odbieranie 0x00
			GPIO_SetBits(USART1_TxEnaPin_GPIO, USART1_TxEnaPin); // kierunek nadawanie
#endif

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

#if defined(USART1_RX485)
			msleep(5);
			GPIO_ResetBits(USART1_TxEnaPin_GPIO, USART1_TxEnaPin); // kierunek odbiór
			USART1->CR1|= USART_Mode_Rx; // w³¹czam odbiornik
#endif

			xSemaphoreGive(usart1_def.usart_tx_usr_sem);

			break;
			} // USART1


		case 2: // USART2
			{

			xSemaphoreTake(usart2_def.usart_tx_usr_sem, portMAX_DELAY);

			while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
				vTaskDelay(tx_ready_try_ms);

#if defined(USART2_RX485)
			USART2->CR1&= ~USART_Mode_Rx; // wy³¹czam odbiornik, brak powoduje odbieranie 0x00
			GPIO_SetBits(USART2_TxEnaPin_GPIO, USART2_TxEnaPin); // kierunek nadawanie
#endif

			DMA_DeInit(USART2_Tx_DMA_Channel);
			DMA_InitStructure.DMA_PeripheralBaseAddr = USART2_DR_Base;
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
			DMA_Init(USART2_Tx_DMA_Channel, &DMA_InitStructure);

			DMA_ClearITPendingBit(DMA1_IT_GL7);
			DMA_ITConfig(USART2_Tx_DMA_Channel, DMA_IT_TC, ENABLE);

			DMA_Cmd(USART2_Tx_DMA_Channel, ENABLE);
			USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);


			// czekam na zakoñczenie DMA

			if (xSemaphoreTake(usart2_def.usart_tx_int_sem, tx_timeout_ms) == pdTRUE)
				result= len;
			else
				result= -1; // timeout

#if defined(USART2_RX485)
			msleep(5);
			GPIO_ResetBits(USART2_TxEnaPin_GPIO, USART2_TxEnaPin); // kierunek odbiór
			USART2->CR1|= USART_Mode_Rx; // w³¹czam odbiornik
#endif

			xSemaphoreGive(usart2_def.usart_tx_usr_sem);

			break;
			} // USART2


		default:
			break;

		} // switch (port_no)

	return result;
	}

//-----------------------------------------------------------------------------

int serial_port_read(unsigned char port_no, char *ptr, int len)
	{
	unsigned int recvn;
	usart_data_s *usart_def;

	if (!ptr || (len < 1))
		return -1;

	switch (port_no)
		{
		case 1:
			usart_def= &usart1_def;
			break;

		case 2:
			usart_def= &usart2_def;
			break;

		default:
			usart_def= NULL;
			break;

		} // switch (port_no)

	if (!usart_def)
		return -1;

	xSemaphoreTake(usart_def->usart_rx_usr_sem, usart_def->usart_rx_timeout);

	recvn= usart_def->usart_rx_data_ptr - usart_def->usart_rx_buffer;

	if ((usart_def->usart_rx_state == 0x01) && (recvn != 0))
		{
		if (len < recvn)
			recvn= len;

		memcpy(ptr, usart_def->usart_rx_buffer, recvn);
		usart_def->usart_rx_data_ptr= usart_def->usart_rx_buffer;
		usart_def->usart_rx_state= 0x00;

		return (int)recvn;
		}
	else
		{
		usart_def->usart_rx_data_ptr= usart_def->usart_rx_buffer;
		usart_def->usart_rx_state= 0x00;

		return 0; // timeout
		}

	}

//-----------------------------------------------------------------------------

int serial_port_rx_timeout_set(unsigned char port_no, int timeout)
	{
	usart_data_s *usart_def;

	if (timeout < 1)
		return -1;

	switch (port_no)
		{
		case 1:
			usart_def= &usart1_def;
			break;

		case 2:
			usart_def= &usart2_def;
			break;

		default:
			usart_def= NULL;
			break;

		} // switch (port_no)

	if (!usart_def)
		return -1;

	if ((portTickType)timeout == portMAX_DELAY)
		usart_def->usart_rx_timeout= (portTickType)portMAX_DELAY;
	else
		usart_def->usart_rx_timeout= (portTickType)timeout / portTICK_RATE_MS;

	return 0;
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

void USART2_Tx_DMA_IRQ_Handler(void)
	{
	static signed portBASE_TYPE xHigherPriorityTaskWoken= pdFALSE;

	DMA_ClearITPendingBit(DMA1_IT_GL7);
	xSemaphoreGiveFromISR(usart2_def.usart_tx_int_sem, &xHigherPriorityTaskWoken);
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

			if (usart1_def.usart_rx_data_ptr == (usart1_rx_buffer + USART1_RX_BUFFER_SIZE))
				{
				usart1_def.usart_rx_state= 0x01;
				xTimerStopFromISR(usart1_def.rx_timer, &xHigherPriorityTaskWoken);
				xSemaphoreGiveFromISR(usart1_def.usart_rx_usr_sem, &xHigherPriorityTaskWoken);
				}
			else
				xTimerResetFromISR(usart1_def.rx_timer, &xHigherPriorityTaskWoken);

			}
		}
	}

//-----------------------------------------------------------------------------

void USART2_Rx_IRQ_Handler(void)
	{
	static unsigned char recv_char;
	static signed portBASE_TYPE xHigherPriorityTaskWoken= pdFALSE;

	if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
		{
		// odebrano znak
		recv_char= USART_ReceiveData(USART2);

		if ((usart2_def.usart_rx_state == 0x00) && (usart2_def.usart_rx_data_ptr < (usart2_rx_buffer + USART2_RX_BUFFER_SIZE)))
			{
			*usart2_def.usart_rx_data_ptr++= recv_char;

			if (usart2_def.usart_rx_data_ptr == (usart2_rx_buffer + USART2_RX_BUFFER_SIZE))
				{
				usart2_def.usart_rx_state= 0x01;
				xTimerStopFromISR(usart2_def.rx_timer, &xHigherPriorityTaskWoken);
				xSemaphoreGiveFromISR(usart2_def.usart_rx_usr_sem, &xHigherPriorityTaskWoken);
				}
			else
				xTimerResetFromISR(usart2_def.rx_timer, &xHigherPriorityTaskWoken);

			}
		}
	}

//-----------------------------------------------------------------------------

void vUSARTx_Rx_TimerCallback(xTimerHandle pxTimer)
	{
	k_ulong timer_id= (k_ulong)pvTimerGetTimerID(pxTimer);

	switch (timer_id)
		{

		case USART1_TIMER_ID:
			{
			usart1_def.usart_rx_state= 0x01;
			xSemaphoreGive(usart1_def.usart_rx_usr_sem);
			break;
			}

		case USART2_TIMER_ID:
			{
			usart2_def.usart_rx_state= 0x01;
			xSemaphoreGive(usart2_def.usart_rx_usr_sem);
			break;
			}

		} // switch (timer_id)

	}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
