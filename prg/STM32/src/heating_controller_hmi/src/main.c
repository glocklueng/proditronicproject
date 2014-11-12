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
#include <time.h>

#include "stm32f10x.h"


#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "FreeRTOSConfig.h"


#include "serial_port.h"
#include "ksystem.h"
#include "wdlist.h"
#include "stm32rtc.h"
#include "sdio_sd.h"
#include "tpgui.h"
#include "keyboard.h"
#include "modbus_mst.h"


#include "global.h"
#include "display_struct.c"


//unsigned char konfiguracja[512] __attribute__ ((section (".flash_config_data")));


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

void cmdline_date_set(unsigned char *param);
void cmdline_time_set(unsigned char *param);



#define mainBLOCK_Q_PRIORITY				( tskIDLE_PRIORITY + 2 )

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

// globalne

main_settings_s main_settings;

__IO uint32_t *cmos1; // BKP_DR1 - BKP_DR10,  10 rejestrów 16 bitowe
__IO uint32_t *cmos2; // BKP_DR11 - BKP_DR42, 32 rejestry 16 bitowe



extern void SD_LowLevel_Init(void);
extern unsigned char *cmd_inter_strtok_del;

int test_val;
float test_val_f;
time_t czas;

modbus_mst_channel_s modbus_mst_channel;


extern struct tm utime_tm;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

uint16_t modbus_addr_poll[]=
	{
	0x0400,
	0x0401,
	0x0402,
	0x0403,
	NULL
	};

//------------------------------------------------------------------------------

#define MODBUS_ADDR_THERM01				0x400
#define MODBUS_ADDR_THERM02				0x410



//------------------------------------------------------------------------------

float therm01_temp= 23.567;

modbus_data_s modbus_dta_therm01_status=
	{
	.modbus_addr= MODBUS_ADDR_THERM01,
	.data_type= MODBUS_DATA_TYPE_INT16,
	.dataptr= NULL,
	};

modbus_data_s modbus_dta_therm01_temp=
	{
	.modbus_addr= MODBUS_ADDR_THERM01 + 0x01,
	.data_type= MODBUS_DATA_TYPE_INT16,
	.dataptr= &screen_1_therm01_temp_label,
	};

modbus_data_s modbus_dta_therm02_status=
	{
	.modbus_addr= MODBUS_ADDR_THERM02,
	.data_type= MODBUS_DATA_TYPE_INT16,
	.dataptr= NULL,
	};

modbus_data_s modbus_dta_therm02_temp=
	{
	.modbus_addr= MODBUS_ADDR_THERM02 + 0x01,
	.data_type= MODBUS_DATA_TYPE_INT16,
	.dataptr= NULL,
	};





//------------------------------------------------------------------------------

modbus_data_s *modbus_data_poll[]=
	{
	&modbus_dta_therm01_status,
	&modbus_dta_therm01_temp,

	&modbus_dta_therm02_status,
	&modbus_dta_therm02_temp,

	NULL
	};


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------





//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//extern tpgui_screen_s main_screen;



//------------------------------------------------------------------------------

int main()
	{
	k_uchar x;

	USART_InitTypeDef USART_InitStructure;

	cmos1= (uint32_t)BKP_BASE + BKP_DR1;
	cmos2= (uint32_t)BKP_BASE + BKP_DR11;


	setenv("TZ","CET-1CEST,M3.5.0/2,M10.5.0/3",1);


	GPIO_Configuration();
	NVIC_Configuration();

	prvSetupHardware();

	stm32rtc_init();
//	SD_LowLevel_Init();


	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);	// ktimerlst_init
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); 	// PWM






	USART_InitStructure.USART_BaudRate = 57600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;

	serial_port_init(1, &USART_InitStructure); // console init
	serial_port_rx_timeout_set(1, 30000);

//	ktimerlst_init();

//	cmd_interpreter_init();





	tpgui_screen_init(&screen_1);
	tpgui_screen_item_add(&screen_1, (tpgui_screen_item_s *)&screen_1_label1);
	tpgui_screen_item_add(&screen_1, (tpgui_screen_item_s *)&screen_1_label2);



	screen_1_time_label.data_ptr= (void*)&utime_tm;
	tpgui_screen_item_add(&screen_1, (tpgui_screen_item_s *)&screen_1_time_label);

	screen_1_therm01_temp_label.data_ptr= &therm01_temp;
	screen_1_therm01_temp_label.precision= 3;
	tpgui_screen_item_add(&screen_1, (tpgui_screen_item_s *)&screen_1_therm01_temp_label);


	tpgui_screen_init(&screen_2);
	tpgui_screen_item_add(&screen_2, (tpgui_screen_item_s *)&screen_2_label1);
	tpgui_screen_item_add(&screen_2, (tpgui_screen_item_s *)&screen_2_label2);

	tpgui_screen_init(&screen_3);
	tpgui_screen_item_add(&screen_3, (tpgui_screen_item_s *)&screen_3_label1);
	tpgui_screen_item_add(&screen_3, (tpgui_screen_item_s *)&screen_3_label2);

	tpgui_screen_init(&screen_4);
	tpgui_screen_item_add(&screen_4, (tpgui_screen_item_s *)&screen_4_label1);
	tpgui_screen_item_add(&screen_4, (tpgui_screen_item_s *)&screen_4_label2);







/*
	test_val= 1024;
	tpgui_screen_item_variable1.data_ptr= (void *)&test_val;
	tpgui_screen_item_add(&main_screen, (tpgui_screen_item_s *)&tpgui_screen_item_variable1);
*/
/*
	test_val_f= 23.152;
	tpgui_screen_item_variable2.data_ptr= (void *)&test_val_f;
	tpgui_screen_item_add(&main_screen, (tpgui_screen_item_s *)&tpgui_screen_item_variable2);
*/

/*
	czas= time(0);
	localtime_r(&czas, &utime_tm);
	tpgui_screen_item_variable3.data_ptr= (void*)&utime_tm;
	tpgui_screen_item_add(&main_screen, (tpgui_screen_item_s *)&tpgui_screen_item_variable3);
*/
/*
	czas= time(0);
	localtime_r(&czas, &utime_tm);
	tpgui_screen_item_variable4.data_ptr= (void*)&utime_tm;
	tpgui_screen_item_add(&main_screen, (tpgui_screen_item_s *)&tpgui_screen_item_variable4);

	tpgui_screen_item_add(&main_screen, (tpgui_screen_item_s *)&main_screen_label4);
	tpgui_screen_item_add(&main_screen, (tpgui_screen_item_s *)&main_screen_label5);
*/

/*
	tpgui_menu_init(&main_menu);
	tpgui_menu_item_add(&main_menu, &tpgui_menu_item1);
	tpgui_menu_item_add(&main_menu, &tpgui_menu_item2);
	tpgui_menu_item_add(&main_menu, &tpgui_menu_item3);
	tpgui_menu_item_add(&main_menu, &tpgui_menu_item4);
	tpgui_menu_item_add(&main_menu, &tpgui_menu_item5);
	tpgui_menu_item_add(&main_menu, &tpgui_menu_item6);
	tpgui_menu_item_add(&main_menu, &tpgui_menu_item7);
	tpgui_menu_item_add(&main_menu, &tpgui_menu_item8);
*/


	tpgui_run(&screen_1);

	keyboard_init();
	keyboard_key_add(0x10, (uint32_t)GPIOC, GPIO_Pin_0);
	keyboard_key_add(0x12, (uint32_t)GPIOC, GPIO_Pin_1);
	keyboard_key_add(0x11, (uint32_t)GPIOC, GPIO_Pin_2);
	keyboard_key_add(0x13, (uint32_t)GPIOC, GPIO_Pin_3);

	keyboard_run();



// odczyt konfiguracji

	if (0)//*cmos1 != 0xA5A5)
		{
		// utrata danych konfiguracyjnych


		*cmos1= 0xA5A5;
		}




	main_settings.global_mode= GLOBAL_MODE_HEATING;

	modbus_mst_channel.chno= 1;
	modbus_mst_channel.modbus_addr_poll= modbus_addr_poll;
	modbus_mst_channel.modbus_data_poll= modbus_data_poll;

	modbus_mst_run(&modbus_mst_channel);


	//xTaskCreate(prvCmdInterpreterTask, (signed char *)"CMD", 512, NULL, mainTEMPCTRLPID_TASK_PRIORITY, NULL);



	vTaskStartScheduler();



	return 0;
	}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void GPIO_Configuration(void)
	{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE); 	// LCD PWM
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); 	// LCD PWM Contrast, PWM Backlight
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);



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

/*
	// Enable the TIM2 Interrupt
	// used by ktimerlst
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
*/

	// Enable the RTC Interrupt
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

/*
	// Enable the SDIO Interrupt
	NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
*/
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

void printf_datetime(struct tm *time_tm)
	{
	char buffer[30];
	strftime(buffer, 32, "%Y-%m-%d %H:%M:%S %z %a", time_tm);
	printf("date: %s\n", buffer);
	}

//------------------------------------------------------------------------------

void cmdline_date_set(unsigned char *param)
	{
	k_uchar nparam= 0;

	const k_uchar max_nparam= 3;
	k_ulong paramsval[max_nparam];

	time_t currtime;
	struct tm currtime_tm;

	if (!param)
		{
		currtime= time(0);
		localtime_r(&currtime, &currtime_tm);
		printf_datetime(&currtime_tm);
		return;
		}

	while (param && (nparam < max_nparam))
		{
		paramsval[nparam]= atoi(param);
		nparam++;
		param= strtok(NULL, cmd_inter_strtok_del);
		}

	if (nparam == 3)
		{
		if ((paramsval[0] >= 2000) && (paramsval[0] < 2100) && (paramsval[1] > 0) && (paramsval[1] <= 12) && (paramsval[2] > 0) && (paramsval[2] <= 31))
			{
			currtime= time(0);
			localtime_r(&currtime, &currtime_tm);

			currtime_tm.tm_year= paramsval[0] - 1900;
			currtime_tm.tm_mon= paramsval[1] - 1;
			currtime_tm.tm_mday= paramsval[2];

			currtime= mktime(&currtime_tm);
			stm32rtc_set(currtime);

			printf_datetime(&currtime_tm);
			}

		} // if (nparam == 3)


	}

//------------------------------------------------------------------------------

void cmdline_time_set(unsigned char *param)
	{
	k_uchar nparam= 0;

	const k_uchar max_nparam= 3;
	k_ulong paramsval[max_nparam];

	time_t currtime;
	struct tm currtime_tm;

	if (!param)
		{
		currtime= time(0);
		localtime_r(&currtime, &currtime_tm);
		printf_datetime(&currtime_tm);
		return;
		}

	while (param && (nparam < max_nparam))
		{
		paramsval[nparam]= atoi(param);
		nparam++;
		param= strtok(NULL, cmd_inter_strtok_del);
		}

	if (nparam == 3)
		{
		if ((paramsval[0] >= 0) && (paramsval[0] <= 23) && (paramsval[1] >= 0) && (paramsval[1] < 60) && (paramsval[2] >= 0) && (paramsval[2] < 60))
			{
			currtime= time(0);
			localtime_r(&currtime, &currtime_tm);

			currtime_tm.tm_hour= paramsval[0];
			currtime_tm.tm_min= paramsval[1];
			currtime_tm.tm_sec= paramsval[2];

			currtime= mktime(&currtime_tm);
			stm32rtc_set(currtime);

			printf_datetime(&currtime_tm);
			}

		} // if (nparam == 3)


	}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
