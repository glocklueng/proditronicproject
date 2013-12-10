#include <stdlib.h>
#include <string.h>
#include <math.h>


#include <FreeRTOS.h>
#include "task.h"
#include "semphr.h"
#include "queue.h"


#include "wdlist.h"
#include "ksystem.h"
#include "global.h"

#include "tempctrlpid_thr.h"

//-----------------------------------------------------------------------------

// PID wspó³czynniki
const k_long Kp= 10000;			// 10,000
const k_long Ki= 10;			// 0,010
const k_long Kd= 10000;			// 10,000

// nastawy min, max

#define PID_REG_MIN				(k_long)0		//  0,000
#define PID_REG_MAX				(k_long)10000	// 10,000


#define PWM_WARM_DOWN_FACTOR		200			// [0,1%]
#define PWM_WARM_CONST_FACTOR		300			// [0,1%]



#define PWM_PERIOD				60 				// sec
#define PWM_TIMER_FREQ			2000			// Hz

//#define PWM_CYCLE_TICKS			(uint16_t)(PWM_PERIOD * PWM_TIMER_FREQ)
//#define	PWM_PID_RATIO			(float)((float)PWM_CYCLE_TICKS / (float)PID_REG_MAX)


// 30%/20


//-----------------------------------------------------------------------------

extern main_settings_s main_settings;

extern xSemaphoreHandle thermometer_sem;

extern wdlist_s heater_list;
extern xQueueHandle temp_pid_queue;

extern heater_ctrl_handler_s heater_ctrl_chnlst[HEATER_NUMBER];
extern unsigned char *cmd_inter_strtok_del;

//-----------------------------------------------------------------------------

k_long temp_pid_process(PID_data_s *PID_data, k_long uchyb);
k_long pwm_channel_init(heater_ctrl_handler_s *heater_ctrl_handler);


void led_switch()
	{
	static char ledstate= 0;

	if (ledstate)
		{
		ledstate= 0;
		GPIO_ResetBits(GPIOB , GPIO_Pin_1);
		}
	else
		{
		ledstate= 1;
		GPIO_SetBits(GPIOB , GPIO_Pin_1);
		}

	}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void prvTempCtrlPIDTask(void *pvParameters)
	{
	k_uchar queue_dta;

	wdlist_entry_s *heater_entry= NULL;
	heater_s *heater;
	thermometer_s *thermometer;
	heater_ctrl_handler_s *heater_ctrl_handler;
	k_long heater_temp;
	k_long temp_uchyb;
	k_long ctrl_param;

	int licznik= 0;
	uint16_t duty_ratio;

	//k_uchar





/*
	heater_ctrl_chnlst[0].peripheral_addr= (uint32_t)GPIOB;
	heater_ctrl_chnlst[0].ctrl_pin= GPIO_Pin_1;
	heater_ctrl_chnlst[0].period= PWM_PERIOD;
	heater_ctrl_chnlst[0].pwm_chn= 4;
	pwm_channel_init(&heater_ctrl_chnlst[0]);
*/


	heater_ctrl_chnlst[0].peripheral_addr= (uint32_t)GPIOB;
	heater_ctrl_chnlst[0].ctrl_pin= GPIO_Pin_6;				// TIM4_CH1
	heater_ctrl_chnlst[0].period= PWM_PERIOD;
	heater_ctrl_chnlst[0].pwm_chn= 1;
	pwm_channel_init(&heater_ctrl_chnlst[0]);

	heater_ctrl_chnlst[1].peripheral_addr= (uint32_t)GPIOB;
	heater_ctrl_chnlst[1].ctrl_pin= GPIO_Pin_7;				// TIM4_CH2
	heater_ctrl_chnlst[1].period= PWM_PERIOD;
	heater_ctrl_chnlst[1].pwm_chn= 2;
	pwm_channel_init(&heater_ctrl_chnlst[1]);
/*
	heater_ctrl_chnlst[2].peripheral_addr= (uint32_t)GPIOB;
	heater_ctrl_chnlst[2].ctrl_pin= GPIO_Pin_8;				// TIM4_CH3
	heater_ctrl_chnlst[2].period= PWM_PERIOD;
	heater_ctrl_chnlst[2].pwm_chn= 3;
	pwm_channel_init(&heater_ctrl_chnlst[2]);

	heater_ctrl_chnlst[3].peripheral_addr= (uint32_t)GPIOB;
	heater_ctrl_chnlst[3].ctrl_pin= GPIO_Pin_9;				// TIM4_CH4
	heater_ctrl_chnlst[3].period= PWM_PERIOD;
	heater_ctrl_chnlst[3].pwm_chn= 4;
	pwm_channel_init(&heater_ctrl_chnlst[3]);
*/

	pwm_channel_set(&heater_ctrl_chnlst[0], 1000);
	pwm_channel_set(&heater_ctrl_chnlst[1], 1000);
//	pwm_channel_set(&heater_ctrl_chnlst[2], 500);
//	pwm_channel_set(&heater_ctrl_chnlst[3], 700);



	heater_entry= heater_list.first_entry;
	while (heater_entry)
		{
		heater= (heater_s *)heater_entry->data;

		heater->PID_data.uchyb_prev= 0x80000000; // magic value
		heater->PID_data.I_prev= 0;

		heater_entry= heater_entry->next;
		}



	queue_dta= 0x01; // init
	xQueueSend(temp_pid_queue, &queue_dta, (portTickType)0);


/*
	duty_ratio= 1000;
	pwm_channel_set(&heater_ctrl_chnlst[0], duty_ratio);

	duty_ratio= 1000;
	pwm_channel_set(&heater_ctrl_chnlst[1], duty_ratio);

	licznik= 180;
	while (licznik != 0)
		{
		printf("%03d: set: %d\n", licznik, duty_ratio);
		msleep(1000);
		licznik-= 1;
		}




	duty_ratio= 200;
	pwm_channel_set(&heater_ctrl_chnlst[0], duty_ratio);

	duty_ratio= 200;
	pwm_channel_set(&heater_ctrl_chnlst[1], duty_ratio);

	licznik= 18000;
	while (licznik != 0)
		{
		printf("%03d: set: %d\n", licznik, duty_ratio);
		msleep(1000);
		licznik-= 1;
		}
*/
/*
	duty_ratio= 400;
	pwm_channel_set(&heater_ctrl_chnlst[0], duty_ratio);

	licznik= 180;
	while (licznik != 0)
		{
		printf("%03d: set: %d\n", licznik, duty_ratio);
		msleep(1000);
		licznik-= 1;
		}


	duty_ratio= 300;
	pwm_channel_set(&heater_ctrl_chnlst[0], duty_ratio);

	licznik= 180;
	while (licznik != 0)
		{
		printf("%03d: set: %d\n", licznik, duty_ratio);
		msleep(1000);
		licznik-= 1;
		}


	duty_ratio= 200;
	pwm_channel_set(&heater_ctrl_chnlst[0], duty_ratio);

	licznik= 180;
	while (licznik != 0)
		{
		printf("%03d: set: %d\n", licznik, duty_ratio);
		msleep(1000);
		licznik-= 1;
		}


	duty_ratio= 100;
	pwm_channel_set(&heater_ctrl_chnlst[0], duty_ratio);

	licznik= 180;
	while (licznik != 0)
		{
		printf("%03d: set: %d\n", licznik, duty_ratio);
		msleep(1000);
		licznik-= 1;
		}

*/


	while (1)
		{

		xQueueReceive(temp_pid_queue, &queue_dta, portMAX_DELAY); // czekam na zakoñczenie cyklu odczytu temperatur


		heater_entry= heater_list.first_entry;
		while (heater_entry)
			{
			heater= (heater_s *)heater_entry->data;

			thermometer= heater->thermometer;
			heater_ctrl_handler= heater->heater_ctrl_handler;



/*
			switch (main_settings.global_mode)
				{

				case GLOBAL_MODE_IDLE:
					{






					break;
					} // GLOBAL_MODE_IDLE


				case GLOBAL_MODE_HEATING:
					{









					break;
					} // GLOBAL_MODE_HEATING


				} // switch (main_settings.global_mode)
*/



			if (thermometer)
				{

				xSemaphoreTake(thermometer_sem, 0);



				if ((queue_dta == 0x01) || !thermometer->temp_vaild)
					{
					// zerowanie, wy³¹cz grzanie

					xSemaphoreGive(thermometer_sem);

					// jesli zmiana
					//pwm_channel_set

					
					}
				else
					{

					heater_temp= (k_long)thermometer->temp_value;

					xSemaphoreGive(thermometer_sem);

					heater_temp*= 1000;
					heater_temp>>= 4;
					
					if (heater->temp_offset != 0)
						heater_temp+= heater->temp_offset;

					temp_uchyb= heater->temp_zadana - heater_temp;
					//temp_uchyb= 100;

					if (heater->PID_data.uchyb_prev == 0x80000000)
						heater->PID_data.uchyb_prev= temp_uchyb;


					ctrl_param= temp_pid_process(&heater->PID_data, temp_uchyb);


					if (temp_uchyb < 0)
						{
						// obni¿anie temperatury

						//duty_ratio=

						}
					else
					if (ctrl_param == PID_REG_MIN)
						{
						// utrzymanie temperatury



						}
					else
						{
						// nagrzewanie





						}





					if (licznik < 20)
						duty_ratio= 500;//(uint16_t)round((((float)ctrl_param / (float)PID_REG_MAX) * 1000));
					else
						duty_ratio= 100;




					//printf("<HTR> dev[%02d]: therm[%02d] tset=%2.3f toff=%2.3 \n", heater->indx, heater->thermometer->indx, (float)heater->temp_zadana/1000, (float)heater->temp_offset/1000);


					if (duty_ratio > 1000)
						duty_ratio= 1000;

					if (heater_ctrl_handler->duty_ratio != duty_ratio)
						{
						printf("set: %d\n", duty_ratio);
						//pwm_channel_set(heater_ctrl_handler, duty_ratio); // 0 - 1000
						}




					printf("%06d  %06d  %06d  %06d duty_ratio: %d\n", heater->temp_zadana, heater_temp, temp_uchyb, ctrl_param, duty_ratio);


					licznik++;


					}


				} // if (thermometer)
			else
				{

				// brak zdefiniowanego termometru dla grzejnika


				}



			heater_entry= heater_entry->next;
			}


		} // while (1)

	}

//-----------------------------------------------------------------------------

k_long temp_pid_process(PID_data_s *PID_data, k_long uchyb)
	{
	k_long result;
    
	k_long P= Kp * uchyb;
	k_long I= Ki * (uchyb + PID_data->uchyb_prev) / 2 + PID_data->I_prev;
	k_long D= Kd * (uchyb - PID_data->uchyb_prev);
	
	PID_data->uchyb_prev= uchyb;
	result= P + I + D;
	result/= 1000;

	if (result < PID_REG_MIN)
        result= PID_REG_MIN;
	else
	if (result > PID_REG_MAX)
		result= PID_REG_MAX;
	else
		PID_data->I_prev= I;

	return result;
	}

//-----------------------------------------------------------------------------

k_long pwm_channel_init(heater_ctrl_handler_s *heater_ctrl_handler)
	{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	uint16_t PrescalerValue;

	static unsigned char GPIOx_clk_enable= 0x00;
	static unsigned char pwm_timer_enabled= 0x00;


	if (!heater_ctrl_handler)
		return -1;

	switch (heater_ctrl_handler->peripheral_addr)
		{

		case (uint32_t)GPIOB:
			{

			if (!(GPIOx_clk_enable & GPIOB_CLK_ENABLE_BIT))
				{
				// only once
				RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
				GPIOx_clk_enable|= GPIOB_CLK_ENABLE_BIT;
				}

			break;
			} // GPIOB

		default:
			{
			return -1;
			}

		} // switch (heater_ctrl_handler->peripheral_addr)


	if (!pwm_timer_enabled)
		{

		PrescalerValue= (uint16_t)(SystemCoreClock / PWM_TIMER_FREQ) - 1;

		// TIMx configuration
		TIM_TimeBaseStructure.TIM_Period= (uint16_t)(heater_ctrl_handler->period * PWM_TIMER_FREQ);
		TIM_TimeBaseStructure.TIM_Prescaler= PrescalerValue;
		TIM_TimeBaseStructure.TIM_ClockDivision= 0;
		TIM_TimeBaseStructure.TIM_CounterMode= TIM_CounterMode_Up;
		TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

		// Immediate load of TIM2,TIM3 and TIM4 Prescaler values
		TIM_PrescalerConfig(TIM4, PrescalerValue, TIM_PSCReloadMode_Immediate);

		TIM_ARRPreloadConfig(TIM4, ENABLE);

		// TIM2, TIM3 and TIM4 enable counters
		TIM_Cmd(TIM4, ENABLE);



		pwm_timer_enabled= 0x01;
		}




/*
	PrescalerValue= (uint16_t)(SystemCoreClock / PWM_TIMER_FREQ) - 1;

	// TIMx configuration
	TIM_TimeBaseStructure.TIM_Period= (uint16_t)(heater_ctrl_handler->period * PWM_TIMER_FREQ);
	TIM_TimeBaseStructure.TIM_Prescaler= PrescalerValue;
	TIM_TimeBaseStructure.TIM_ClockDivision= 0;
	TIM_TimeBaseStructure.TIM_CounterMode= TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	GPIO_InitStructure.GPIO_Pin= heater_ctrl_handler->ctrl_pin;
	GPIO_InitStructure.GPIO_Speed= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode= GPIO_Mode_Out_PP;
	GPIO_Init((GPIO_TypeDef *)heater_ctrl_handler->peripheral_addr, &GPIO_InitStructure);
	GPIO_ResetBits((GPIO_TypeDef *)heater_ctrl_handler->peripheral_addr, heater_ctrl_handler->ctrl_pin);
*/

	GPIO_InitStructure.GPIO_Pin= heater_ctrl_handler->ctrl_pin;
	GPIO_InitStructure.GPIO_Speed= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode= GPIO_Mode_Out_PP;
	GPIO_Init((GPIO_TypeDef *)heater_ctrl_handler->peripheral_addr, &GPIO_InitStructure);
	GPIO_ResetBits((GPIO_TypeDef *)heater_ctrl_handler->peripheral_addr, heater_ctrl_handler->ctrl_pin);








	heater_ctrl_handler->duty_ratio= 0;


	return 0;
	}

//-----------------------------------------------------------------------------

k_long pwm_channel_set(heater_ctrl_handler_s *heater_ctrl_handler, uint16_t duty_ratio)
	{
	uint16_t PrescalerValue;
	TIM_OCInitTypeDef TIM_OCInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	float duty_f;


	if (!heater_ctrl_handler)
		return -1;


	if (duty_ratio > 1000)
		duty_ratio= 1000;

	heater_ctrl_handler->duty_ratio= duty_ratio;

	if (heater_ctrl_handler->peripheral_addr == NULL)
		return -1;


	if (duty_ratio == 0)
		{
		// PWM OFF

		GPIO_InitStructure.GPIO_Pin= heater_ctrl_handler->ctrl_pin;
		GPIO_InitStructure.GPIO_Speed= GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode= GPIO_Mode_Out_PP;
		GPIO_Init((GPIO_TypeDef *)heater_ctrl_handler->peripheral_addr, &GPIO_InitStructure);
		GPIO_ResetBits((GPIO_TypeDef *)heater_ctrl_handler->peripheral_addr, heater_ctrl_handler->ctrl_pin);

		//TIM_Cmd(TIM3, DISABLE);

		}
	else
	if (duty_ratio == 1000)
		{
		// static ON

		GPIO_InitStructure.GPIO_Pin= heater_ctrl_handler->ctrl_pin;
		GPIO_InitStructure.GPIO_Speed= GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode= GPIO_Mode_Out_PP;
		GPIO_Init((GPIO_TypeDef *)heater_ctrl_handler->peripheral_addr, &GPIO_InitStructure);
		GPIO_SetBits((GPIO_TypeDef *)heater_ctrl_handler->peripheral_addr, heater_ctrl_handler->ctrl_pin);

		//TIM_Cmd(TIM3, DISABLE);

		}
	else
		{
		// PWM ON

		//TIM_Cmd(TIM3, DISABLE);
/*
		PrescalerValue= (uint16_t)(SystemCoreClock / PWM_TIMER_FREQ) - 1;

		// Immediate load of TIM2,TIM3 and TIM4 Prescaler values
		TIM_PrescalerConfig(TIM3, PrescalerValue, TIM_PSCReloadMode_Immediate);
*/
		duty_f= (float)(heater_ctrl_handler->period * PWM_TIMER_FREQ) * (float)duty_ratio / 1000;

		// Output Compare Timing Mode configuration: Channel1
		TIM_OCStructInit(&TIM_OCInitStructure);
		TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
		TIM_OCInitStructure.TIM_OutputState= TIM_OutputState_Enable;
		TIM_OCInitStructure.TIM_Pulse= (uint16_t)duty_f;
		TIM_OCInitStructure.TIM_OCPolarity= TIM_OCPolarity_High;

		switch (heater_ctrl_handler->pwm_chn)
			{

			case 1:
				{
				TIM_OC1Init(TIM4, &TIM_OCInitStructure);
				TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);
				break;
				}

			case 2:
				{
				TIM_OC2Init(TIM4, &TIM_OCInitStructure);
				TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);
				break;
				}

			case 3:
				{
				TIM_OC3Init(TIM4, &TIM_OCInitStructure);
				TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);
				break;
				}

			case 4:
				{
				TIM_OC4Init(TIM4, &TIM_OCInitStructure);
				TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);
				break;
				}

			} // switch (heater_ctrl_handler->pwm_chn)


		//TIM_ARRPreloadConfig(TIM3, ENABLE);

		// TIM2, TIM3 and TIM4 enable counters
		//TIM_Cmd(TIM3, ENABLE);



		GPIO_InitStructure.GPIO_Pin= heater_ctrl_handler->ctrl_pin;
		GPIO_InitStructure.GPIO_Speed= GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode= GPIO_Mode_AF_PP;
		GPIO_Init((GPIO_TypeDef *)heater_ctrl_handler->peripheral_addr, &GPIO_InitStructure);

		}


	return 0;
	}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/*

heater <number> pwm <value>

*/

void cmdline_heater_pwm_set(unsigned char *param)
	{
	unsigned char *tokptr;
	k_uchar indx= 0;

	heater_s *heater= NULL;
	k_uchar cmd= 0xFF;
	k_uchar nparam= 0;

	const k_uchar max_nparam= 3;
	k_ulong paramsval[max_nparam];

	while (param)
		{

		switch (indx)
			{

			case 0:	// indeks grzejnika
				{
				wdlist_entry_s *entry;
				heater_s *heater_tmp;
				k_uchar hindx;

				hindx= atoi(param);

				entry= heater_list.first_entry;
				while (entry)
					{
					heater_tmp= (heater_s *)entry->data;

					if (heater_tmp->indx == hindx)
						{
						heater= heater_tmp;
						break;
						}

					entry= entry->next;
					}

				break;
				}


			case 1:
				{

				if (!strcmp(param, "pwm"))
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

	printf("cmd: %08X %02X %d\n", heater, cmd, nparam);

	if (heater)
		{

		switch (cmd)
			{

			case 0x01:	// pwm
				{

				if (nparam == 1)
					{
					printf("<HTR> dev[%02d]: PWM set to %d\n", heater->indx, paramsval[0]);
					pwm_channel_set(heater->heater_ctrl_handler, paramsval[0]);
					}

				break;
				}

			} // switch (cmd)


		}




	}

//-----------------------------------------------------------------------------

void cmdline_heater_pwm_set2(unsigned char *param)
	{


	}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

