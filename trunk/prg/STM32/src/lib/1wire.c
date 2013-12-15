/*------------------------------------------------------------------------------/
 * 1wire.c
 *
 *  Created on: 2010-01-23
 *      Author: przybysz.tomasz@gmail.com
 *
/------------------------------------------------------------------------------*/

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>


#if defined (__STM32__)

#endif // __STM32__

#if defined (__AVR__)
	#include <avr/io.h>
	#include <util/delay.h>
	#include <avr/pgmspace.h>
#endif


#include "ksystem.h"
#include "global.h"
#include "1wire.h"



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

const k_uchar dscrc_table[] = {
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

ktimerlst_spec_s onewire_timer;

k_uchar callback_phase;
xQueueHandle owire_queue= NULL;


void onewire_write_bit_pullup(onewire_handler_s *onewire_handler, k_uchar bit);

void owire_reset_callback(ktimerlst_spec_s *ktimerlst_spec);
void onewire_write_bit_callback(ktimerlst_spec_s *ktimerlst_spec);
void onewire_write_bit_callback_pullup(ktimerlst_spec_s *ktimerlst_spec);
void onewire_read_bit_callback(ktimerlst_spec_s *ktimerlst_spec);

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

	if (!owire_queue)
		owire_queue= xQueueCreate(1, 1);



#if defined (__STM32__)

	switch (onewire_handler->peripheral_addr)
		{

		case (uint32_t)GPIOB:
			{

			if (!(GPIOx_clk_enable & GPIOB_CLK_ENABLE_BIT))
				{
				// only once
                RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
				GPIOx_clk_enable|= GPIOB_CLK_ENABLE_BIT;
				}

			break;
			} // GPIOB

		case (uint32_t)GPIOC:
			{

			if (!(GPIOx_clk_enable & GPIOC_CLK_ENABLE_BIT))
				{
				// only once
                RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
				GPIOx_clk_enable|= GPIOC_CLK_ENABLE_BIT;
				}

			break;
			} // GPIOC

		case (uint32_t)GPIOD:
			{

			if (!(GPIOx_clk_enable & GPIOD_CLK_ENABLE_BIT))
				{
				// only once
                RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
				GPIOx_clk_enable|= GPIOD_CLK_ENABLE_BIT;
				}

			break;
			} // GPIOD

		default:
			{
			return -1;
			}

		} // switch (onewire_handler->peripheral_addr)


	// data bus
	GPIO_InitStructure.GPIO_Pin= onewire_handler->data_pin;
	GPIO_InitStructure.GPIO_Speed= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode= GPIO_Mode_Out_OD;
	GPIO_Init((GPIO_TypeDef *)onewire_handler->peripheral_addr, &GPIO_InitStructure);
	GPIO_SetBits((GPIO_TypeDef *)onewire_handler->peripheral_addr, onewire_handler->data_pin); // data bus: Hi


	// strong pull-up
	if (onewire_handler->strong_pull_up_enable)
		{
		GPIO_InitStructure.GPIO_Pin= onewire_handler->strong_pull_up_pin;
		GPIO_InitStructure.GPIO_Speed= GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode= GPIO_Mode_Out_OD;
		GPIO_Init((GPIO_TypeDef *)onewire_handler->peripheral_addr, &GPIO_InitStructure);
        GPIO_SetBits((GPIO_TypeDef *)onewire_handler->peripheral_addr, onewire_handler->strong_pull_up_pin); // strong pull-up OFF
		} // if (onewire_handler->strong_pull_up_enable)



#endif // __STM32__


	return 0;
	}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

k_uchar onewire_bus_reset(onewire_handler_s *onewire_handler)
    {
	k_uchar ow_result;

#if defined (__STM32__)
	GPIO_InitTypeDef GPIO_InitStructure;
#endif // __STM32__


	if (!onewire_handler)
		return (k_uchar)-1;


#if defined (__STM32__)

	// pin jako wyjscie

	GPIO_InitStructure.GPIO_Pin= onewire_handler->data_pin;
	GPIO_InitStructure.GPIO_Speed= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode= GPIO_Mode_Out_OD;
	GPIO_Init((GPIO_TypeDef *)onewire_handler->peripheral_addr, &GPIO_InitStructure);


	xQueueReset(owire_queue);

	onewire_timer.callback= owire_reset_callback;
	onewire_timer.callback_param= (void *)onewire_handler;
	onewire_timer.value_usec[0]= 0;		// output, Lo
	onewire_timer.value_usec[1]= ONEWIRE_PULSE_WIDTH_RESET_LO;	// 480us //  input, Hi
	onewire_timer.value_usec[2]= ONEWIRE_PULSE_WIDTH_RESET_HI;	// 80us, input, Hi, read
	onewire_timer.nrepeat= 3;

	ktimerlst_create(&onewire_timer);

	xQueueReceive(owire_queue, &ow_result, portMAX_DELAY); // czekam na zakoñczenie obs³ugi


	msleep(1);

	// pin jako wyjscie
	// wymuszenie stanu HI na magistrali
	GPIO_InitStructure.GPIO_Mode= GPIO_Mode_Out_OD;
	GPIO_Init((GPIO_TypeDef *)onewire_handler->peripheral_addr, &GPIO_InitStructure);

	((GPIO_TypeDef *)onewire_handler->peripheral_addr)->BSRR= (GPIO_TypeDef *)onewire_handler->peripheral_addr; // data bus: Hi

	msleep(1);

	return ow_result;

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

void owire_reset_callback(ktimerlst_spec_s *ktimerlst_spec)
	{
	k_uchar callback_resp;
	static signed portBASE_TYPE xHigherPriorityTaskWoken= pdFALSE;
	onewire_handler_s *onewire_handler= (onewire_handler_s *)ktimerlst_spec->callback_param;

#if defined (__STM32__)
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_TypeDef* GPIOx= (GPIO_TypeDef *)onewire_handler->peripheral_addr;
#endif // __STM32__


	switch (ktimerlst_spec->phase)
		{

		case 0x00:
			{
			// pin jako wyjscie
			// wymuszenie stanu LO na magistrali

			GPIOx->BRR= onewire_handler->data_pin;

			break;
			}

		case 0x01:
			{
			// pin jako wejscie
			// wymuszenie stanu HI na magistrali

			GPIO_InitStructure.GPIO_Pin= onewire_handler->data_pin;
			GPIO_InitStructure.GPIO_Speed= GPIO_Speed_50MHz;
			GPIO_InitStructure.GPIO_Mode= GPIO_Mode_IN_FLOATING;
			GPIO_Init((GPIO_TypeDef *)onewire_handler->peripheral_addr, &GPIO_InitStructure);

			GPIOx->BSRR= onewire_handler->data_pin;

			break;
			}

		case 0x02:
			{
			// odczytaj stan magistrali

			callback_resp= (GPIOx->IDR & onewire_handler->data_pin) ? 0x01 : 0x00;

			xQueueSendFromISR(owire_queue, &callback_resp, &xHigherPriorityTaskWoken);

			break;
			}

		} // switch (ktimerlst_spec->phase)

	}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void onewire_write_bit(onewire_handler_s *onewire_handler, k_uchar bit)
    {
	k_uchar result;

	if (!onewire_handler)
		return;

#if defined (__STM32__)

	xQueueReset(owire_queue);

	onewire_timer.callback= onewire_write_bit_callback;
	onewire_timer.callback_param= (void *)onewire_handler;
	onewire_timer.value_usec[0]= 0;

	if (bit)
		{
		onewire_timer.value_usec[1]= ONEWIRE_PULSE_WIDTH_WR1_LO;
		onewire_timer.value_usec[2]= ONEWIRE_PULSE_WIDTH_WR_TIMESLOT - ONEWIRE_PULSE_WIDTH_WR1_LO;
		}
	else
		{
		onewire_timer.value_usec[1]= ONEWIRE_PULSE_WIDTH_WR0_LO;
		onewire_timer.value_usec[2]= ONEWIRE_PULSE_WIDTH_WR_TIMESLOT - ONEWIRE_PULSE_WIDTH_WR0_LO;
		}

	onewire_timer.nrepeat= 3;

	ktimerlst_create(&onewire_timer);

	xQueueReceive(owire_queue, &result, portMAX_DELAY); // czekam na zakoñczenie obs³ugi

	return;

#else

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

#endif

    }

//------------------------------------------------------------------------------

void onewire_write_bit_callback(ktimerlst_spec_s *ktimerlst_spec)
	{
	k_uchar callback_resp;
	static signed portBASE_TYPE xHigherPriorityTaskWoken= pdFALSE;
	onewire_handler_s *onewire_handler= (onewire_handler_s *)ktimerlst_spec->callback_param;

#if defined (__STM32__)
	GPIO_TypeDef* GPIOx= (GPIO_TypeDef *)onewire_handler->peripheral_addr;
#endif // __STM32__


	switch (ktimerlst_spec->phase)
		{

		case 0x00:
			{
			// wymuszenie stanu Lo na magistrali
			// pin jako wyjscie
			GPIOx->BRR= onewire_handler->data_pin; // data bus: Lo

			break;
			}

		case 0x01:
			{
			GPIOx->BSRR= onewire_handler->data_pin; // data bus: Hi
			break;
			}

		case 0x02:
			{
			callback_resp= 0x00;
			xQueueSendFromISR(owire_queue, &callback_resp, &xHigherPriorityTaskWoken);
			break;
			}

		} // switch (ktimerlst_spec->phase)

	}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void onewire_write_bit_pullup(onewire_handler_s *onewire_handler, k_uchar bit)
    {
	k_uchar result;

	if (!onewire_handler)
		return;

#if defined (__STM32__)

	xQueueReset(owire_queue);

	onewire_timer.callback= onewire_write_bit_callback_pullup;
	onewire_timer.callback_param= (void *)onewire_handler;
	onewire_timer.value_usec[0]= 0;

	if (bit)
		{
		onewire_timer.value_usec[1]= ONEWIRE_PULSE_WIDTH_WR1_LO;
		onewire_timer.value_usec[2]= ONEWIRE_PULSE_WIDTH_WR_TIMESLOT - ONEWIRE_PULSE_WIDTH_WR1_LO;
		}
	else
		{
		onewire_timer.value_usec[1]= ONEWIRE_PULSE_WIDTH_WR0_LO;
		onewire_timer.value_usec[2]= ONEWIRE_PULSE_WIDTH_WR_TIMESLOT - ONEWIRE_PULSE_WIDTH_WR0_LO;
		}

	onewire_timer.nrepeat= 3;

	ktimerlst_create(&onewire_timer);

	xQueueReceive(owire_queue, &result, portMAX_DELAY); // czekam na zakoñczenie obs³ugi

	return;

#else

#endif

    }

//------------------------------------------------------------------------------

void onewire_write_bit_callback_pullup(ktimerlst_spec_s *ktimerlst_spec)
	{
	static k_uchar callback_resp;
	static signed portBASE_TYPE xHigherPriorityTaskWoken= pdFALSE;
	onewire_handler_s *onewire_handler= (onewire_handler_s *)ktimerlst_spec->callback_param;

#if defined (__STM32__)
	GPIO_TypeDef* GPIOx= (GPIO_TypeDef *)onewire_handler->peripheral_addr;
#endif // __STM32__


	switch (ktimerlst_spec->phase)
		{

		case 0x00:
			{
			// wymuszenie stanu Lo na magistrali
			// pin jako wyjscie
			((GPIO_TypeDef *)onewire_handler->peripheral_addr)->BRR= onewire_handler->data_pin; // data bus: Lo
			break;
			}

		case 0x01:
			{
			((GPIO_TypeDef *)onewire_handler->peripheral_addr)->BSRR= onewire_handler->data_pin; // data bus: Hi
			((GPIO_TypeDef *)onewire_handler->peripheral_addr)->BRR= onewire_handler->strong_pull_up_pin; // strong pull-up ON
			break;
			}

		case 0x02:
			{
			callback_resp= 0x00;
			xQueueSendFromISR(owire_queue, &callback_resp, &xHigherPriorityTaskWoken);
			break;
			}

		} // switch (ktimerlst_spec->phase)

	}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

k_uchar onewire_read_bit(onewire_handler_s *onewire_handler)
    {
	k_uchar result;

#if defined (__STM32__)
	GPIO_InitTypeDef GPIO_InitStructure;
#endif // __STM32__

	if (!onewire_handler)
		return 0;


#if defined (__STM32__)

	xQueueReset(owire_queue);

	onewire_timer.callback= onewire_read_bit_callback;
	onewire_timer.callback_param= (void *)onewire_handler;
	onewire_timer.value_usec[0]= 0;

	onewire_timer.value_usec[1]= ONEWIRE_PULSE_WIDTH_RD_LO;
	onewire_timer.value_usec[2]= ONEWIRE_PULSE_WIDTH_RD_HI;

	onewire_timer.nrepeat= 3;

	ktimerlst_create(&onewire_timer);

	xQueueReceive(owire_queue, &result, portMAX_DELAY); // czekam na zakoñczenie obs³ugi


	// pin jako wyjscie
	// wymuszenie stanu HI na magistrali

	GPIO_InitStructure.GPIO_Pin= onewire_handler->data_pin;
	GPIO_InitStructure.GPIO_Speed= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode= GPIO_Mode_Out_OD;
	GPIO_Init((GPIO_TypeDef *)onewire_handler->peripheral_addr, &GPIO_InitStructure);

	((GPIO_TypeDef *)onewire_handler->peripheral_addr)->BSRR= (GPIO_TypeDef *)onewire_handler->peripheral_addr; // data bus: Hi


	return result;
	

#else

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

#endif

    }

//------------------------------------------------------------------------------

void onewire_read_bit_callback(ktimerlst_spec_s *ktimerlst_spec)
	{
	static k_uchar callback_resp;
	static signed portBASE_TYPE xHigherPriorityTaskWoken= pdFALSE;
	onewire_handler_s *onewire_handler= (onewire_handler_s *)ktimerlst_spec->callback_param;

#if defined (__STM32__)
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_TypeDef* GPIOx= (GPIO_TypeDef *)onewire_handler->peripheral_addr;
#endif // __STM32__


	switch (ktimerlst_spec->phase)
		{

		case 0x00:
			{
			// wymuszenie stanu Lo na magistrali
			// pin jako wyjscie

			GPIOx->BRR= onewire_handler->data_pin; // data bus: Lo

			break;
			}

		case 0x01:
			{
			// pin jako wejscie
			// wymuszenie stanu HI na magistrali

			GPIO_InitStructure.GPIO_Pin= onewire_handler->data_pin;
			GPIO_InitStructure.GPIO_Speed= GPIO_Speed_50MHz;
			GPIO_InitStructure.GPIO_Mode= GPIO_Mode_IN_FLOATING;
			GPIO_Init((GPIO_TypeDef *)onewire_handler->peripheral_addr, &GPIO_InitStructure);

			GPIOx->BSRR= onewire_handler->data_pin; // data bus: Hi

			break;
			}

		case 0x02:
			{
			// odczytaj stan magistrali

			callback_resp= (GPIOx->IDR & onewire_handler->data_pin) ? 0x01 : 0x00;
			xQueueSendFromISR(owire_queue, &callback_resp, &xHigherPriorityTaskWoken);

			break;
			}

		} // switch (ktimerlst_spec->phase)

	}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void onewire_write_byte(onewire_handler_s *onewire_handler, k_uchar byte)
    {
    unsigned char bit_cntr= 7;

	if (!onewire_handler)
		return;

    do
		{
		onewire_write_bit(onewire_handler, byte & 0x01);
		byte>>= 1;
		} while (bit_cntr-- != 0);

    }

//------------------------------------------------------------------------------

void onewire_write_byte_pullup(onewire_handler_s *onewire_handler, k_uchar byte)
    {
    unsigned char bit_cntr= 7;

	if (!onewire_handler)
		return;

    do
		{
		if (bit_cntr != 0)
			onewire_write_bit(onewire_handler, byte & 0x01);
		else
			onewire_write_bit_pullup(onewire_handler, byte & 0x01);
		byte>>= 1;
		} while (bit_cntr-- != 0);

    }

//------------------------------------------------------------------------------

k_uchar onewire_read_byte(onewire_handler_s *onewire_handler)
    {
	k_uchar result= 0x00;
	k_uchar recv_bit;
    k_uchar bit_cntr= 7;

	if (!onewire_handler)
		return 0x00;

    do
		{
		recv_bit= onewire_read_bit(onewire_handler);
		result>>= 1;
		result|= recv_bit ? 0x80 : 0x00;
		} while (bit_cntr-- != 0);

    return result;
    }

//------------------------------------------------------------------------------

void onewire_strong_pullup_enable(onewire_handler_s *onewire_handler, k_uchar state)
	{

	if (!onewire_handler)
		return;

	if (!onewire_handler->strong_pull_up_enable)
		return;


#if defined (__STM32__)

	if (state)
		{
		// strong pullup: enable
		GPIO_ResetBits((GPIO_TypeDef *)onewire_handler->peripheral_addr, onewire_handler->strong_pull_up_pin); // strong pull-up ON
		}
	else
		{
		// strong pullup: disable
		GPIO_SetBits((GPIO_TypeDef *)onewire_handler->peripheral_addr, onewire_handler->strong_pull_up_pin); // strong pull-up OFF
		}

#endif // __STM32__


	}

//------------------------------------------------------------------------------

void onewire_crc8(unsigned char *crc8, unsigned char value)
    {
    const k_uchar *ptr= &dscrc_table[*crc8 ^ value];
    *crc8= *ptr;
    }

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------


