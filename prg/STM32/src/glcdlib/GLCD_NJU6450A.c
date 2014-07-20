/*-----------------------------------------------------------------------------/
 * GLCD_NJU6450
 *
 *  Created on: 17-05-2014
 *      Author: Tomasz Przybysz
/-----------------------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <FreeRTOS.h>
#include <task.h>

#include "stm32f10x.h"
#include "platform_config.h"

#include "font5x7V.h"
#include "lcd_interface.h"

#include "GLCD_NJU6450A.h"

//------------------------------------------------------------------------------


GPIO_InitTypeDef gpio_data;


#define LCD_A0_PIN				GPIO_Pin_1
#define LCD_E1_PIN				GPIO_Pin_0
#define LCD_E2_PIN				GPIO_Pin_3
#define LCD_RW_PIN				GPIO_Pin_2
#define LCD_RST_PIN				GPIO_Pin_10

#define LCD_PWM_CONTRAST		GPIO_Pin_1
#define LCD_PWM_BACKLIGHT		GPIO_Pin_0


#define CYCLES_10n				5

#define LCD_PWM_TIMER_FREQ		120000			// Hz
#define LCD_PWM_PERIOD			100 			//


//------------------------------------------------------------------------------

extern const unsigned char font_5x7_V_data[FONT_DATA_SIZE * 95];

unsigned char c1posx, c1posy;
unsigned char c2posx, c2posy;

const unsigned char font2size_tab[16]= {
	0x00, 0x03, 0x0C, 0x0F,
	0x30, 0x33, 0x3C, 0x3F,
	0xC0, 0xC3, 0xCC, 0xCF,
	0xF0, 0xF3, 0xFC, 0xFF};




//------------------------------------------------------------------------------

void GLCD_NJU6450A_init();

void GLCD_NJU6450A_screen_clear();
void GLCD_NJU6450A_display_on();
void GLCD_NJU6450A_display_off();

void GLCD_NJU6450A_char_draw(unsigned char code, unsigned char column, unsigned char row, unsigned char attr);
void GLCD_NJU6450A_region_fill(unsigned char posx, unsigned char posy, unsigned char width, unsigned char height, unsigned char data);
void GLCD_NJU6450A_bmp_draw(unsigned char posx, unsigned char posy, unsigned char width, unsigned char height, unsigned char attr, unsigned char *bmpptr);

//------------------------------------------------------------------------------

void GLCD_NJU6450A_reset();

void GLCD_NJU6450A_data_wr(unsigned char chip, unsigned char data);
unsigned char GLCD_NJU6450A_data_rd(unsigned char chip);
void GLCD_NJU6450A_cmd_wr(unsigned char chip, unsigned char data);
void GLCD_NJU6450A_busy(unsigned char chip);

void GLCD_NJU6450A_column_set(unsigned char column);
void GLCD_NJU6450A_row_set(unsigned char chip, unsigned char row);

void GLCD_NJU6450A_char1s_draw(unsigned char code, unsigned char column, unsigned char row, bool invert);
void GLCD_NJU6450A_char2s_draw(unsigned char code, unsigned char column, unsigned char row, bool invert);



void GLCD_NJU6450A_contrast_set(uint8_t value);
void GLCD_NJU6450A_backlight_set(uint8_t value);

void Delay (uint32_t nCount);

//------------------------------------------------------------------------------



lcd_handler_s lcd=
	{
	.init=			GLCD_NJU6450A_init,
	.screen_clear=	GLCD_NJU6450A_screen_clear,
	.display_on=	GLCD_NJU6450A_display_on,
	.display_off=	GLCD_NJU6450A_display_off,

	.char_draw=		GLCD_NJU6450A_char_draw,
	.region_fill=	GLCD_NJU6450A_region_fill,
	.bmp_draw=		GLCD_NJU6450A_bmp_draw,

	.contrast_set=	GLCD_NJU6450A_contrast_set,
	.backlight_set=	GLCD_NJU6450A_backlight_set,

	.width=			SCREEN_WIDTH,
	.height=		SCREEN_HEIGHT,
	.cols=			SCREEN_COLS,
	.rows=			SCREEN_ROWS,

	};


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void GLCD_NJU6450A_init()
	{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	uint16_t PrescalerValue;


	c1posx= 0xFF;
	c1posy= 0xFF;

	c2posx= 0xFF;
	c2posy= 0xFF;


	RCC_APB2PeriphClockCmd(LCD_GPIO_CLK, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);


	GPIO_InitStructure.GPIO_Pin= LCD_A0_PIN | LCD_E1_PIN | LCD_E2_PIN | LCD_RW_PIN;
	GPIO_InitStructure.GPIO_Speed= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode= GPIO_Mode_Out_OD;
	GPIO_Init(LCD_GPIO, &GPIO_InitStructure);
	GPIO_ResetBits(LCD_GPIO, LCD_A0_PIN | LCD_E1_PIN | LCD_E2_PIN | LCD_RW_PIN);

	gpio_data.GPIO_Pin= 0xFF00;
	gpio_data.GPIO_Speed= GPIO_Speed_50MHz;
	gpio_data.GPIO_Mode= GPIO_Mode_Out_OD;
	GPIO_Init(LCD_GPIO, &gpio_data);


	GPIO_InitStructure.GPIO_Pin= LCD_RST_PIN;
	GPIO_InitStructure.GPIO_Speed= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode= GPIO_Mode_Out_OD;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, LCD_RST_PIN);





// PWM Contrast, PWM Backlight

	PrescalerValue= (uint16_t)(SystemCoreClock / LCD_PWM_TIMER_FREQ) - 1;

	if ((SystemCoreClock / LCD_PWM_TIMER_FREQ) >= 0x10000)
		printf("<ERR> %s: TIM_Prescaler overrange !\n", __FUNCTION__);

	//if (((int)LCD_PWM_PERIOD * LCD_PWM_TIMER_FREQ) >= 0x10000)
		//printf("<ERR> %s: TIM_Period overrange !\n", __FUNCTION__);


	// TIMx configuration
	TIM_TimeBaseStructure.TIM_Period= (uint16_t)LCD_PWM_PERIOD;
	TIM_TimeBaseStructure.TIM_Prescaler= PrescalerValue;
	TIM_TimeBaseStructure.TIM_ClockDivision= 0;
	TIM_TimeBaseStructure.TIM_CounterMode= TIM_CounterMode_Up;
	TIM_TimeBaseInit(LCD_PWM_TIMER, &TIM_TimeBaseStructure);

	// Immediate load of TIM2,TIM3 and TIM4 Prescaler values
	TIM_PrescalerConfig(LCD_PWM_TIMER, PrescalerValue, TIM_PSCReloadMode_Immediate);

	TIM_ARRPreloadConfig(LCD_PWM_TIMER, ENABLE);

	// TIM2, TIM3 and TIM4 enable counters
	TIM_Cmd(LCD_PWM_TIMER, ENABLE);


	GPIO_InitStructure.GPIO_Pin= LCD_PWM_CONTRAST | LCD_PWM_BACKLIGHT;
	GPIO_InitStructure.GPIO_Speed= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode= GPIO_Mode_Out_PP;
	GPIO_Init(LCD_PWM_GPIO, &GPIO_InitStructure);
	GPIO_SetBits(LCD_PWM_GPIO, LCD_PWM_CONTRAST | LCD_PWM_BACKLIGHT);



	GLCD_NJU6450A_contrast_set(0);
	GLCD_NJU6450A_backlight_set(0);


	GLCD_NJU6450A_reset();
	}

//------------------------------------------------------------------------------

void GLCD_NJU6450A_busy(unsigned char chip)
	{
	volatile unsigned short read16;

	gpio_data.GPIO_Mode= GPIO_Mode_IN_FLOATING;
	GPIO_Init(LCD_GPIO, &gpio_data);

	LCD_GPIO->BRR= LCD_E1_PIN | LCD_E2_PIN | LCD_A0_PIN;
	LCD_GPIO->BSRR= LCD_RW_PIN;

	while (1)
		{
		//Delay(CYCLES_10n * 2);
		Delay(9); // 10

		LCD_GPIO->BSRR= (chip == 0) ? LCD_E1_PIN : LCD_E2_PIN;

//		Delay(CYCLES_10n * 9);
		Delay(9);

		read16= LCD_GPIO->IDR;

		if (!(read16 & 0x8000))
			break;

		Delay(1);

		LCD_GPIO->BRR= LCD_E1_PIN | LCD_E2_PIN;

		} // while (1)

	LCD_GPIO->BRR= LCD_E1_PIN | LCD_E2_PIN | LCD_RW_PIN | LCD_A0_PIN;

//	Delay(CYCLES_10n * 5); // czekam na wy³¹czenie nadawania w LCD
	Delay(3); // czekam na wy³¹czenie nadawania w LCD
// 5!!!
	}

//------------------------------------------------------------------------------

unsigned char GLCD_NJU6450A_data_rd(unsigned char chip)
	{
	volatile unsigned short read16;

	GLCD_NJU6450A_busy(chip);


	gpio_data.GPIO_Mode= GPIO_Mode_IN_FLOATING;
	GPIO_Init(LCD_GPIO, &gpio_data);

	LCD_GPIO->BRR= LCD_E1_PIN | LCD_E2_PIN;
	LCD_GPIO->BSRR= LCD_RW_PIN | LCD_A0_PIN;

	Delay(CYCLES_10n * 2);

	LCD_GPIO->BSRR= (chip == 0) ? LCD_E1_PIN : LCD_E2_PIN;

	Delay(CYCLES_10n * 100);

	read16= LCD_GPIO->IDR;

	LCD_GPIO->BRR= LCD_E1_PIN | LCD_E2_PIN | LCD_RW_PIN | LCD_A0_PIN;
	Delay(CYCLES_10n * 5); // czekam na wy³¹czenie nadawania w LCD

	read16>>= 8;

	return (unsigned char)read16;
	}


//------------------------------------------------------------------------------

void GLCD_NJU6450A_data_wr(unsigned char chip, unsigned char data)
	{
	uint32_t tmp;

	GLCD_NJU6450A_busy(chip);

	gpio_data.GPIO_Mode= GPIO_Mode_Out_OD;
	GPIO_Init(LCD_GPIO, &gpio_data);

	LCD_GPIO->BRR= LCD_E1_PIN | LCD_E2_PIN | LCD_RW_PIN;
	LCD_GPIO->BSRR= LCD_A0_PIN;

	tmp= LCD_GPIO->ODR;
	tmp&= 0x00FF;
	tmp|= ((uint32_t)data << 8);
	LCD_GPIO->ODR= tmp;
	Delay(2); // 2

	LCD_GPIO->BSRR= (chip == 0) ? LCD_E1_PIN : LCD_E2_PIN;
	Delay(8);  //9

	LCD_GPIO->BRR= LCD_E1_PIN | LCD_E2_PIN;
	Delay(2);

	LCD_GPIO->BRR= LCD_RW_PIN | LCD_A0_PIN;
	Delay(2);

	}

//------------------------------------------------------------------------------

void GLCD_NJU6450A_cmd_wr(unsigned char chip, unsigned char data)
	{
	uint32_t tmp;

	GLCD_NJU6450A_busy(chip);

	gpio_data.GPIO_Mode= GPIO_Mode_Out_OD;
	GPIO_Init(LCD_GPIO, &gpio_data);

	LCD_GPIO->BRR= LCD_E1_PIN | LCD_E2_PIN | LCD_RW_PIN | LCD_A0_PIN;

	tmp= LCD_GPIO->ODR;
	tmp&= 0x00FF;
	tmp|= ((uint32_t)data << 8);
	LCD_GPIO->ODR= tmp;
	Delay(2);

	LCD_GPIO->BSRR= (chip == 0) ? LCD_E1_PIN : LCD_E2_PIN;
	Delay(8); // 9

	LCD_GPIO->BRR= LCD_E1_PIN | LCD_E2_PIN;
	Delay(2);

	LCD_GPIO->BRR= LCD_RW_PIN | LCD_A0_PIN;
	Delay(2);

	}

//------------------------------------------------------------------------------

void GLCD_NJU6450A_reset()
	{

	GPIO_ResetBits(GPIOB, LCD_RST_PIN);
	vTaskDelay(3);
	GPIO_SetBits(GPIOB, LCD_RST_PIN);

	GLCD_NJU6450A_cmd_wr(0, 0xAE);	//wylacz wyswietlac
	GLCD_NJU6450A_cmd_wr(1, 0xAE);

	GLCD_NJU6450A_cmd_wr(0, 0xA0);	//tryb norma (ADC)
	GLCD_NJU6450A_cmd_wr(1, 0xA0);

	GLCD_NJU6450A_cmd_wr(0, 0xA4);	//dynamic drive
	GLCD_NJU6450A_cmd_wr(1, 0xA4);

	GLCD_NJU6450A_cmd_wr(0, 0xA9); 	// duty 1/32
	GLCD_NJU6450A_cmd_wr(1, 0xA9); 	//

	GLCD_NJU6450A_cmd_wr(0, 0xAF); 	//wlacz wyswietlacz
	GLCD_NJU6450A_cmd_wr(1, 0xAF);

	GLCD_NJU6450A_cmd_wr(0, 0xB8 | 0); // page
	GLCD_NJU6450A_cmd_wr(1, 0xB8 | 0);

	GLCD_NJU6450A_cmd_wr(0, 0x00);	// column
	GLCD_NJU6450A_cmd_wr(1, 0x00);

	}

//------------------------------------------------------------------------------

void GLCD_NJU6450A_screen_clear()
	{
	unsigned char x, y, page;
//	static unsigned char val= 0x00;

	c1posx= 0xFF;
	c1posy= 0xFF;

	c2posx= 0xFF;
	c2posy= 0xFF;

	page= 0;

//	GLCD_NJU6450A_display_off();

	for (y=0;y<SCREEN_ROWS;y++)
		{
		GLCD_NJU6450A_cmd_wr(0, 0xB8 | page);
		GLCD_NJU6450A_cmd_wr(1, 0xB8 | page);

		GLCD_NJU6450A_cmd_wr(0, 0x00);	// column
		GLCD_NJU6450A_cmd_wr(1, 0x00);

		//int maxx= (val == 0) ? SUBSCREEN_WIDTH : SUBSCREEN_WIDTH -1;


		for (x=0;x<SUBSCREEN_WIDTH;x++)
			{

			//GLCD_NJU6450A_column_set(x);
			//GLCD_NJU6450A_data_wr(0, 0x55);


			//GLCD_NJU6450A_column_set(x+ SUBSCREEN_WIDTH);
			//GLCD_NJU6450A_data_wr(1, 0x55);

//			GLCD_NJU6450A_data_wr(0, !val ? val : (x & 1) ? 0x55 : 0xAA);
//			GLCD_NJU6450A_data_wr(1, !val ? val : (x & 1) ? 0x55 : 0xAA);

			GLCD_NJU6450A_data_wr(0, 0x00);
			GLCD_NJU6450A_data_wr(1, 0x00);

			}

		page+= 1;
		}

	//val= !val ? 1 : 0;

	//GLCD_NJU6450A_display_on();

	}

//------------------------------------------------------------------------------

void GLCD_NJU6450A_display_on()
	{
	GLCD_NJU6450A_cmd_wr(0, 0xAF);	// display: ON
	GLCD_NJU6450A_cmd_wr(1, 0xAF);
	}

//------------------------------------------------------------------------------

void GLCD_NJU6450A_display_off()
	{
	GLCD_NJU6450A_cmd_wr(0, 0xAE);	// display: OFF
	GLCD_NJU6450A_cmd_wr(1, 0xAE);
	}

//------------------------------------------------------------------------------

void GLCD_NJU6450A_column_set(unsigned char column)
	{
	if (column < SUBSCREEN_WIDTH)
		{
		c1posx= column;
        GLCD_NJU6450A_cmd_wr(0, column);
		}
	else
	if ((column - SUBSCREEN_WIDTH) < SUBSCREEN_WIDTH)
		{
		c2posx= column;
		GLCD_NJU6450A_cmd_wr(1, column - SUBSCREEN_WIDTH);
		}
	}

//------------------------------------------------------------------------------

void GLCD_NJU6450A_row_set(unsigned char chip, unsigned char row)
	{
	if (row < SCREEN_ROWS)
		{
		if (chip == 0)
			{
			c1posy= row;
			GLCD_NJU6450A_cmd_wr(0, 0xB8 | row);
			}
		else
			{
			c2posy= row;
			GLCD_NJU6450A_cmd_wr(1, 0xB8 | row);
			}
		}
	}

//------------------------------------------------------------------------------

void GLCD_NJU6450A_char_draw(unsigned char code, unsigned char column, unsigned char row, unsigned char attr)
	{
	if (!(attr & TPGUI_ITEM_ATTRIB_DOUBLESIZE))
		GLCD_NJU6450A_char1s_draw(code, column, row, (bool)(attr & TPGUI_ITEM_ATTRIB_INVERT));
	else
		GLCD_NJU6450A_char2s_draw(code, column, row, (bool)(attr & TPGUI_ITEM_ATTRIB_INVERT)); // double size font
	}

//------------------------------------------------------------------------------

void GLCD_NJU6450A_char1s_draw(unsigned char code, unsigned char column, unsigned char row, bool invert)
	{
	unsigned char x;
	unsigned char *char_fontdta;
	static unsigned char val=0;

	if (column != 0)
		column*= FONT_CHAR_WIDTH;

	column+= GLCD_X_OFFSET;

	if ((code < 0x20) || (code > 0x7E))
		char_fontdta= (unsigned char *)font_5x7_V_data; // space
	else
		char_fontdta= (unsigned char *)&font_5x7_V_data[(code - 0x20) * FONT_DATA_SIZE];


	if (column < SUBSCREEN_WIDTH)
		{
		// CHIP1 Left

		if (column != c1posx)
			GLCD_NJU6450A_column_set(column);

		if (row != c1posy)
			GLCD_NJU6450A_row_set(0, row);

		if (!invert)
			{
			for (x=0;x<(FONT_CHAR_WIDTH-1);x++)
				GLCD_NJU6450A_data_wr(0, *char_fontdta++);

			GLCD_NJU6450A_data_wr(0, 0x00);
			}
		else
			{
			for (x=0;x<(FONT_CHAR_WIDTH-1);x++)
				GLCD_NJU6450A_data_wr(0, 0xFF ^ *char_fontdta++);

			GLCD_NJU6450A_data_wr(0, 0xFF);
			}

		c1posx+= FONT_CHAR_WIDTH;

		} // CHIP1 Left
	else
		{
		// CHIP2 Right

		if (column != c2posx)
			GLCD_NJU6450A_column_set(column);

		if (row != c2posy)
			GLCD_NJU6450A_row_set(1, row);

		if (!invert)
			{
			for (x=0;x<(FONT_CHAR_WIDTH-1);x++)
				GLCD_NJU6450A_data_wr(1, *char_fontdta++);

			GLCD_NJU6450A_data_wr(1, 0x00);
			}
		else
			{
			for (x=0;x<(FONT_CHAR_WIDTH-1);x++)
				GLCD_NJU6450A_data_wr(1, 0xFF ^ *char_fontdta++);

			GLCD_NJU6450A_data_wr(1, 0xFF);
			}

		c2posx+= FONT_CHAR_WIDTH;

		} // CHIP2 Right

	val= !val ? 1:0;
	}

//------------------------------------------------------------------------------

void GLCD_NJU6450A_char2s_draw(unsigned char code, unsigned char column, unsigned char row, bool invert)
	{
	unsigned char x;
	unsigned char *char_fontdta;
	unsigned char line= 0;
	unsigned char data2write;
	unsigned char column_t;

	if (column != 0)
		column*= FONT_CHAR_WIDTH;

	column+= GLCD_X_OFFSET;



	if ((code < 0x20) || (code > 0x7E))
		code= 0x00;
	else
		code-= 0x20;


	while (line < 2)
		{

		char_fontdta= (unsigned char *)&font_5x7_V_data[code * FONT_DATA_SIZE];
		column_t= column;

		for (x=0;x<FONT_CHAR_WIDTH;x++)
			{
			
			if (column_t < SUBSCREEN_WIDTH)
				{
				// CHIP1 Left

				if (column_t != c1posx)
					GLCD_NJU6450A_column_set(column_t);

				if (row != c1posy)
					GLCD_NJU6450A_row_set(0, row);


				if (x == (FONT_CHAR_WIDTH -1))
					data2write= 0x00;
				else
					{
					data2write= (line == 0) ? font2size_tab[*char_fontdta & 0x0F] : font2size_tab[*char_fontdta >> 4];
					char_fontdta++;
					}

				if (invert)
					data2write^= 0xFF;

				GLCD_NJU6450A_data_wr(0, data2write);
				GLCD_NJU6450A_data_wr(0, data2write);

				c1posx+= 2;
				column_t+= 2;

				} // CHIP1 Left
			else
				{
				// CHIP2 Right

				if (column_t != c2posx)
					GLCD_NJU6450A_column_set(column_t);

				if (row != c2posy)
					GLCD_NJU6450A_row_set(1, row);


				if (x == (FONT_CHAR_WIDTH -1))
					data2write= 0x00;
				else
					{
					data2write= (line == 0) ? font2size_tab[*char_fontdta & 0x0F] : font2size_tab[*char_fontdta >> 4];
					char_fontdta++;
					}

				if (invert)
					data2write^= 0xFF;

				GLCD_NJU6450A_data_wr(1, data2write);
				GLCD_NJU6450A_data_wr(1, data2write);

				c2posx+= 2;
				column_t+= 2;

				} // CHIP2 Right

			} // for (x=0;x<FONT_CHAR_WIDTH;x++)


		row += 1;
		line+= 1;

		} // while (line < 2)


	}

//------------------------------------------------------------------------------

void GLCD_NJU6450A_region_fill(unsigned char posx, unsigned char posy, unsigned char width, unsigned char height, unsigned char data)
	{
	unsigned char x;
	unsigned char posx_end, posy_end;

	posx_end= posx + width;
	posy_end= posy + height;

	if ((posx >= SCREEN_WIDTH) || (posx_end > SCREEN_WIDTH))
		return;

	if ((posy >= (SCREEN_HEIGHT / SCREEN_LINES_PER_ROW)) || (posy_end > (SCREEN_HEIGHT / SCREEN_LINES_PER_ROW)))
		return;

	while (posy < posy_end)
		{
		x= posx;

		while (x < posx_end)
			{

			if (x < SUBSCREEN_WIDTH)
				{
				// CHIP1 Left

				if (x != c1posx)
					GLCD_NJU6450A_column_set(x);

				if (posy != c1posy)
					GLCD_NJU6450A_row_set(0, posy);

				c1posx+= 1;

				GLCD_NJU6450A_data_wr(0, data);
				}
			else
				{
				// CHIP2 Right

				if (x != c2posx)
					GLCD_NJU6450A_column_set(x);

				if (posy != c2posy)
					GLCD_NJU6450A_row_set(1, posy);

				c2posx+= 1;

				GLCD_NJU6450A_data_wr(1, data);
				}

			x+= 1;

			} // while (x < posx_end)

		posy++;

		} // while (posy < posy_end)


	}

//------------------------------------------------------------------------------

void GLCD_NJU6450A_bmp_draw(unsigned char posx, unsigned char posy, unsigned char width, unsigned char height, unsigned char attr, unsigned char *bmpptr)
	{
	unsigned char posx_end, posy_end;
	unsigned char x, y, data;

	c1posx= 0xFF; // ?
	c1posy= 0xFF; // ?

	c2posx= 0xFF; // ?
	c2posy= 0xFF; // ?

	posx_end= posx + width;
	posy_end= posy + height;


	if (bmpptr == NULL)
		return;

	if ((posx >= SCREEN_WIDTH) || (posx_end > SCREEN_WIDTH))
		return;

	if ((posy >= (SCREEN_HEIGHT / SCREEN_LINES_PER_ROW)) || (posy_end > (SCREEN_HEIGHT / SCREEN_LINES_PER_ROW)))
		return;



	for (y=posy;y<posy_end;y++)
		{

		if ((posx < SUBSCREEN_WIDTH) && (y != c1posy))
			GLCD_NJU6450A_row_set(0, y);

		if ((posx_end > SUBSCREEN_WIDTH) && (y != c2posy))
			GLCD_NJU6450A_row_set(1, y);

		for (x=posx;x<posx_end;x++)
			{

			data= *bmpptr;

			if (attr & TPGUI_ITEM_ATTRIB_INVERT)
				data^= 0xFF;

			if (x < SUBSCREEN_WIDTH)
				{
				// CHIP1 Left

				if (x != c1posx)
					GLCD_NJU6450A_column_set(x);

				c1posx+= 1;
				GLCD_NJU6450A_data_wr(0, data);
				}
            else
				{
				// CHIP2 Right

				if (x != c2posx)
					GLCD_NJU6450A_column_set(x);

				c2posx+= 1;
				GLCD_NJU6450A_data_wr(1, data);
				}

			bmpptr++;

			} // for (x=posx;x<posx_end;x++)

		} // for (y=posy;y<posy_end;y++)
	

	}

//------------------------------------------------------------------------------

void Delay (uint32_t nCount)
	{
	for(; nCount != 0; nCount--);
	}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void GLCD_NJU6450A_contrast_set(uint8_t value)
	{
	TIM_OCInitTypeDef TIM_OCInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	float duty_f;

	if (value > 100)
		value= 100;

	if (value == 0)
		{
		// PWM OFF
		GPIO_InitStructure.GPIO_Pin= LCD_PWM_CONTRAST;
		GPIO_InitStructure.GPIO_Speed= GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode= GPIO_Mode_Out_PP;
		GPIO_Init(LCD_PWM_GPIO, &GPIO_InitStructure);
		GPIO_ResetBits(LCD_PWM_GPIO, LCD_PWM_CONTRAST);
		}
	else
	if (value == 100)
		{
		// static ON
		GPIO_InitStructure.GPIO_Pin= LCD_PWM_CONTRAST;
		GPIO_InitStructure.GPIO_Speed= GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode= GPIO_Mode_Out_PP;
		GPIO_Init(LCD_PWM_GPIO, &GPIO_InitStructure);
		GPIO_SetBits(LCD_PWM_GPIO, LCD_PWM_CONTRAST);
		}
	else
		{
		duty_f= (float)LCD_PWM_PERIOD * (float)value / 100;

		// Output Compare Timing Mode configuration: Channel1
		TIM_OCStructInit(&TIM_OCInitStructure);
		TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
		TIM_OCInitStructure.TIM_OutputState= TIM_OutputState_Enable;
		TIM_OCInitStructure.TIM_Pulse= (uint16_t)duty_f;
		TIM_OCInitStructure.TIM_OCPolarity= TIM_OCPolarity_High;

		TIM_OC2Init(LCD_PWM_TIMER, &TIM_OCInitStructure); // CH2
		TIM_OC2PreloadConfig(LCD_PWM_TIMER, TIM_OCPreload_Enable);

		GPIO_InitStructure.GPIO_Pin= LCD_PWM_CONTRAST;
		GPIO_InitStructure.GPIO_Speed= GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode= GPIO_Mode_AF_PP;
		GPIO_Init(LCD_PWM_GPIO, &GPIO_InitStructure);
		}

	}

//------------------------------------------------------------------------------


void GLCD_NJU6450A_backlight_set(uint8_t value)
	{

	TIM_OCInitTypeDef TIM_OCInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	float duty_f;

	if (value > 100)
		value= 100;

	if (value == 0)
		{
		// PWM OFF
		GPIO_InitStructure.GPIO_Pin= LCD_PWM_BACKLIGHT;
		GPIO_InitStructure.GPIO_Speed= GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode= GPIO_Mode_Out_PP;
		GPIO_Init(LCD_PWM_GPIO, &GPIO_InitStructure);
		GPIO_ResetBits(LCD_PWM_GPIO, LCD_PWM_BACKLIGHT);
		}
	else
	if (value == 100)
		{
		// static ON
		GPIO_InitStructure.GPIO_Pin= LCD_PWM_BACKLIGHT;
		GPIO_InitStructure.GPIO_Speed= GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode= GPIO_Mode_Out_PP;
		GPIO_Init(LCD_PWM_GPIO, &GPIO_InitStructure);
		GPIO_SetBits(LCD_PWM_GPIO, LCD_PWM_BACKLIGHT);
		}
	else
		{
		duty_f= (float)LCD_PWM_PERIOD * (float)value / 100;

		// Output Compare Timing Mode configuration: Channel1
		TIM_OCStructInit(&TIM_OCInitStructure);
		TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
		TIM_OCInitStructure.TIM_OutputState= TIM_OutputState_Enable;
		TIM_OCInitStructure.TIM_Pulse= (uint16_t)duty_f;
		TIM_OCInitStructure.TIM_OCPolarity= TIM_OCPolarity_High;

		TIM_OC1Init(LCD_PWM_TIMER, &TIM_OCInitStructure); // CH2
		TIM_OC1PreloadConfig(LCD_PWM_TIMER, TIM_OCPreload_Enable);

		GPIO_InitStructure.GPIO_Pin= LCD_PWM_BACKLIGHT;
		GPIO_InitStructure.GPIO_Speed= GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode= GPIO_Mode_AF_PP;
		GPIO_Init(LCD_PWM_GPIO, &GPIO_InitStructure);
		}

	}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

