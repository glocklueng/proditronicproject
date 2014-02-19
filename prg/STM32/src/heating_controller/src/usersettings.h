/*
 * usersettings.h
 *
 *  Created on: 08-02-2014
 *      Author: Tomek
 */

#ifndef USERSETTINGS_H_
#define USERSETTINGS_H_

#include <time.h>


#include <ktypes.h>



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#define USERSETTINGS_MAX			10


#define HEATER_PRECONFIG_TEMP_MAX	6

#define HEATER_TEMP_NIGHT			0
#define HEATER_TEMP_WORK			1
#define HEATER_TEMP_DAY_LIFE		2
#define HEATER_TEMP_DAY_COMFORT		3
#define HEATER_TEMP_USER1			4
#define HEATER_TEMP_USER2			5






//-----------------------------------------------------------------------------

typedef struct
	{
	k_uchar week_days;		// bit7 - enable, bit6 - sat, bit5 - fri, .. , bit0 - sun
	k_uchar heaters;

	k_ushort dummy1;		// wynika z organizacji pamiêci w obszarze BKP

	k_uchar begin_time;		// liczba minut od poczatku dnia podzielona przez 10 (zakres 0 - 143)
	k_uchar pred_temp_indx;

	k_ushort dummy2;

	} user_settings_s;


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

int usersettings_get(k_uchar heater_index, struct tm *current_time, k_long *temperature);




//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#endif /* USERSETTINGS_H_ */
