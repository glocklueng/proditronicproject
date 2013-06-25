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


typedef struct
	{
	uint32_t peripheral_addr;
    uint16_t data_pin;

	uint16_t strong_pull_up_pin;
	k_uchar strong_pull_up_enable;
    
	} onewire_handler_s;


//------------------------------------------------------------------------------

int onewire_bus_init(onewire_handler_s *onewire_handler);
int onewire_bus_reset(onewire_handler_s *onewire_handler);

/*
void onewire_write_byte(unsigned char intf_mask, unsigned char byte);
void onewire_write_bit(unsigned char intf_mask, unsigned char bit);

void onewire_read_byte(unsigned char intf_mask, unsigned char *read_byte_tab);
unsigned char onewire_read_bit(unsigned char intf_mask);

void docrc8(unsigned char *crc8, unsigned char value);
*/
//------------------------------------------------------------------------------

#endif /* _1WIRE_H_ */
