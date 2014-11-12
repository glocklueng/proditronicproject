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
#define MODBUS_STATE_POLLING		0x20


#define MODBUS_STATE_CTRL_IDLE		0x00
#define MODBUS_STATE_CTRL_REQ		0x01
#define MODBUS_STATE_CTRL_PENDING	0x11



#define MODBUS_DATA_TYPE_BIT		0x01
#define MODBUS_DATA_TYPE_INT16		0x02
#define MODBUS_DATA_TYPE_UINT16		0x03
#define MODBUS_DATA_TYPE_INT32		0x04
#define MODBUS_DATA_TYPE_UINT32		0x05
#define MODBUS_DATA_TYPE_FLOAT		0x06




//-----------------------------------------------------------------------------

typedef struct
	{


	} modbus_ctrl_param_s;

typedef struct
	{
	uint16_t modbus_addr;
	uint8_t data_type;
	uint8_t bit;

	void *dataptr;

	} modbus_data_s;


typedef struct
	{

	uint16_t *modbus_addr_poll;
	modbus_data_s **modbus_data_poll;

	unsigned char chno;



	bool connected;
	//unsigned char state;

	
	uint8_t ctrl_state;
	xSemaphoreHandle ctrl_sem;

	modbus_ctrl_param_s ctrl_param_waiting;


	} modbus_mst_channel_s;



//-----------------------------------------------------------------------------




//-----------------------------------------------------------------------------

#endif // MODBUS_COMMON_H_

