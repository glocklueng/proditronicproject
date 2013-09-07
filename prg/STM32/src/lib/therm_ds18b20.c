/*------------------------------------------------------------------------------/
 * therm_ds18b20.c
 *
 *  Created on: 2010-03-08
 *      Author: przybysz.tomasz@gmail.com
 *
/------------------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>

#include "therm_ds18b20.h"

//------------------------------------------------------------------------------

k_uchar therm_ds18b20_conversion_start(onewire_handler_s *onewire_handler, k_uchar *dev_id)
    {
	k_uchar result;
	int x;

	if (!onewire_handler)
		return (k_uchar)-1;

	result= onewire_bus_reset(onewire_handler);

	if (result != 0x00)
		return (k_uchar)-1;

	if (dev_id == NULL)
		onewire_write_byte(onewire_handler, ONE_WIRE_CMD_SKIP_ROM);				// 0xCC
	else
		{
		onewire_write_byte(onewire_handler, ONE_WIRE_CMD_MATCH_ROM);			// 0x55

		for (x=0;x<ONEWIRE_DEV_ID_LENGTH;x++)
			onewire_write_byte(onewire_handler, dev_id[x]); 					// dev_id
		}

	if (onewire_handler->strong_pull_up_enable)
		onewire_write_byte_pullup(onewire_handler, ONE_WIRE_CMD_CONV_TEMP);		// 0x44
	else
		onewire_write_byte(onewire_handler, ONE_WIRE_CMD_CONV_TEMP);			// 0x44

    return result;
    }

//------------------------------------------------------------------------------

k_uchar therm_ds18b20_temperature_read(onewire_handler_s *onewire_handler, k_uchar *dev_id, k_ushort *temp)
	{
	k_uchar result;
	int x;
	k_uchar crc8= 0x00;
	k_uchar correct= 0x01;

	if (!onewire_handler || !temp)
		return (k_uchar)-1;

	result= onewire_bus_reset(onewire_handler);

	if (result != 0x00)
		return (k_uchar)-1;

	if (dev_id == NULL)
		onewire_write_byte(onewire_handler, ONE_WIRE_CMD_SKIP_ROM);		// 0xCC
	else
		{
		onewire_write_byte(onewire_handler, ONE_WIRE_CMD_MATCH_ROM);	// 0x55

		for (x=0;x<ONEWIRE_DEV_ID_LENGTH;x++)
			onewire_write_byte(onewire_handler, dev_id[x]); 			// dev_id
		}

	onewire_write_byte(onewire_handler, ONE_WIRE_CMD_READ_SCRATCHPAD);	// 0xBE

	for (x=0;x<9;x++)
		{
		result= onewire_read_byte(onewire_handler);
		onewire_crc8(&crc8, result);

		if (x < 2)
			*((k_uchar *)temp + x)= result;
		else
		if ((x == 5) && (result != 0xFF))
			correct= 0x00;
		else
		if ((x == 7) && (result != 0x10))
			correct= 0x00;

		} // for

	return ((crc8 == 0x00) && correct) ? 0x00 : (k_uchar)-1;
	}

//------------------------------------------------------------------------------

k_uchar therm_ds18b20_id_read(onewire_handler_s *onewire_handler, k_uchar *dev_id)
	{
	k_uchar result;
	int x;
	k_uchar crc8= 0x00;
	k_uchar therm_family= 0x00;

	if (!onewire_handler || !dev_id)
		return (k_uchar)-1;

	result= onewire_bus_reset(onewire_handler);

	if (result != 0x00)
		return (k_uchar)-1;

	onewire_write_byte(onewire_handler, ONE_WIRE_CMD_READ_ROM);

	for (x=0;x<8;x++)
		{
		result= onewire_read_byte(onewire_handler);
		onewire_crc8(&crc8, result);

		if ((x == 0) && (result== DS18B20_FAMILY_CODE))
			therm_family= 0x01;
		else
		if ((x > 0) && (x < 7))
			dev_id[x-1]= result;
		}

	return ((crc8 == 0x00) && therm_family) ? 0x00 : (k_uchar)-1;
	}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------