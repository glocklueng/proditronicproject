/*
 * 1wire_thr.c
 *
 *  Created on: 25-06-2013
 *      Author: Tomek
 */


#if defined (__STM32__)
	#include "stm32f10x.h"
#endif


#include "ksystem.h"
#include "1wire.h"


#include "1wire_thr.h"

//-----------------------------------------------------------------------------





//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void prvOneWireTask(void *pvParameters)
	{

	onewire_handler_s onewire_chn1;

	msleep(1000);


	onewire_chn1.peripheral_addr= (uint32_t)GPIOB;
	onewire_chn1.data_pin= GPIO_Pin_1;
	onewire_chn1.strong_pull_up_enable= 0x00;
	onewire_bus_init(&onewire_chn1);

	msleep(3000);


	while (1)
		{


		//msleep(3000);

		//onewire_bus_reset(&onewire_chn1);

		msleep(2000);
		onewire_write_bit(&onewire_chn1, 0x00);

		//msleep(2000);
		//onewire_write_bit(&onewire_chn1, 0x01);




		} // while (1)



	}




//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

