/*------------------------------------------------------------------------------/
 * 1wire.c
 *
 *  Created on: 2010-01-23
 *      Author: przybysz.tomasz@gmail.com
 *
/------------------------------------------------------------------------------*/


#if defined (__STM32__)



#endif // __STM32__


#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include <FreeRTOS.h>
#include <task.h>

#include "emb_delay.h"
#include "1wire_cfg.h"
#include "1wire.h"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

unsigned char PROGMEM dscrc_table[] = {
        0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
      157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220,
       35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98,
      190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
       70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7,
      219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154,
      101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36,
      248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185,
      140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
       17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
      175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
       50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
      202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139,
       87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
      233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
      116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53};

//------------------------------------------------------------------------------

#define GPIOA_CLK_ENABLE_BIT	(1 << 0)
#define GPIOB_CLK_ENABLE_BIT	(1 << 1)
#define GPIOC_CLK_ENABLE_BIT	(1 << 2)


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

int onewire_bus_init(onewire_handler_s *onewire_handler)
	{

#if defined (__STM32__)

	GPIO_InitTypeDef GPIO_InitStructure;
	static unsigned char GPIOx_clk_enable= 0x00;

#endif // __STM32__


	if (!onewire_handler)
		return -1;


#if defined (__STM32__)

	switch (onewire_handler->peripheral_addr)
		{

		case GPIOB:
			{

			if (!(GPIOx_clk_enable & GPIOB_CLK_ENABLE_BIT))
				{
				// only once
                RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
				GPIOx_clk_enable|= GPIOB_CLK_ENABLE_BIT;
				}

			break;
			} // GPIOB


		default:
			{
			return -1;
			}

		} // switch (onewire_handler->peripheral_addr)


	// data bus
	GPIO_InitStructure.GPIO_Pin= onewire_handler->data_pin;
	GPIO_InitStructure.GPIO_Speed= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode= GPIO_Mode_Out_PP;
	GPIO_Init((GPIO_TypeDef *)onewire_handler->peripheral_addr, &GPIO_InitStructure);

	GPIO_SetBits((GPIO_TypeDef *)onewire_handler->peripheral_addr, onewire_handler->data_pin); // data bus: Hi


	// strong pull-up
	if (onewire_handler->strong_pull_up_enable)
		{
		GPIO_InitStructure.GPIO_Pin= onewire_handler->strong_pull_up_pin;
		GPIO_InitStructure.GPIO_Speed= GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode= GPIO_Mode_Out_PP;
		GPIO_Init((GPIO_TypeDef *)onewire_handler->peripheral_addr, &GPIO_InitStructure);

        GPIO_ResetBits((GPIO_TypeDef *)onewire_handler->peripheral_addr, onewire_handler->strong_pull_up_pin); // strong pull-up OFF

		} // if (onewire_handler->strong_pull_up_enable)



#endif // __STM32__


	return 0;
	}

//------------------------------------------------------------------------------

int onewire_bus_reset(onewire_handler_s *onewire_handler)
    {
    int result;

#if defined (__STM32__)
	GPIO_InitTypeDef GPIO_InitStructure;
#endif // __STM32__


	if (!onewire_handler)
		return -1;


#if defined (__STM32__)

	// pin jako wyjscie
	// wymuszenie stanu LO na magistrali
	
	GPIO_InitStructure.GPIO_Pin= onewire_handler->data_pin;
	GPIO_InitStructure.GPIO_Speed= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode= GPIO_Mode_Out_PP;
	GPIO_Init((GPIO_TypeDef *)onewire_handler->peripheral_addr, &GPIO_InitStructure);
	GPIO_ResetBits((GPIO_TypeDef *)onewire_handler->peripheral_addr, onewire_handler->data_pin); // data bus: Lo


	msleep(1); // wait at least 480us

	
	// wymuszenie stanu HI na magistrali
	// pin jako wejscie




	timer_1.value_usec= 3000000;
	timer_1.interval_usec= 20;
	timer_1.callback= led_switch;
	timer_1.nrepeat= 1;

	ktimer_create(&timer_1);



	GPIO_InitStructure.GPIO_Mode= GPIO_Mode_IPU;
	GPIO_Init((GPIO_TypeDef *)onewire_handler->peripheral_addr, &GPIO_InitStructure);
	GPIO_SetBits((GPIO_TypeDef *)onewire_handler->peripheral_addr, onewire_handler->data_pin); // data bus: Hi

	



	usleep_sthr(60);


	// odczytaj stan magistrali
    result= (int)GPIO_ReadInputDataBit((GPIO_TypeDef *)onewire_handler->peripheral_addr, onewire_handler->data_pin);


	return xxx;

#else

	// pin jako wyjscie
	// wymuszenie stanu LO na magistrali

	ONEWIRE_INTF_DDRx|= intf_mask;
	ONEWIRE_INTF_PORTx&= 0xFF ^ intf_mask;

	emb_msleep(1); // czekaj > 480 us

	taskENTER_CRITICAL();

	// wymuszenie stanu HI na magistrali
	// pin jako wejscie

	ONEWIRE_INTF_PORTx|= intf_mask;
	ONEWIRE_INTF_DDRx&= 0xFF ^ intf_mask;

	_delay_us(75);

	// odczytaj stan magistrali

	result= ONEWIRE_INTF_PINx & intf_mask;

	taskEXIT_CRITICAL();

	emb_msleep(1); // czekaj > 480 us

	return ~result;

#endif

    }

//------------------------------------------------------------------------------

void onewire_write_byte(unsigned char intf_mask, unsigned char byte)
    {
    unsigned char bit_cntr= 7;

    do
		{
		onewire_write_bit(intf_mask, byte & 0x01);
		byte>>= 1;
		} while (bit_cntr-- != 0);
    }

//------------------------------------------------------------------------------

void onewire_read_byte(unsigned char intf_mask, unsigned char *read_byte_tab)
    {
    unsigned char bit_cntr= 7;

    do
		{
		unsigned char x= 0;
		unsigned char recv_bits= onewire_read_bit(intf_mask);

		for (unsigned char bit_mask=0x01;bit_mask!=0x00;bit_mask<<=1)
			{
			if (bit_mask & intf_mask)
				{
				read_byte_tab[x]>>= 1;
				if (recv_bits & bit_mask)
					read_byte_tab[x]|= 0x80;
				x++;
				}
			}

		} while (bit_cntr-- != 0);

    }

//------------------------------------------------------------------------------

void onewire_write_bit(unsigned char intf_mask, unsigned char bit)
    {
    taskENTER_CRITICAL();

    // pin jako wyjscie
    // wymuszenie stanu LO na magistrali

    ONEWIRE_INTF_DDRx|= intf_mask;
    ONEWIRE_INTF_PORTx&= 0xFF ^ intf_mask;

    if (bit)
		_delay_us(5);
    else
		_delay_us(70);

    // wymuszenie stanu HI na magistrali
    // pin jako wejscie

    ONEWIRE_INTF_PORTx|= intf_mask;
    ONEWIRE_INTF_DDRx&= 0xFF ^ intf_mask;

    taskEXIT_CRITICAL();

    if (bit)
		_delay_us(70);
    else
		_delay_us(5);

    }

//------------------------------------------------------------------------------

unsigned char onewire_read_bit(unsigned char intf_mask)
    {
    unsigned char result;

    taskENTER_CRITICAL();

    // pin jako wyjscie
    // wymuszenie stanu LO na magistrali

    ONEWIRE_INTF_DDRx|= intf_mask;
    ONEWIRE_INTF_PORTx&= 0xFF ^ intf_mask;

    _delay_us(5);

    // wymuszenie stanu HI na magistrali
    // pin jako wejscie

    ONEWIRE_INTF_PORTx|= intf_mask;
    ONEWIRE_INTF_DDRx&= 0xFF ^ intf_mask;

    _delay_us(10);

    // odczytaj stan magistrali

    result= ONEWIRE_INTF_PINx & intf_mask;

    taskEXIT_CRITICAL();

    _delay_us(15);

    return result;
    }

//------------------------------------------------------------------------------

void docrc8(unsigned char *crc8, unsigned char value)
    {
    const prog_char *ptr= &dscrc_table[*crc8 ^ value];
    *crc8= pgm_read_byte(ptr);
    }

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

