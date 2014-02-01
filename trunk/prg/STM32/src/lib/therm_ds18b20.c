/*------------------------------------------------------------------------------/
 * therm_ds18b20.c
 *
 *  Created on: 2010-03-08
 *      Author: przybysz.tomasz@gmail.com
 *
/------------------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "therm_ds18b20.h"

//------------------------------------------------------------------------------

//#define ONEWIRE_DEBUG

//------------------------------------------------------------------------------

k_uchar therm_ds18b20_conversion_start(onewire_handler_s *onewire_handler, k_uchar *dev_id)
    {
	k_uchar result;
	int x;
	k_uchar crc8;

	if (!onewire_handler)
		return (k_uchar)-1;

	result= onewire_bus_reset(onewire_handler);

	if (result != 0x00)
		return 0xF4;

	if (dev_id == NULL)
		onewire_write_byte(onewire_handler, ONE_WIRE_CMD_SKIP_ROM);				// 0xCC
	else
		{
		crc8= 0x00;

		onewire_write_byte(onewire_handler, ONE_WIRE_CMD_MATCH_ROM);			// 0x55

		onewire_write_byte(onewire_handler, DS18B20_FAMILY_CODE);				// 0x28
		onewire_crc8(&crc8, DS18B20_FAMILY_CODE);

		for (x=0;x<ONEWIRE_DEV_ID_LENGTH;x++)
			{
			onewire_write_byte(onewire_handler, dev_id[x]); 					// dev_id
			onewire_crc8(&crc8, dev_id[x]);
			}

		onewire_write_byte(onewire_handler, crc8);
		}

	if (onewire_handler->strong_pull_up_enable)
		onewire_write_byte_pullup(onewire_handler, ONE_WIRE_CMD_CONV_TEMP);		// 0x44
	else
		onewire_write_byte(onewire_handler, ONE_WIRE_CMD_CONV_TEMP);			// 0x44

    return result;
    }

//------------------------------------------------------------------------------

k_uchar therm_ds18b20_temperature_read(onewire_handler_s *onewire_handler, k_uchar *dev_id, k_short *temp)
	{
	k_uchar result;
	int x;
	k_uchar crc8;
	k_uchar correct= 0x01;
	bool only0s= true;
	bool only1s= true;

	if (!onewire_handler || !temp)
		return (k_uchar)-1;

	result= onewire_bus_reset(onewire_handler);

	if (result != 0x00)
		return 0xF4;

	if (dev_id == NULL)
		onewire_write_byte(onewire_handler, ONE_WIRE_CMD_SKIP_ROM);		// 0xCC
	else
		{
		crc8= 0x00;

		onewire_write_byte(onewire_handler, ONE_WIRE_CMD_MATCH_ROM);	// 0x55

		onewire_write_byte(onewire_handler, DS18B20_FAMILY_CODE);		// 0x28
		onewire_crc8(&crc8, DS18B20_FAMILY_CODE);

		for (x=0;x<ONEWIRE_DEV_ID_LENGTH;x++)
			{
			onewire_write_byte(onewire_handler, dev_id[x]); 			// dev_id
			onewire_crc8(&crc8, dev_id[x]);
			}

		onewire_write_byte(onewire_handler, crc8);
		}

	crc8= 0x00;

	onewire_write_byte(onewire_handler, ONE_WIRE_CMD_READ_SCRATCHPAD);	// 0xBE

#if defined(ONEWIRE_DEBUG)
	printf("<DBG> 1WIRE: INT:%04X:", onewire_handler->data_pin);
#endif

	for (x=0;x<9;x++)
		{
		result= onewire_read_byte(onewire_handler);
		onewire_crc8(&crc8, result);

		if (result != 0x00)
			only0s= false;

		if (result != 0xFF)
			only1s= false;

#if defined(ONEWIRE_DEBUG)
		printf(" %02X", result);
#endif

		if (x < 2)
			*((k_uchar *)temp + x)= result;
		else
		if ((x == 5) && (result != 0xFF))
			correct= 0x00;
		else
		if ((x == 7) && (result != 0x10))
			correct= 0x00;

		} // for

#if defined(ONEWIRE_DEBUG)
	printf("  CRC: %s\n", ((crc8 == 0x00) ? "OK" : "BAD"));
#endif


	if ((crc8 == 0x00) && correct)
		return 0x00;

	if (only0s)
		return 0xF0;

	if (only1s)
		return 0xF1;

	if (crc8 != 0x00)
		return 0xF2;

	if (!correct)
		return 0xF3;

	return (k_uchar)-1;
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

#if defined(ONEWIRE_DEBUG)
	printf("<DBG> 1WIRE: INT:%04X: RST: %02X\n", onewire_handler->data_pin, result);
#endif

	if (result != 0x00)
		return 0xF4;

	onewire_write_byte(onewire_handler, ONE_WIRE_CMD_READ_ROM);

#if defined(ONEWIRE_DEBUG)
	printf("<DBG> 1WIRE: INT:%04X:", onewire_handler->data_pin);
#endif


	for (x=0;x<8;x++)
		{
		result= onewire_read_byte(onewire_handler);
		onewire_crc8(&crc8, result);

#if defined(ONEWIRE_DEBUG)
		printf(" %02X", result);
#endif

		if ((x == 0) && (result== DS18B20_FAMILY_CODE))
			therm_family= 0x01;
		else
		if ((x > 0) && (x < 7))
			dev_id[6-x]= result;
		}

#if defined(ONEWIRE_DEBUG)
	printf("  CRC: %s\n", ((crc8 == 0x00) ? "OK" : "BAD"));
#endif

	return ((crc8 == 0x00) && therm_family) ? 0x00 : (k_uchar)-1;
	}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
