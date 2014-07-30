/*-----------------------------------------------------------------------------/
 * modbus_common.h
 *
 *  Created on: 26-06-2014
 *      Author: Tomasz Przybysz
/-----------------------------------------------------------------------------*/

#ifndef MODBUS_COMMON_H_
#define MODBUS_COMMON_H_

#include <stdbool.h>

#include "stm32f10x.h"
#include "platform_config.h"


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#define MODBUS_STATE_IDLE			0x00
#define MODBUS_STATE_CTRL_REQ		0x10
#define MODBUS_STATE_CTRL_PENDING	0x11

#define MODBUS_STATE_POLLING		0x20


//-----------------------------------------------------------------------------

typedef struct
	{


	} modbus_ctrl_param_s;


typedef struct
	{

	unsigned char chno;

	bool connected;
	unsigned char state;

	

	xSemaphoreHandle ctrl_sem;

	modbus_ctrl_param_s ctrl_param_waiting;


	} modbus_mst_channel_s;


typedef struct
	{

	uint16_t modbus_addr;
	void *dataptr;



	} modbus_data_s;

//-----------------------------------------------------------------------------




//-----------------------------------------------------------------------------

#endif // MODBUS_COMMON_H_

