#ifndef TEMPCTLRPID_THR_H_
#define TEMPCTLRPID_THR_H_

//-----------------------------------------------------------------------------

#if defined (__STM32__)
	#include "stm32f10x.h"
#endif

#include "ktypes.h"
#include "1wire_thr.h"




//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------





#define HTR_STATE_IDLE				0x00
#define HTR_STATE_RUNNING			0x01
#define HTR_STATE_					0x02
#define HTR_STATE_			0x03
#define HTR_STATE_			0x04


#define HTR_PID_STATE_IDLE				0x00
#define HTR_PID_STATE_HEATING			0x01
#define HTR_PID_STATE_HEATING_MAX		0x02
#define HTR_PID_STATE_OVERHEATING		0x03
#define HTR_PID_STATE_FULLTHR_LIMITER	0x04


//-----------------------------------------------------------------------------

typedef struct
	{
	k_long uchyb_prev;
	k_long I_prev;

	k_long P;
	k_long I;
	k_long D;


	} PID_data_s;


typedef struct
	{
	uint32_t peripheral_addr;
	uint16_t ctrl_pin;

	uint16_t period;
	uint16_t duty_ratio;

	uint8_t pwm_chn;

	} heater_ctrl_handler_s;


typedef struct
	{
	k_uchar indx;

	thermometer_s *thermometer;
	heater_ctrl_handler_s *heater_ctrl_handler;

	k_long temp_zadana;
	k_long temp_offset; // korekta temperatury
	k_long temp_current;

	k_uchar pid_state;

	PID_data_s PID_data;

	k_uchar state;

	bool max_throttle_timeout_active;
	k_ushort max_throttle_timeout;


	} heater_s;




//-----------------------------------------------------------------------------

k_long pwm_channel_set(heater_ctrl_handler_s *heater_ctrl_handler, uint16_t duty_ratio);
void prvTempCtrlPIDTask(void *pvParameters);


void cmdline_heater_param_set(unsigned char *param);
void cmdline_heater_pwm_duty_ratio_max_set(unsigned char *param);
void cmdline_pid_set(unsigned char *param);

//-----------------------------------------------------------------------------

#endif // TEMPCTLRPID_THR_H_
