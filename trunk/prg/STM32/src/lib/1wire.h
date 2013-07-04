/*------------------------------------------------------------------------------/
 * 1wire.c
 *
 *  Created on: 2010-01-23
 *      Author: przybysz.tomasz@gmail.com
 *
/------------------------------------------------------------------------------*/
#ifndef _1WIRE_H_
#define _1WIRE_H_

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#if defined (__STM32__)
	#include "stm32f10x.h"
#endif


#include "ktypes.h"

//------------------------------------------------------------------------------

#define ONEWIRE_DEV_ID_LENGTH	8

//------------------------------------------------------------------------------


typedef struct
	{
	uint32_t peripheral_addr;
    uint16_t data_pin;

	uint16_t strong_pull_up_pin;
	k_uchar strong_pull_up_enable;
    
	} onewire_handler_s;


//------------------------------------------------------------------------------

int onewire_bus_init(onewire_handler_s *onewire_handler);

k_uchar onewire_bus_reset(onewire_handler_s *onewire_handler);
void onewire_strong_pullup_enable(onewire_handler_s *onewire_handler, k_uchar state);

void onewire_write_bit(onewire_handler_s *onewire_handler, k_uchar bit);
k_uchar onewire_read_bit(onewire_handler_s *onewire_handler);

void onewire_write_byte(onewire_handler_s *onewire_handler, k_uchar byte);
void onewire_write_byte_pullup(onewire_handler_s *onewire_handler, k_uchar byte); // po wys³aniu bajtu za³¹cza strong pullup

k_uchar onewire_read_byte(onewire_handler_s *onewire_handler);

void onewire_crc8(unsigned char *crc8, unsigned char value);

//------------------------------------------------------------------------------

#endif /* _1WIRE_H_ */
