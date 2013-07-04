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

#define THERMOMETER_READ_PERIOD		10 // sec

//-----------------------------------------------------------------------------

typedef struct
	{
	onewire_handler_s *onewire_handler;
	k_uchar dev_id[ONEWIRE_DEV_ID_LENGTH];
	



	} thermometer_s;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void prvOneWireTask(void *pvParameters);



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#endif // ONEWIRE_THR_H_
