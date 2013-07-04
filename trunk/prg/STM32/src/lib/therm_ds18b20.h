/*------------------------------------------------------------------------------/
 * therm_ds18b20.h
 *
 *  Created on: 2010-03-08
 *      Author: przybysz.tomasz@gmail.com
 *
/------------------------------------------------------------------------------*/

#ifndef THERM_DS18B20_H_
#define THERM_DS18B20_H_

//------------------------------------------------------------------------------

#include "ktypes.h"
#include "1wire.h"

//------------------------------------------------------------------------------

#define ONE_WIRE_CMD_SKIP_ROM			0xCC
#define ONE_WIRE_CMD_MATCH_ROM			0x55

#define ONE_WIRE_CMD_CONV_TEMP			0x44
#define ONE_WIRE_CMD_READ_SCRATCHPAD	0xBE

//------------------------------------------------------------------------------

k_uchar therm_ds18b20_conversion_start(onewire_handler_s *onewire_handler, k_uchar *dev_id);
k_uchar therm_ds18b20_temperature_read(onewire_handler_s *onewire_handler, k_uchar *dev_id, k_ushort *temp);

//------------------------------------------------------------------------------

#endif /* THERM_DS18B20_H_ */
