#ifndef CONFIG_H_
#define CONFIG_H_

//-----------------------------------------------------------------------------

#include "1wire.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#define ONEWIRE_NBUS		4
#define HEATER_NUMBER		4


#define GLOBAL_MODE_IDLE		0x00
#define GLOBAL_MODE_HEATING		0x01










#define GPIOA_CLK_ENABLE_BIT	(1 << 0)
#define GPIOB_CLK_ENABLE_BIT	(1 << 1)
#define GPIOC_CLK_ENABLE_BIT	(1 << 2)
#define GPIOD_CLK_ENABLE_BIT	(1 << 3)


//-----------------------------------------------------------------------------

typedef struct
	{

	k_uchar dev_id[ONEWIRE_DEV_ID_LENGTH];
	k_uchar chn_no;

	} thermometer_cfg_s;


typedef struct
	{
	k_uchar thermometer_no;


	} heater_cfg_s;


typedef struct
	{

	k_uchar global_mode;



	} main_settings_s;


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#endif // CONFIG_H_

