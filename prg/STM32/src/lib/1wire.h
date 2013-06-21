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

unsigned char onewire_bus_reset(unsigned char intf_mask);

void onewire_write_byte(unsigned char intf_mask, unsigned char byte);
void onewire_write_bit(unsigned char intf_mask, unsigned char bit);

void onewire_read_byte(unsigned char intf_mask, unsigned char *read_byte_tab);
unsigned char onewire_read_bit(unsigned char intf_mask);

void docrc8(unsigned char *crc8, unsigned char value);

//------------------------------------------------------------------------------

#endif /* _1WIRE_H_ */
