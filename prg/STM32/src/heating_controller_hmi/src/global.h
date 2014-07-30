#ifndef CONFIG_H_
#define CONFIG_H_

//-----------------------------------------------------------------------------

#include "1wire.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/*
#define ONEWIRE_NBUS		4
#define HEATER_NUMBER		4



#define GPIOA_CLK_ENABLE_BIT	(1 << 0)
#define GPIOB_CLK_ENABLE_BIT	(1 << 1)
#define GPIOC_CLK_ENABLE_BIT	(1 << 2)
#define GPIOD_CLK_ENABLE_BIT	(1 << 3)
*/

#define GLOBAL_MODE_IDLE		0x00
#define GLOBAL_MODE_HEATING		0x01


#define NMEASUREMENT	8



//-----------------------------------------------------------------------------

#define STM32_BKP_REG_BYTE_WR(regaddr,data) {*(uint16_t *)((uint32_t)regaddr & 0xFFFFFFFE)= ((uint32_t)regaddr & 1) ? ((*(uint16_t *)((uint32_t)regaddr & 0xFFFFFFFE) & 0x00FF) | (((uint16_t)data & 0x00FF) << 8)) : ((*(uint16_t *)((uint32_t)regaddr & 0xFFFFFFFE) & 0xFF00) | ((uint16_t)data & 0xFF));}

//-----------------------------------------------------------------------------

typedef struct
	{
	k_uchar dev_id[ONEWIRE_DEV_ID_LENGTH];

	k_uchar chn_no;
//	k_uchar dev_no;
//	k_uchar id_crc;

	} thermometer_cfg_s;


typedef struct
	{
	k_uchar thermometer_no;


	} heater_cfg_s;


typedef struct
	{

	k_uchar global_mode;



	} main_settings_s;


typedef struct
	{
	int32_t value;
	uint8_t flag;


	} measurement_s;


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#endif // CONFIG_H_

