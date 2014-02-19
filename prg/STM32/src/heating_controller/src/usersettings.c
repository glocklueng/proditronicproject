/*
 * usersettings.c
 *
 *  Created on: 08-02-2014
 *      Author: Tomek
 */

//-----------------------------------------------------------------------------


#include "usersettings.h"


//-----------------------------------------------------------------------------


extern user_settings_s *user_settings_tab;
extern k_uchar *temp_preconfig_tab;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

int usersettings_get(k_uchar heater_index, struct tm *current_time, k_long *temperature)
	{
	k_long x;

	k_long systime;
	k_uchar today;
	k_uchar yesterday;

	k_long indx= -1;
	k_long time_diff;
	k_long time_diff_min= 0x7FFFFFF;
	k_long begin_time;


	if (!current_time || !temperature)
		return -1;


	today= (1 << current_time->tm_wday); // dzien aktualny
	yesterday= (current_time->tm_wday == 0) ? 0x40 : (1 << (current_time->tm_wday - 1)); // dzien poprzedni
	systime= current_time->tm_hour * 60 + current_time->tm_min; // czas aktualny w minutach od poczatku dnia

	for (x=0;x<HEATER_PRECONFIG_TEMP_MAX;x++)
		{
		k_uchar *addr= temp_preconfig_tab + (int)((x >> 1) * 4 + (x & 1));


		printf("addr: %08X %d\n", addr, *addr);
		}



	for (x=0;x<USERSETTINGS_MAX;x++)
		{
		user_settings_s *user_settings= &user_settings_tab[x];

		begin_time= user_settings->begin_time * 10;


		printf("user[%02d] %08X   %d %02X %02X %02X   %d\n", x, user_settings,
				user_settings->week_days >> 7,
				user_settings->week_days & 0x7F,
				user_settings->heaters,
				user_settings->pred_temp_indx,
				begin_time);


		if (user_settings->week_days & 0x80) // enabled ?
			{

			if (user_settings->heaters & (1 << heater_index)) // sprawdz zgodnosc indeksu grzejnika
				{

				if (user_settings->week_days & today)
					{
					time_diff= systime - begin_time;

					if ((systime >= begin_time) && (time_diff < time_diff_min))
						{
						time_diff_min= time_diff;
						indx= x;
						}

					} // today

				if (user_settings->week_days & yesterday)
					{
					time_diff= 1440 - begin_time + systime;

					if ((systime < begin_time) && (time_diff < time_diff_min))
						{
						time_diff_min= time_diff;
						indx= x;
						}

					} // yesterday

				} // if (user_settings->heaters & (1 << heater_index))

			} // enabled

		} // for (x=0;x<USERSETTINGS_MAX;x++)


	if (indx != -1)
		{
		k_uchar pred_indx;
		k_uchar *pred_addr;

		pred_indx= user_settings_tab[indx].pred_temp_indx;
		pred_addr= temp_preconfig_tab + (int)((pred_indx >> 1) * 4 + (pred_indx & 1));
		*temperature= (k_long)(*(k_uchar *)pred_addr) * 500;
		}
	else
		*temperature= 0;

	printf("index: %d %d\n", indx, *temperature);

	return (indx != -1) ? 0 : -1;
	}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------


