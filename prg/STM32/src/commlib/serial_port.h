/*
 * serial_port.h
 *
 *  Created on: 01-02-2013
 *      Author: Tomek
 */

#ifndef SERIAL_PORT_H_
#define SERIAL_PORT_H_


#include "stm32f10x.h"
#include "platform_config.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


int serial_port_init(unsigned char port_no, USART_InitTypeDef *USART_InitStructure);
int serial_port_write(unsigned char port_no, char *ptr, int len);
int serial_port_read(unsigned char port_no, char *ptr, int len);
int serial_port_rx_timeout_set(unsigned char port_no, int timeout);


void USART1_Tx_DMA_IRQ_Handler(void);
void USART1_Rx_IRQ_Handler(void);




//-----------------------------------------------------------------------------

#endif /* SERIAL_PORT_H_ */
