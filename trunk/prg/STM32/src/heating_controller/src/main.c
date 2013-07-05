//============================================================================
// Name        : STM32_Dev_Template.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stm32f10x.h"


#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "FreeRTOSConfig.h"

#include "serial_port.h"
#include "ksystem.h"
#include "wdlist.h"


#include "global.h"

#include "1wire.h"
#include "1wire_thr.h"

#include "tempctrlpid_thr.h"



#define mainECHO_TASK_PRIORITY					( tskIDLE_PRIORITY + 1 )
#define mainONEWIRE_TASK_PRIORITY				( tskIDLE_PRIORITY + 1 )
#define mainTEMPCTRLPID_TASK_PRIORITY			( tskIDLE_PRIORITY + 1 )

//using namespace std;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

static void prvSetupHardware(void);

void GPIO_Configuration(void);
void NVIC_Configuration(void);
void USART_Configuration(void);


static void prvUSARTEchoTask(void *pvParameters);


#define mainBLOCK_Q_PRIORITY				( tskIDLE_PRIORITY + 2 )

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

// globalne

onewire_handler_s onewire_chnlst[ONEWIRE_NBUS];

wdlist_s thermometer_list;
xSemaphoreHandle thermometer_sem;

wdlist_s heater_list;
xQueueHandle temp_pid_queue;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void  Delay (uint32_t nCount)
{
  for(; nCount != 0; nCount--);
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

ktimer_spec_s timer_1;
static int cntr= 0;


void led_switch()
	{
	static char ledstate= 0;

	if (ledstate)
		{
		ledstate= 0;
		GPIO_ResetBits(GPIOB , GPIO_Pin_0 | GPIO_Pin_1);
		}
	else
		{
		ledstate= 1;
		GPIO_SetBits(GPIOB , GPIO_Pin_0 | GPIO_Pin_1);
		}

	}


//------------------------------------------------------------------------------

int main()
	{

	USART_InitTypeDef USART_InitStructure;

	thermometer_s *therm_new;
	heater_s *heater_new;



	// zatanowic sie czy dla 1-wire wyjscie PP czy OC !!!
	// zatanowic sie czy dla 1-wire wyjscie PP czy OC !!!
	// zatanowic sie czy dla 1-wire wyjscie PP czy OC !!!
	// zatanowic sie czy dla 1-wire wyjscie PP czy OC !!!




	GPIO_Configuration();
	NVIC_Configuration();

	prvSetupHardware();


	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	USART_InitStructure.USART_BaudRate = 57600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;

	serial_port_init(1, &USART_InitStructure); // console init
	serial_port_rx_timeout_set(1, 1000);

	ktimerlst_init();



	vSemaphoreCreateBinary(thermometer_sem);
	wdlist_init(&thermometer_list);

	wdlist_init(&heater_list);

	temp_pid_queue= xQueueCreate(1, 1);


// odczyt konfiguracji


	// termometry

	therm_new= (thermometer_s *)malloc(sizeof(thermometer_s));
	therm_new->onewire_handler= &onewire_chnlst[0];
	therm_new->temp_vaild= false;
	therm_new->temp_read_error_cntr= 0;
	memset(therm_new->dev_id, 0x55, 8);
	wdlist_append(&thermometer_list, (void *)therm_new);

	
	// grzejniki

	heater_new= (heater_s *)malloc(sizeof(heater_s));
	heater_new->thermometer= therm_new;
	heater_new->temp_zadana= 21000; // 21,0
	heater_new->temp_offset= 0;
	wdlist_append(&heater_list, (void *)heater_new);








	xTaskCreate(prvUSARTEchoTask, (signed char *)"Echo", configMINIMAL_STACK_SIZE, NULL, mainECHO_TASK_PRIORITY, NULL);
	xTaskCreate(prvOneWireTask, (signed char *)"1Wire", configMINIMAL_STACK_SIZE, NULL, mainONEWIRE_TASK_PRIORITY, NULL);
	xTaskCreate(prvTempCtrlPIDTask, (signed char *)"PID", configMINIMAL_STACK_SIZE, NULL, mainTEMPCTRLPID_TASK_PRIORITY, NULL);



	vTaskStartScheduler();



	return 0;
	}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void GPIO_Configuration(void)
	{
	GPIO_InitTypeDef GPIO_InitStructure;
/*
//	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC , ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
*/
	}

//------------------------------------------------------------------------------

void NVIC_Configuration(void)
	{
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	// USART1 DMA Interrupt
	NVIC_InitStructure.NVIC_IRQChannel= DMA1_Channel4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority= 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority= 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd= ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// USART1 Interrupt
	NVIC_InitStructure.NVIC_IRQChannel= USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority= 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority= 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd= ENABLE;
	NVIC_Init(&NVIC_InitStructure);


	// Enable the TIM2 Interrupt
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);



	}


//------------------------------------------------------------------------------

static void prvUSARTEchoTask(void *pvParameters)
	{
	int result;

	//char *text= "test";
	char rxbuff[32];
	char txbuff[32];


	while (1)
		{

		result= serial_port_read(1, rxbuff, 32);


		switch (result)
			{

			case -1:
				{
				printf("recv: error\n");
				break;
				}

			case 0:
				{
				printf("recv: timeout\n");

				break;
				}

			default:
				{
				int x;
				printf("recv[%02d]:", result);

				for (x=0;x<result;x++)
					printf(" %02X", rxbuff[x]);

				printf("\n");

				break;
				}

			}

//		GPIO_SetBits(GPIOB , GPIO_Pin_0 | GPIO_Pin_1);
//		vTaskDelay(1000);
//		GPIO_ResetBits(GPIOB , GPIO_Pin_0 | GPIO_Pin_1);


	    }


	}

//------------------------------------------------------------------------------

static void prvSetupHardware(void)
	{
	/* Configure HCLK clock as SysTick clock source. */
	//SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
	}

//------------------------------------------------------------------------------

void vApplicationStackOverflowHook( xTaskHandle pxTask, signed char *pcTaskName )
	{
	/* This function will get called if a task overflows its stack.   If the
	parameters are corrupt then inspect pxCurrentTCB to find which was the
	offending task. */


	while (1)
		{
		GPIO_ResetBits(GPIOB , GPIO_Pin_0 | GPIO_Pin_1);
		Delay(300000);
		GPIO_SetBits(GPIOB , GPIO_Pin_0 | GPIO_Pin_1);
		Delay(300000);
		}


	( void ) pxTask;
	( void ) pcTaskName;

	for( ;; );
	}

//------------------------------------------------------------------------------

