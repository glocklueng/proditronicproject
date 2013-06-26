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

#include "1wire.h"


//#include "emb_delay.h"
//#include "1wire_cfg.h"

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

#define GPIOA_CLK_ENABLE_BIT	(1 << 0)
#define GPIOB_CLK_ENABLE_BIT	(1 << 1)
#define GPIOC_CLK_ENABLE_BIT	(1 << 2)


//ktimer_spec_s onewire_timer;
ktimerlst_spec_s onewire_timer;

k_uchar callback_phase;
xQueueHandle owire_queue= NULL;



void owire_reset_callback(ktimerlst_spec_s *ktimerlst_spec);
void onewire_write_bit_callback(onewire_handler_s *onewire_handler);
void onewire_read_bit_callback(onewire_handler_s *onewire_handler);

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


		default:
			{
			return -1;
			}

		} // switch (onewire_handler->peripheral_addr)


	// data bus
	GPIO_InitStructure.GPIO_Pin= onewire_handler->data_pin;
	GPIO_InitStructure.GPIO_Speed= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode= GPIO_Mode_Out_PP; // GPIO_Mode_Out_OD
	GPIO_Init((GPIO_TypeDef *)onewire_handler->peripheral_addr, &GPIO_InitStructure);

	GPIO_SetBits((GPIO_TypeDef *)onewire_handler->peripheral_addr, onewire_handler->data_pin); // data bus: Hi


	// strong pull-up
	if (onewire_handler->strong_pull_up_enable)
		{
		GPIO_InitStructure.GPIO_Pin= onewire_handler->strong_pull_up_pin;
		GPIO_InitStructure.GPIO_Speed= GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode= GPIO_Mode_Out_PP; // GPIO_Mode_Out_OD ????
		GPIO_Init((GPIO_TypeDef *)onewire_handler->peripheral_addr, &GPIO_InitStructure);

        GPIO_ResetBits((GPIO_TypeDef *)onewire_handler->peripheral_addr, onewire_handler->strong_pull_up_pin); // strong pull-up OFF

		} // if (onewire_handler->strong_pull_up_enable)



#endif // __STM32__


	return 0;
	}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

int onewire_bus_reset(onewire_handler_s *onewire_handler)
    {
    k_uchar result;

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
	GPIO_InitStructure.GPIO_Mode= GPIO_Mode_Out_PP; // GPIO_Mode_Out_OD
	GPIO_Init((GPIO_TypeDef *)onewire_handler->peripheral_addr, &GPIO_InitStructure);
	GPIO_ResetBits((GPIO_TypeDef *)onewire_handler->peripheral_addr, onewire_handler->data_pin); // data bus: Lo


	//msleep(1); // wait at least 480us
	//msleep(1000); // wait at least 480us
	

	onewire_timer.callback= owire_reset_callback;
	onewire_timer.callback_param= (void *)onewire_handler;
	onewire_timer.value_usec[0]= 65535;
	onewire_timer.nrepeat= 1;


	ktimerlst_create(&onewire_timer);







/*
	// wywolaj funkcje callback
	onewire_timer.value_usec= 0;
	onewire_timer.interval_usec= 1000000; // 80 usec
	onewire_timer.callback= owire_reset_callback;
	onewire_timer.callback_param= (void *)onewire_handler;
	onewire_timer.nrepeat= 2;


	xQueueReset(owire_queue);

	callback_phase= 0x00;
	ktimer_create(&onewire_timer);

	xQueueReceive(owire_queue, &result, ((portTickType)5000 / portTICK_RATE_MS)); // czekam na zakoñczenie obs³ugi
*/

	// pin jako wyjscie
	// wymuszenie stanu HI na magistrali

/*
	GPIO_InitStructure.GPIO_Mode= GPIO_Mode_Out_PP; // GPIO_Mode_Out_OD
	GPIO_Init((GPIO_TypeDef *)onewire_handler->peripheral_addr, &GPIO_InitStructure);
	GPIO_SetBits((GPIO_TypeDef *)onewire_handler->peripheral_addr, onewire_handler->data_pin); // data bus: Hi
*/
	msleep(1);

	return result;

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


void led_switch_x()
	{
	static char ledstate= 0;

	if (ledstate)
		{
		ledstate= 0;
		GPIO_ResetBits(GPIOB , GPIO_Pin_1);
		}
	else
		{
		ledstate= 1;
		GPIO_SetBits(GPIOB ,  GPIO_Pin_1);
		}

	}

void owire_reset_callback(ktimerlst_spec_s *ktimerlst_spec)
	{
	static k_uchar callback_resp;
	static signed portBASE_TYPE xHigherPriorityTaskWoken= pdFALSE;
	onewire_handler_s *onewire_handler= (onewire_handler_s *)ktimerlst_spec->callback_param;



#if defined (__STM32__)
	GPIO_InitTypeDef GPIO_InitStructure;
#endif // __STM32__


	GPIO_ResetBits(GPIOB , GPIO_Pin_1);


	/*
	switch (callback_phase)
		{

		case 0x00:
			{

			callback_phase= 0x01;
			break;
			}

		case 0x01:
			{
			// odczytaj stan magistrali

			callback_resp= GPIO_ReadInputDataBit((GPIO_TypeDef *)onewire_handler->peripheral_addr, onewire_handler->data_pin);
			xQueueSendFromISR(owire_queue, &callback_resp, &xHigherPriorityTaskWoken);

			break;
			}

		} // switch (callback_phase)
*/
	}

/*
void owire_reset_callback(onewire_handler_s *onewire_handler)
	{
	static k_uchar callback_resp;
	static signed portBASE_TYPE xHigherPriorityTaskWoken= pdFALSE;

#if defined (__STM32__)
	GPIO_InitTypeDef GPIO_InitStructure;
#endif // __STM32__


	switch (callback_phase)
		{

		case 0x00:
			{
			// wymuszenie stanu HI na magistrali
			// pin jako wejscie

			GPIO_InitStructure.GPIO_Pin= onewire_handler->data_pin;
			GPIO_InitStructure.GPIO_Speed= GPIO_Speed_50MHz;
			GPIO_InitStructure.GPIO_Mode= GPIO_Mode_IPU;
			GPIO_Init((GPIO_TypeDef *)onewire_handler->peripheral_addr, &GPIO_InitStructure);
			GPIO_SetBits((GPIO_TypeDef *)onewire_handler->peripheral_addr, onewire_handler->data_pin); // data bus: Hi

			callback_phase= 0x01;
			break;
			}

		case 0x01:
			{
			// odczytaj stan magistrali

			callback_resp= GPIO_ReadInputDataBit((GPIO_TypeDef *)onewire_handler->peripheral_addr, onewire_handler->data_pin);
			xQueueSendFromISR(owire_queue, &callback_resp, &xHigherPriorityTaskWoken);

			break;
			}

		} // switch (callback_phase)

	}
*/
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/*
void onewire_write_bit(onewire_handler_s *onewire_handler, k_uchar bit)
    {

#if defined (__STM32__)

	if (!onewire_handler)
		return;

	// wywolaj funkcje callback
	onewire_timer.value_usec= 0;
	onewire_timer.interval_usec= (bit == 0x01) ? 10 : 80; // usec
	onewire_timer.callback= onewire_write_bit_callback;
	onewire_timer.callback_param= (void *)onewire_handler;
	onewire_timer.nrepeat= 2;

	callback_phase= 0x00;
	ktimer_create(&onewire_timer);

	msleep(1);

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

void onewire_write_bit_callback(onewire_handler_s *onewire_handler)
	{

#if defined (__STM32__)
	GPIO_InitTypeDef GPIO_InitStructure;
#endif // __STM32__


	switch (callback_phase)
		{

		case 0x00:
			{
			// wymuszenie stanu Lo na magistrali
			// pin jako wyjscie

			GPIO_ResetBits((GPIO_TypeDef *)onewire_handler->peripheral_addr, onewire_handler->data_pin); // data bus: Lo

			callback_phase= 0x01;
			break;
			}

		case 0x01:
			{

			GPIO_SetBits((GPIO_TypeDef *)onewire_handler->peripheral_addr, onewire_handler->data_pin); // data bus: Hi

			break;
			}

		} // switch (callback_phase)

	}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

k_uchar onewire_read_bit(onewire_handler_s *onewire_handler)
    {
	k_uchar result;

#if defined (__STM32__)
	GPIO_InitTypeDef GPIO_InitStructure;
#endif // __STM32__



#if defined (__STM32__)

	if (!onewire_handler)
		return 0x00;

	// wywolaj funkcje callback
	onewire_timer.value_usec= 0;
	onewire_timer.interval_usec= 5; // usec
	onewire_timer.callback= onewire_read_bit_callback;
	onewire_timer.callback_param= (void *)onewire_handler;
	onewire_timer.nrepeat= 3;


	xQueueReset(owire_queue);

	callback_phase= 0x00;
	ktimer_create(&onewire_timer);

	xQueueReceive(owire_queue, &result, ((portTickType)1000 / portTICK_RATE_MS)); // czekam na zakoñczenie obs³ugi


	// pin jako wyjscie
	// wymuszenie stanu HI na magistrali

	GPIO_InitStructure.GPIO_Pin= onewire_handler->data_pin;
	GPIO_InitStructure.GPIO_Speed= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode= GPIO_Mode_Out_PP; // GPIO_Mode_Out_OD
	GPIO_Init((GPIO_TypeDef *)onewire_handler->peripheral_addr, &GPIO_InitStructure);
	GPIO_SetBits((GPIO_TypeDef *)onewire_handler->peripheral_addr, onewire_handler->data_pin); // data bus: Hi

	msleep(1);

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

void onewire_read_bit_callback(onewire_handler_s *onewire_handler)
	{
	static k_uchar callback_resp;
	static signed portBASE_TYPE xHigherPriorityTaskWoken= pdFALSE;

#if defined (__STM32__)
	GPIO_InitTypeDef GPIO_InitStructure;
#endif // __STM32__


	switch (callback_phase)
		{

		case 0x00:
			{
			// wymuszenie stanu Lo na magistrali
			// pin jako wyjscie

			GPIO_ResetBits((GPIO_TypeDef *)onewire_handler->peripheral_addr, onewire_handler->data_pin); // data bus: Lo

			callback_phase= 0x01;
			break;
			}

		case 0x01:
			{
			// wymuszenie stanu HI na magistrali
			// pin jako wejscie

			GPIO_InitStructure.GPIO_Pin= onewire_handler->data_pin;
			GPIO_InitStructure.GPIO_Speed= GPIO_Speed_50MHz;
			GPIO_InitStructure.GPIO_Mode= GPIO_Mode_IPU;
			GPIO_Init((GPIO_TypeDef *)onewire_handler->peripheral_addr, &GPIO_InitStructure);
			GPIO_SetBits((GPIO_TypeDef *)onewire_handler->peripheral_addr, onewire_handler->data_pin); // data bus: Hi

			callback_phase= 0x02;
			break;
			}

		case 0x02:
			{
			// odczytaj stan magistrali

			callback_resp= GPIO_ReadInputDataBit((GPIO_TypeDef *)onewire_handler->peripheral_addr, onewire_handler->data_pin);
			xQueueSendFromISR(owire_queue, &callback_resp, &xHigherPriorityTaskWoken);

			break;
			}


		} // switch (callback_phase)

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
		GPIO_SetBits((GPIO_TypeDef *)onewire_handler->peripheral_addr, onewire_handler->strong_pull_up_pin); // strong pull-up OFF
		}
	else
		{
		// strong pullup: disable
		GPIO_ResetBits((GPIO_TypeDef *)onewire_handler->peripheral_addr, onewire_handler->strong_pull_up_pin); // strong pull-up OFF
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

*/
