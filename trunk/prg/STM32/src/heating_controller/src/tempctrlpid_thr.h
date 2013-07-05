#ifndef TEMPCTLRPID_THR_H_
#define TEMPCTLRPID_THR_H_

//-----------------------------------------------------------------------------

#include "1wire_thr.h"




//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------

typedef struct
	{
	k_long uchyb_prev;
	k_long I_prev;

	} PID_data_s;


typedef struct
	{

	thermometer_s *thermometer;

	k_long temp_zadana;
	k_long temp_offset; // korekta temperatury

	PID_data_s PID_data;



	} heater_s;



//-----------------------------------------------------------------------------


void prvTempCtrlPIDTask(void *pvParameters);



//-----------------------------------------------------------------------------

#endif // TEMPCTLRPID_THR_H_
