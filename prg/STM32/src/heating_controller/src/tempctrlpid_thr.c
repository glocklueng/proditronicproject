#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>


#include "stm32f10x.h"
#include "sdio_sd.h"
#include "ff.h"


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
const k_long Kp_def= 14000;			// 16,000 ok
const k_long Ki_def= 300;			// 0,010			400 -ok
const k_long Kd_def= 12000;			// 10,000

// nastawy min, max

#define PID_REG_MIN				(k_long)0		//  0,000
#define PID_REG_MAX				(k_long)10000	// 10,000


#define PWM_WARM_DOWN_FACTOR		200			// [0,1%]
#define PWM_WARM_CONST_FACTOR		500			// [0,1%]



#define PWM_PERIOD						20 				// sec
#define PWM_TIMER_FREQ					1200			// Hz
#define PWM_DUTY_RATIO_MAX_DEFAULT		1000
#define PWM_DUTY_RATIO_CUTOFF			350

#define PWM_MAX_THROTTLE_TIMEOUT		600				// sec

#define LOGFILE_BUFFER_LENGTH			256

//#define PWM_CYCLE_TICKS			(uint16_t)(PWM_PERIOD * PWM_TIMER_FREQ)
//#define	PWM_PID_RATIO			(float)((float)PWM_CYCLE_TICKS / (float)PID_REG_MAX)


// 45%/20 !!!


//-----------------------------------------------------------------------------

extern main_settings_s main_settings;

extern xSemaphoreHandle thermometer_sem;

extern wdlist_s heater_list;
extern xQueueHandle temp_pid_queue;

extern heater_ctrl_handler_s heater_ctrl_chnlst[HEATER_NUMBER];
extern unsigned char *cmd_inter_strtok_del;





int pwm_duty_ratio_max;
k_long Kp, Ki, Kd;

FATFS fs;
FIL fsrc;
bool logfile_open_f;
struct tm currtime_tm;



//-----------------------------------------------------------------------------

k_long temp_pid_process(PID_data_s *PID_data, k_long uchyb);
k_long pwm_channel_init(heater_ctrl_handler_s *heater_ctrl_handler);

void tempctrl_write2logfile();



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/*
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
*/


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

	uint16_t duty_ratio;

	portTickType ticks_curr;
	portTickType ticks_prev;
	k_ulong time_elapsed_sec;

	time_t currtime;
	char conbuffer[32];



	pwm_duty_ratio_max= PWM_DUTY_RATIO_MAX_DEFAULT;

	Kp= Kp_def;
	Ki= Ki_def;
	Kd= Kd_def;

	logfile_open_f= false;




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





	msleep(5000);







	heater_entry= heater_list.first_entry;
	while (heater_entry)
		{
		heater= (heater_s *)heater_entry->data;

		heater->PID_data.uchyb_prev= 0x80000000; // magic value
		heater->PID_data.I_prev= 0;

		heater_entry= heater_entry->next;
		}


	ticks_prev= xTaskGetTickCount();

	queue_dta= 0x01; // init
	xQueueSend(temp_pid_queue, &queue_dta, (portTickType)0);



	while (1)
		{


		xQueueReceive(temp_pid_queue, &queue_dta, portMAX_DELAY); // czekam na zakoñczenie cyklu odczytu temperatur


		ticks_curr= xTaskGetTickCount();
		time_elapsed_sec= (ticks_curr >= ticks_prev) ? (ticks_curr - ticks_prev) : (0xFFFFFFFF - ticks_prev + 1 + ticks_curr);
		time_elapsed_sec/= configTICK_RATE_HZ;
		ticks_prev= ticks_curr;

		currtime= time(0);
		localtime_r(&currtime, &currtime_tm);
		strftime(conbuffer, 32, "%Y-%m-%d %H:%M:%S", &currtime_tm);
		printf("<TIME> %s\n", conbuffer);




		heater_entry= heater_list.first_entry;
		while (heater_entry)
			{
			heater= (heater_s *)heater_entry->data;

			thermometer= heater->thermometer;
			heater_ctrl_handler= thermometer ? heater->heater_ctrl_handler : NULL;




			switch (main_settings.global_mode)
				{

				case GLOBAL_MODE_IDLE:
					{


					// wy³¹czenie timera
					heater->max_throttle_timeout_active= false;


					break;
					} // GLOBAL_MODE_IDLE


				case GLOBAL_MODE_HEATING:
					{





					break;
					} // GLOBAL_MODE_HEATING


				} // switch (main_settings.global_mode)




			if (thermometer)
				{

				xSemaphoreTake(thermometer_sem, 0);



				if ((queue_dta == 0x01) || !thermometer->temp_vaild)
					{
					// zerowanie, wy³¹cz grzanie

					xSemaphoreGive(thermometer_sem);

					// jesli zmiana
					//pwm_channel_set

					
					// wy³¹czenie timera
					heater->max_throttle_timeout_active= false;
					heater->max_throttle_timeout= 0;



					}
				else
					{

					heater_temp= (k_long)thermometer->temp_value;


					xSemaphoreGive(thermometer_sem);

					heater_temp*= 1000;
					heater_temp>>= 4;

					heater->temp_current= heater_temp;

					if (heater->temp_offset != 0)
						heater_temp+= heater->temp_offset;

					temp_uchyb= heater->temp_zadana - heater_temp;

					if (heater->PID_data.uchyb_prev == 0x80000000)
						heater->PID_data.uchyb_prev= temp_uchyb;

					ctrl_param= temp_pid_process(&heater->PID_data, temp_uchyb);

					duty_ratio= (uint16_t)round((((float)ctrl_param / (float)PID_REG_MAX) * pwm_duty_ratio_max));



					if (duty_ratio > pwm_duty_ratio_max)
						{
						duty_ratio= pwm_duty_ratio_max;
						heater->pid_state= HTR_PID_STATE_HEATING_MAX;
						}
					else
					if (duty_ratio < PWM_DUTY_RATIO_CUTOFF)
						{
						printf("<HTR> dev[%02d]: CLOSED\n", heater->indx);
						duty_ratio= 0;
						heater->pid_state= HTR_PID_STATE_IDLE;
						}
					else
						heater->pid_state= HTR_PID_STATE_HEATING;

					if ((duty_ratio > 0) && (temp_uchyb <= 0))
						{
						printf("<HTR> dev[%02d]: CLOSED, OVERHEATING\n", heater->indx);
						duty_ratio= 0;
						heater->pid_state= HTR_PID_STATE_OVERHEATING;
						}


					// zabezpieczenie przed ci¹g³ym za³¹czeniem grzania na full, w przypadku, gdy np. wy³aczony jest piec
					// jesli czas otwarcia na full (np. > 800) trwa powyzej x sekund, to za³¹czyc pwm na podtrzymanie

					if ((heater->max_throttle_timeout_active == true) && (duty_ratio < 800))
						{
						// wy³¹czenie timera
						heater->max_throttle_timeout_active= false;
						heater->max_throttle_timeout= 0;
						printf("<HTR> dev[%02d]: full throttle timer stop\n", heater->indx);
						}

					if (heater->max_throttle_timeout_active == true)
						{
						if (heater->max_throttle_timeout != 0)
							{
							if (heater->max_throttle_timeout >= time_elapsed_sec)
								heater->max_throttle_timeout-= time_elapsed_sec;
							else
								heater->max_throttle_timeout= 0;
							}

						if (heater->max_throttle_timeout == 0)
							{
							printf("<HTR> dev[%02d]: full throttle limiter\n", heater->indx);
							duty_ratio= PWM_WARM_CONST_FACTOR;
							heater->pid_state= HTR_PID_STATE_FULLTHR_LIMITER;
							}
						else
							printf("<HTR> dev[%02d]: full throttle timer: %03d\n", heater->indx, heater->max_throttle_timeout);
						}

					if ((duty_ratio > 900) && (heater->max_throttle_timeout_active == false))
						{
						// za³¹czenie timera
						heater->max_throttle_timeout= PWM_MAX_THROTTLE_TIMEOUT;
						heater->max_throttle_timeout_active= true;

						printf("<HTR> dev[%02d]: full throttle timer start\n", heater->indx);
						}


					printf("<HTR> dev[%02d]: therm[%02X] tset=%2.3f toff=%2.3f tdiff=%2.3f\n", heater->indx, heater->thermometer->indx, (float)heater->temp_zadana/1000, (float)heater->temp_offset/1000, (float)temp_uchyb/1000);
					printf("<HTR> dev[%02d]: PID: %4d %4d %4d \n", heater->indx, heater->PID_data.P/1000, heater->PID_data.I/1000, heater->PID_data.D/1000);
					printf("<HTR> dev[%02d]: PID param: %05d  duty_ratio: %04d\n", heater->indx, ctrl_param, duty_ratio);


					if (heater_ctrl_handler->duty_ratio != duty_ratio)
						{
						printf("<HTR> dev[%02d]: PWM duty ratio set to:  %04d\n", heater->indx, duty_ratio);
						pwm_channel_set(heater_ctrl_handler, duty_ratio); // 0 - pwm_duty_ratio_max
						}







					}


				} // if (thermometer)
			else
				{
				// brak zdefiniowanego termometru dla grzejnika



				}



			heater_entry= heater_entry->next;

			} //while (heater_entry)


		if (queue_dta == 0x00)
			tempctrl_write2logfile();



		} // while (1)

	}

//-----------------------------------------------------------------------------

k_long temp_pid_process(PID_data_s *PID_data, k_long uchyb)
	{
	k_long result;
    
	PID_data->P= Kp * uchyb;
	PID_data->I= Ki * (uchyb + PID_data->uchyb_prev) / 2 + PID_data->I_prev;
	PID_data->D= Kd * (uchyb - PID_data->uchyb_prev);
	
	PID_data->uchyb_prev= uchyb;
	result= PID_data->P + PID_data->I + PID_data->D;
	result/= 1000;

	if (result < PID_REG_MIN)
        result= PID_REG_MIN;
	else
	if (result > PID_REG_MAX)
		result= PID_REG_MAX;
	else
		PID_data->I_prev= PID_data->I;

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

		if ((SystemCoreClock / PWM_TIMER_FREQ) >= 0x10000)
			printf("<ERR> %s: TIM_Prescaler overrange !\n", __FUNCTION__);

		if (((int)heater_ctrl_handler->period * PWM_TIMER_FREQ) >= 0x10000)
			printf("<ERR> %s: TIM_Period overrange !\n", __FUNCTION__);

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

void tempctrl_write2logfile()
	{
	FRESULT res;
	UINT br;
	wdlist_entry_s *heater_entry= NULL;
	heater_s *heater;

	static u8 buffer_tmp[64];
	static u8 logfile_buffer[LOGFILE_BUFFER_LENGTH];


	if (!logfile_open_f)
		{

		if (SD_Detect() == SD_PRESENT)
			{
			res= f_mount(0, &fs);

			sprintf(logfile_buffer, "0:/");
			strftime(buffer_tmp, 16, "%Y%m%d", &currtime_tm);
			strcat(logfile_buffer, buffer_tmp);
			strcat(logfile_buffer, ".csv");
			res= f_open(&fsrc, logfile_buffer, FA_OPEN_ALWAYS | FA_WRITE);
			printf("f_open: %s\n", (res == FR_OK) ? "success" : "fail");

			if (res == FR_OK)
				{
				res= f_lseek(&fsrc, f_size(&fsrc));
				printf("f_lseek: %s\n", (res == FR_OK) ? "success" : "fail");

				logfile_open_f= true;
				}
			}

		} // if (!logfile_open_f)


	if (!logfile_open_f)
		return ;

	if (SD_Detect() != SD_PRESENT)
		{
		printf("SD CARD: not present\n");

		logfile_open_f= false;
		f_close(&fsrc);

		f_mount(0, NULL); // umount

		return ;
		}

	strftime(logfile_buffer, LOGFILE_BUFFER_LENGTH, "%H:%M:%S", &currtime_tm);

	sprintf(buffer_tmp, ";%d", main_settings.global_mode);
	strcat(logfile_buffer, buffer_tmp);

	heater_entry= heater_list.first_entry;
	while (heater_entry)
		{
		heater= (heater_s *)heater_entry->data;

		sprintf(buffer_tmp, ";%d;%d;%d;%d;%2.3f;%2.3f;%2.3f;%d;%d;%d;%d",
			heater->state,
			heater->PID_data.P/1000,
			heater->PID_data.I/1000,
			heater->PID_data.D/1000,
			(float)heater->temp_zadana/1000,
			(float)heater->temp_offset/1000,
			(float)heater->temp_current/1000,
			heater->pid_state,
			heater->heater_ctrl_handler->duty_ratio,
			heater->max_throttle_timeout_active,
			heater->max_throttle_timeout
			);

		strcat(logfile_buffer, buffer_tmp);

		heater_entry= heater_entry->next;
		}

//	printf("%s\n", logfile_buffer);
	strcat(logfile_buffer, "\r\n");

	res= f_write(&fsrc, logfile_buffer, strlen(logfile_buffer), &br);
	f_sync(&fsrc);

	}

//-----------------------------------------------------------------------------


/*

heater <number> pwm <value>

*/

void cmdline_heater_param_set(unsigned char *param)
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
				else
				if (!strcmp(param, "temp"))
					cmd= 0x02;

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

			case 0x02:	// temperatura zadana
				{
				if (nparam == 1)
					{
					printf("<HTR> dev[%02d]: temp set to %d\n", heater->indx, paramsval[0]);

					heater->temp_zadana= paramsval[0];

					heater->PID_data.uchyb_prev= 0x80000000; // magic value
					heater->PID_data.I_prev= 0;

					}
				break;
				}

			} // switch (cmd)


		} // if (heater)

	}

//-----------------------------------------------------------------------------

void cmdline_heater_pwm_duty_ratio_max_set(unsigned char *param)
	{
	k_ulong paramsval;

	if (!param)
		return;

	paramsval= atoi(param);

	if (paramsval < 0)
		paramsval= 0;
	else
	if (paramsval > 1000)
		paramsval= 1000;

	pwm_duty_ratio_max= paramsval;

	printf("<PWM> max duty ratio set to %d\n", pwm_duty_ratio_max);

	}

//-----------------------------------------------------------------------------

void cmdline_pid_set(unsigned char *param)
	{
	unsigned char *tokptr;
	k_uchar indx= 0;

	k_uchar cmd= 0xFF;
	k_uchar nparam= 0;

	const k_uchar max_nparam= 3;
	k_ulong paramsval[max_nparam];

	while (param)
		{

		switch (indx)
			{

			case 0:
				{

				if (!strcmp(param, "p"))
					cmd= 0x01;
				else
				if (!strcmp(param, "i"))
					cmd= 0x02;
				else
				if (!strcmp(param, "d"))
					cmd= 0x03;

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



	switch (cmd)
		{

		case 0x01:	// PID P
			{

			if (nparam == 1)
				{
				Kp= paramsval[0];
				printf("<PID> P set to: %d\n", Kp);
				}

			break;
			}

		case 0x02:	// PID I
			{

			if (nparam == 1)
				{
				Ki= paramsval[0];
				printf("<PID> I set to: %d\n", Ki);
				}

			break;
			}

		case 0x03:	// PID D
			{

			if (nparam == 1)
				{
				Kd= paramsval[0];
				printf("<PID> D set to: %d\n", Kd);
				}

			break;
			}

		} // switch (cmd)



	}


//-----------------------------------------------------------------------------

