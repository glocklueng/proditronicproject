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
#include "cmd_interpreter.h"


unsigned char konfiguracja[512] __attribute__ ((section (".flash_config_data")));


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

main_settings_s main_settings;

onewire_handler_s onewire_chnlst[ONEWIRE_NBUS];
heater_ctrl_handler_s heater_ctrl_chnlst[HEATER_NUMBER];


wdlist_s thermometer_list;
xSemaphoreHandle thermometer_sem;

wdlist_s heater_list;
xQueueHandle temp_pid_queue;




const thermometer_cfg_s thermometer_cfg_tab[]=
	{

	{{0x00, 0x00, 0x04, 0xB1, 0x92, 0x3A}, 0},
//	{{0x00, 0x00, 0x04, 0xB1, 0x6E, 0x30}, 1},
//	{{0x00, 0x00, 0x04, 0xB1, 0x8D, 0xB8}, 2},
//	{{0x00, 0x00, 0x04, 0xB1, 0xD7, 0x6A}, 3},

	};

const heater_cfg_s heater_cfg_tab[]=
	{
	// termometr_index,
	{0},
	{0},
//	{2},
//	{3},

	};




//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void  Delay (uint32_t nCount)
{
  for(; nCount != 0; nCount--);
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//ktimer_spec_s timer_1;
//static int cntr= 0;




//------------------------------------------------------------------------------

int main()
	{
	k_uchar x;

	USART_InitTypeDef USART_InitStructure;

	GPIO_Configuration();
	NVIC_Configuration();

	prvSetupHardware();


	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	//RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); // PWM

	USART_InitStructure.USART_BaudRate = 57600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;

	serial_port_init(1, &USART_InitStructure); // console init
	serial_port_rx_timeout_set(1, 30000);

	ktimerlst_init();

	cmd_interpreter_init();

	cmd_interpreter_cmd_append("heater", &cmdline_heater_param_set);
	cmd_interpreter_cmd_append("therm", &cmdline_thermometer_temp_set);
	cmd_interpreter_cmd_append("pwmmaxduty", &cmdline_heater_pwm_duty_ratio_max_set);
	cmd_interpreter_cmd_append("pid", &cmdline_pid_set);



	vSemaphoreCreateBinary(thermometer_sem);
	wdlist_init(&thermometer_list);

	wdlist_init(&heater_list);

	temp_pid_queue= xQueueCreate(1, 1);



	for (x=0;x<HEATER_NUMBER;x++)
		heater_ctrl_chnlst[x].peripheral_addr= NULL;



	// uwaga! przy zmianie nastaw temperatury dla grzejnika wyczycic dane PID jak w funckji cmdline_heater_param_set
	// uwaga! przy zmianie nastaw temperatury dla grzejnika wyczycic dane PID jak w funckji cmdline_heater_param_set
	// uwaga! przy zmianie nastaw temperatury dla grzejnika wyczycic dane PID jak w funckji cmdline_heater_param_set
	// uwaga! przy zmianie nastaw temperatury dla grzejnika wyczycic dane PID jak w funckji cmdline_heater_param_set


// odczyt konfiguracji


	main_settings.global_mode= GLOBAL_MODE_HEATING;


	// termometry

	for (x=0;x<(sizeof(thermometer_cfg_tab)/sizeof(thermometer_cfg_s));x++)
		{
		thermometer_cfg_s *thermometer_cfg= (thermometer_cfg_s *)&thermometer_cfg_tab[x];
		thermometer_s *therm_new;

		therm_new= (thermometer_s *)malloc(sizeof(thermometer_s));
		therm_new->indx= x;
		therm_new->onewire_handler= &onewire_chnlst[thermometer_cfg->chn_no];
		therm_new->dev_id= thermometer_cfg->dev_id;
		therm_new->temp_vaild= false;
		therm_new->temp_read_error_cntr= 0;
		therm_new->error_code= 0;
		therm_new->temp_read_0x0550_cntr= 0;
		therm_new->temp_debug_f= false;

		wdlist_append(&thermometer_list, (void *)therm_new);

		} // thermometer_cfg_tab

	
	// grzejniki

	for (x=0;x<(sizeof(heater_cfg_tab)/sizeof(heater_cfg_s));x++)
		{
		heater_cfg_s *heater_cfg= (heater_cfg_s *)&heater_cfg_tab[x];
		heater_s *heater_new;
		wdlist_entry_s *therm_entry;
		k_uchar tindx;


		tindx= 0;
		therm_entry= thermometer_list.first_entry;

		while (therm_entry && (tindx < heater_cfg->thermometer_no))
			{
			tindx++;
			therm_entry= therm_entry->next;
			}

		heater_new= (heater_s *)malloc(sizeof(heater_s));
		heater_new->indx= x;
		heater_new->temp_zadana= 22000;
		heater_new->temp_offset= 0;
		heater_new->max_throttle_timeout_active= false;
		heater_new->thermometer= (therm_entry && (tindx == heater_cfg->thermometer_no)) ? (thermometer_s *)therm_entry->data : NULL;
		heater_new->heater_ctrl_handler= &heater_ctrl_chnlst[x];

		wdlist_append(&heater_list, (void *)heater_new);

		} // heater_cfg_tab





	//xTaskCreate(prvUSARTEchoTask, (signed char *)"Echo", configMINIMAL_STACK_SIZE, NULL, mainECHO_TASK_PRIORITY, NULL);
	xTaskCreate(prvOneWireTask, (signed char *)"1Wire", configMINIMAL_STACK_SIZE, NULL, mainONEWIRE_TASK_PRIORITY, NULL);
	xTaskCreate(prvTempCtrlPIDTask, (signed char *)"PID", configMINIMAL_STACK_SIZE, NULL, mainTEMPCTRLPID_TASK_PRIORITY, NULL);
	xTaskCreate(prvCmdInterpreterTask, (signed char *)"CMD", configMINIMAL_STACK_SIZE, NULL, mainTEMPCTRLPID_TASK_PRIORITY, NULL);



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
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB , ENABLE);

//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
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
	// used by ktimerlst
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

		msleep(1000);

		led_switch();


/*
		result= serial_port_read(1, rxbuff, 32);


		switch (result)
			{

			case -1:
				{
				printf("recv: error\n");
				printf("%s\n", konfiguracja);
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
*/
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

