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


#include "global.h"

#include "1wire.h"
#include "1wire_thr.h"

#include "tempctrlpid_thr.h"
#include "cmd_interpreter.h"
#include "usersettings.h"


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

onewire_handler_s onewire_chnlst[ONEWIRE_NBUS];
heater_ctrl_handler_s heater_ctrl_chnlst[HEATER_NUMBER];

wdlist_s thermometer_list;
xSemaphoreHandle thermometer_sem;

wdlist_s heater_list;
xQueueHandle temp_pid_queue;


__IO uint32_t *cmos1; // BKP_DR1 - BKP_DR10,  10 rejestrów 16 bitowe
__IO uint32_t *cmos2; // BKP_DR11 - BKP_DR42, 32 rejestry 16 bitowe



extern void SD_LowLevel_Init(void);
extern unsigned char *cmd_inter_strtok_del;


const temp_preconfig_default[HEATER_PRECONFIG_TEMP_MAX]= {19, 20, 21, 22, 20, 20};

const heater_cfg_s heater_cfg_tab[]=
	{
	// termometr_index,
	{0x01},
	{0x01},
//	{2},
//	{3},

	};


// cmos2


k_uchar *heater_anticalc_status;			// + 0x0040 BKP_DR11, ilosc=1 rejestrów 16-bit
k_uchar *temp_preconfig_tab;				// + 0x0044 BKP_DR12, ilosc=3 rejestrów 16-bit
user_settings_s *user_settings_tab;			// + 0x0050 BKP_DR15, ilosc=20 rejestrów 16-bit
uint16_t *thermometer_id_crc8_tab;			// + 0x00A0 BKP_DR35, ilosc=8  rejestrów 16-bit




thermometer_s *thermometer_ptr_tab[THERMOMETERS_MAX];


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

	cmos1= (uint32_t)BKP_BASE + BKP_DR1;
	cmos2= (uint32_t)BKP_BASE + BKP_DR11;


	heater_anticalc_status=		(uint32_t)BKP_BASE + 0x0040;
	temp_preconfig_tab=			(uint32_t)BKP_BASE + 0x0044;
	user_settings_tab= 			(uint32_t)BKP_BASE + 0x0050;
	thermometer_id_crc8_tab=	(uint32_t)BKP_BASE + 0x00A0;



	setenv("TZ","CET-1CEST,M3.5.0/2,M10.5.0/3",1);


	GPIO_Configuration();
	NVIC_Configuration();

	prvSetupHardware();

	stm32rtc_init();
	SD_LowLevel_Init();


	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);	// ktimerlst_init
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); 	// PWM






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
	cmd_interpreter_cmd_append("date", &cmdline_date_set);
	cmd_interpreter_cmd_append("time", &cmdline_time_set);



	vSemaphoreCreateBinary(thermometer_sem);
	wdlist_init(&thermometer_list);

	wdlist_init(&heater_list);

	temp_pid_queue= xQueueCreate(1, 1);



	for (x=0;x<THERMOMETERS_MAX;x++)
		thermometer_ptr_tab[x]= NULL;

	for (x=0;x<HEATER_NUMBER;x++)
		heater_ctrl_chnlst[x].peripheral_addr= NULL;



	// uwaga! przy zmianie nastaw temperatury dla grzejnika wyczycic dane PID jak w funckji cmdline_heater_param_set
	// uwaga! przy zmianie nastaw temperatury dla grzejnika wyczycic dane PID jak w funckji cmdline_heater_param_set
	// uwaga! przy zmianie nastaw temperatury dla grzejnika wyczycic dane PID jak w funckji cmdline_heater_param_set
	// uwaga! przy zmianie nastaw temperatury dla grzejnika wyczycic dane PID jak w funckji cmdline_heater_param_set


// odczyt konfiguracji


	if (0)//*cmos1 != 0xA5A5)
		{
		// utrata danych konfiguracyjnych



		for (x=0;x<THERMOMETERS_MAX;x++)
			{
			k_uchar *idptr= (k_uchar *)thermometer_id_crc8_tab + (x>>1)*4 + (x & 1);
			STM32_BKP_REG_BYTE_WR(idptr, 0x00);
			}



		*cmos1= 0xA5A5;
		}


	for (x=0;x<USERSETTINGS_MAX;x++)
		{
		user_settings_s *user_settings= &user_settings_tab[x];

		STM32_BKP_REG_BYTE_WR(&user_settings->week_days, 0x00);
		STM32_BKP_REG_BYTE_WR(&user_settings->heaters, 0x00);
		STM32_BKP_REG_BYTE_WR(&user_settings->begin_time, 0x00);
		STM32_BKP_REG_BYTE_WR(&user_settings->pred_temp_indx, 0x00);
		}


	for (x=0;x<HEATER_PRECONFIG_TEMP_MAX;x++)
		{
		k_uchar *pred_addr= temp_preconfig_tab + (int)((x >> 1) * 4 + (x & 1));
		STM32_BKP_REG_BYTE_WR(pred_addr, temp_preconfig_default[x] * 2)
		}


//	*heater_anticalc_status= 0xFF;



	//for (x=0;x<7;x++)
		{
		user_settings_s *user_settings;
		k_uchar week_days;


		week_days= 0x80 | 0x3E; // mon - fri

		user_settings= &user_settings_tab[0];
		STM32_BKP_REG_BYTE_WR(&user_settings->week_days, week_days);
		STM32_BKP_REG_BYTE_WR(&user_settings->heaters, 0x03);
		STM32_BKP_REG_BYTE_WR(&user_settings->begin_time, 33);		// 05:30
		STM32_BKP_REG_BYTE_WR(&user_settings->pred_temp_indx, HEATER_TEMP_DAY_LIFE);

/*
		user_settings= &user_settings_tab[1];
		STM32_BKP_REG_BYTE_WR(&user_settings->week_days, week_days);
		STM32_BKP_REG_BYTE_WR(&user_settings->heaters, 0x03);
		STM32_BKP_REG_BYTE_WR(&user_settings->begin_time, 45);		// 07:30
		STM32_BKP_REG_BYTE_WR(&user_settings->pred_temp_indx, HEATER_TEMP_WORK);
*/
/*
		user_settings= &user_settings_tab[2];
		STM32_BKP_REG_BYTE_WR(&user_settings->week_days, week_days);
		STM32_BKP_REG_BYTE_WR(&user_settings->heaters, 0x03);
		STM32_BKP_REG_BYTE_WR(&user_settings->begin_time, 78);		// 13:00
		STM32_BKP_REG_BYTE_WR(&user_settings->pred_temp_indx, HEATER_TEMP_DAY_LIFE);
*/
/*
		user_settings= &user_settings_tab[3];
		STM32_BKP_REG_BYTE_WR(&user_settings->week_days, week_days);
		STM32_BKP_REG_BYTE_WR(&user_settings->heaters, 0x03);
		STM32_BKP_REG_BYTE_WR(&user_settings->begin_time, 132);		// 22:00
		STM32_BKP_REG_BYTE_WR(&user_settings->pred_temp_indx, HEATER_TEMP_NIGHT);
*/


		week_days= 0x80 | 0x41; // sat, sun

		user_settings= &user_settings_tab[4];
		STM32_BKP_REG_BYTE_WR(&user_settings->week_days, week_days);
		STM32_BKP_REG_BYTE_WR(&user_settings->heaters, 0x03);
		STM32_BKP_REG_BYTE_WR(&user_settings->begin_time, 33);		// 05:30
		STM32_BKP_REG_BYTE_WR(&user_settings->pred_temp_indx, HEATER_TEMP_DAY_LIFE);
/*
		user_settings= &user_settings_tab[5];
		STM32_BKP_REG_BYTE_WR(&user_settings->week_days, week_days);
		STM32_BKP_REG_BYTE_WR(&user_settings->heaters, 0x03);
		STM32_BKP_REG_BYTE_WR(&user_settings->begin_time, 132);		// 22:00
		STM32_BKP_REG_BYTE_WR(&user_settings->pred_temp_indx, HEATER_TEMP_NIGHT);
*/

		}



	main_settings.global_mode= GLOBAL_MODE_HEATING;

	


	// grzejniki

	for (x=0;x<(sizeof(heater_cfg_tab)/sizeof(heater_cfg_s));x++)
		{
		heater_cfg_s *heater_cfg= (heater_cfg_s *)&heater_cfg_tab[x];
		heater_s *heater_new;

		heater_new= (heater_s *)malloc(sizeof(heater_s));
		heater_new->indx= x;
		heater_new->state= 0x00;
		heater_new->temp_setpoint= 0;
		heater_new->temp_offset= 0;

		heater_new->temp_setpoint_forced_timeout= 0;



		heater_new->temp_prev= 0;
		heater_new->owp_state= 0x00;
		heater_new->owp_begin_time= 0;

		heater_new->anti_calc_state= 0x00;

		heater_new->max_throttle_timeout_active= false;
		heater_new->max_throttle_timeout= 0;
		heater_new->thermometer= heater_cfg->thermometer_no; // pocz¹tkowo indeks termometru, potem wskaŸnik
		heater_new->heater_ctrl_handler= &heater_ctrl_chnlst[x];

		wdlist_append(&heater_list, (void *)heater_new);

		} // heater_cfg_tab





	//xTaskCreate(prvUSARTEchoTask, (signed char *)"Echo", 512, NULL, mainECHO_TASK_PRIORITY, NULL);
	xTaskCreate(prvOneWireTask, (signed char *)"1Wire", 512, NULL, mainONEWIRE_TASK_PRIORITY, NULL);
	xTaskCreate(prvTempCtrlPIDTask, (signed char *)"PID", 1024, NULL, mainTEMPCTRLPID_TASK_PRIORITY, NULL);
	xTaskCreate(prvCmdInterpreterTask, (signed char *)"CMD", 512, NULL, mainTEMPCTRLPID_TASK_PRIORITY, NULL);



	vTaskStartScheduler();



	return 0;
	}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void GPIO_Configuration(void)
	{
	GPIO_InitTypeDef GPIO_InitStructure;

//	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC , ENABLE);
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB , ENABLE);

//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

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


	// Enable the RTC Interrupt
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);


	// Enable the SDIO Interrupt
	NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
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
