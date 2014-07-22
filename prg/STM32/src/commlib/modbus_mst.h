/*-----------------------------------------------------------------------------/
 * modbus_mst.c
 *
 *  Created on: 26-06-2014
 *      Author: Tomasz Przybysz
/-----------------------------------------------------------------------------*/

#ifndef MODBUS_MST_H_
#define MODBUS_MST_H_


#include "stm32f10x.h"
#include "platform_config.h"

#include "modbus_common.h"


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------





//-----------------------------------------------------------------------------

int modbus_mst_run(modbus_channel_s *modbus_channel);
void modbus_ctrl(modbus_channel_s *channel, modbus_ctrl_param_s *ctrl_param);



//-----------------------------------------------------------------------------

#endif // MODBUS_MST_H_

