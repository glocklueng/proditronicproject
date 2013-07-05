#include <stdlib.h>
#include <string.h>


#include <FreeRTOS.h>
#include "task.h"
#include "semphr.h"


#include "wdlist.h"
#include "ksystem.h"

#include "tempctrlpid_thr.h"

//-----------------------------------------------------------------------------

// PID wspó³czynniki
const k_long Kp= 10000;			// 10,000
const k_long Ki= 10;			// 0,010
const k_long Kd= 10000;			// 10,000

// nastawy min, max
const k_long reg_min= 0; 		// 0,000
const k_long reg_max= 10000		// 10,000



//-----------------------------------------------------------------------------


extern wdlist_s thermometer_list;
extern xSemaphoreHandle thermometer_sem;

extern wdlist_s heater_list;


//-----------------------------------------------------------------------------

k_long temp_pid_process(PID_data_s *PID_data, k_long uchyb);


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void prvTempCtrlPIDTask(void *pvParameters)
	{
	k_uchar queue_dta;

	wdlist_entry_s *heater_entry= NULL;
	heater_s *heater;
	thermometer_s *thermometer;
	k_long heater_temp;
	k_long temp_uchyb;
	k_long ctrl_param;


	heater_entry= heater_list.first_entry;
	while (heater_entry)
		{
		heater= (heater_s *)heater_entry->data;

		heater->PID_data.uchyb_prev= 0x80000000; // magic value
		heater->PID_data.I_prev= 0;

		heater_entry= heater_entry->next;
		}



	queue_dta= 0x01; // init
	xQueueSend(temp_pid_queue, &queue_dta);


	while (1)
		{

		xQueueReceive(temp_pid_queue, &queue_dta, portMAX_DELAY); // czekam na zakoñczenie cyklu odczytu temperatur

		heater_entry= heater_list.first_entry;
		while (heater_entry)
			{
			heater= (heater_s *)heater_entry->data;
			thermometer= heater->thermometer;

			if (thermometer)
				{

				xSemaphoreTake(thermometer_sem, 0);

				if ((queue_dta == 0x01) || !thermometer->temp_vaild)
					{
					// zerowanie, wy³¹cz grzanie

					xSemaphoreGive(thermometer_sem);



					
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

					if (heater->PID_data.uchyb_prev == 0x80000000)
						heater->PID_data.uchyb_prev= temp_uchyb;


					ctrl_param= temp_pid_process(&heater->PID_data, temp_uchyb);


					}


				} // if (thermometer)
            
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

	if (result < reg_min)
        result= reg_min;
	else
	if (result > reg_max)
		result= reg_max;
	else
		PID_data->I_prev= I;

	return result;
	}

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

