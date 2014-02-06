/*
 * 1wire_thr.h
 *
 *  Created on: 25-06-2013
 *      Author: Tomek
 */

#ifndef ONEWIRE_THR_H_
#define ONEWIRE_THR_H_

//-----------------------------------------------------------------------------

#include <stdbool.h>

#include "ktypes.h"
#include "1wire.h"


//-----------------------------------------------------------------------------

#define THERMOMETERS_MAX			16
#define THERMOMETERS_PER_CHANNEL	4


#define THERMOMETER_READ_PERIOD			15 // sec
#define THERMOMETER_READ_ERROR_MAX		3

#define THERMOMETER_READ_CNTR			3

// error codes

// 0x01 	- niezdefiniowany kana³ 1-wire dla termometru
// 0x02 	- b³¹d w czasie wykonywania funkcji therm_ds18b20_conversion_start
// 0x03 	- b³¹d w czasie wykonywania funkcji therm_ds18b20_temperature_read, odebrano 0000......
// 0x04 	- b³¹d w czasie wykonywania funkcji therm_ds18b20_temperature_read, odebrano 1111......
// 0x05 	- b³¹d w czasie wykonywania funkcji therm_ds18b20_temperature_read, blad CRC
// 0x06 	- b³¹d w czasie wykonywania funkcji therm_ds18b20_temperature_read, blad formatu
// 0x07 	- brak odpowiedzi na reset
// 0x19 	- b³¹d w czasie wykonywania funkcji therm_ds18b20_temperature_read, inny blad



// 0x11		- b³êdna wartosc odczytu tempertury, problem z konwersj¹, power on reset value, wartosc odczytu 0x0550
// 0x12		- b³êdna wartosc odczytu tempertury, problem ze strong pullup, wartosc odczytu 0x07FF
// 0x13		- b³êdna wartosc odczytu tempertury, wartosc spoza zakresu


//-----------------------------------------------------------------------------

typedef struct
	{
	k_uchar indx;
	onewire_handler_s *onewire_handler;
	k_uchar dev_id[6];
	
	k_short temp_value;
	k_short temp_offset; // offset temperatury: +/- 1000 => +/- 1.0 C
	k_short temp_valuex[THERMOMETER_READ_CNTR];
	bool temp_vaild;

	k_uchar temp_read_error_cntr;
	k_uchar error_code;
	k_uchar temp_read_0x0550_cntr;

	k_short temp_debug_value;
	bool temp_debug_f;

	} thermometer_s;



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void prvOneWireTask(void *pvParameters);

void cmdline_thermometer_temp_set(unsigned char *param);


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#endif // ONEWIRE_THR_H_
