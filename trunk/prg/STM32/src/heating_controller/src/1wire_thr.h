/*
 * 1wire_thr.h
 *
 *  Created on: 25-06-2013
 *      Author: Tomek
 */

#ifndef ONEWIRE_THR_H_
#define ONEWIRE_THR_H_

//-----------------------------------------------------------------------------

#include "ktypes.h"
#include "1wire.h"


//-----------------------------------------------------------------------------

#define THERMOMETER_READ_PERIOD			10 // sec
#define THERMOMETER_READ_ERROR_MAX		3

//-----------------------------------------------------------------------------

typedef struct
	{
	onewire_handler_s *onewire_handler;
	k_uchar dev_id[ONEWIRE_DEV_ID_LENGTH];
	
	k_ushort temp_value;
	bool temp_vaild;
	k_uchar temp_read_error_cntr;


	} thermometer_s;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void prvOneWireTask(void *pvParameters);



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#endif // ONEWIRE_THR_H_
