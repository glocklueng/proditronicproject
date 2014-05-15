
#include "font5x7V.h"
#include "lcd_interface.h"


#include "GLCD_NJU6450A.h"

//------------------------------------------------------------------------------

lcd_handler_s lcd=
	{
	.init= GLCD_NJU6450A_init,
	.screen_clear= GLCD_NJU6450A_screen_clear,
	.char_draw= GLCD_NJU6450A_char_draw,
	.region_fill= GLCD_NJU6450A_region_fill,





	};

//------------------------------------------------------------------------------

extern const unsigned char font_5x7_H_data[];

unsigned char c1posx, c1posy;
unsigned char c2posx, c2posy;

const unsigned char font2size_tab[16]= {
	0x00, 0x03, 0x0C, 0x0F,
	0x30, 0x33, 0x3C, 0x3F,
	0xC0, 0xC3, 0xCC, 0xCF,
	0xF0, 0xF3, 0xFC, 0xFF};




//------------------------------------------------------------------------------

void GLCD_NJU6450A_char1s_draw(unsigned char code, unsigned char column, unsigned char row, bool invert);
void GLCD_NJU6450A_char2s_draw(unsigned char code, unsigned char column, unsigned char row, bool invert);

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void GLCD_NJU6450A_init()
	{

	c1posx= 0xFF;
	c1posy= 0xFF;

	c2posx= 0xFF;
	c2posy= 0xFF;




	}

//------------------------------------------------------------------------------

void GLCD_NJU6450A_screen_clear()
	{

	c1posx= 0xFF;
	c1posy= 0xFF;

	c2posx= 0xFF;
	c2posy= 0xFF;

	}

//------------------------------------------------------------------------------

void GLCD_NJU6450A_position_set()
	{

	}

//------------------------------------------------------------------------------

void GLCD_NJU6450A_column_set(unsigned char column)
	{
	if (column < SUBSCREEN_WIDTH)
		;
	else
		; // minus SUBSCREEN_WIDTH
	}

//------------------------------------------------------------------------------

void GLCD_NJU6450A_row_set(unsigned char chip, unsigned char row)
	{
	if (chip == 0)
		;
	else
		;
	}

//------------------------------------------------------------------------------

void GLCD_NJU6450A_data_write(unsigned char chip, unsigned char data)
	{
	if (chip == 0)
		;
	else
		;
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

	if (column != 0)
		column*= FONT_CHAR_WIDTH;

	column+= GLCD_X_OFFSET;

	if ((code < 0x20) || (code > 0x7E)
		char_fontdta= font_5x7_V_data; // space
	else
		char_fontdta= &font_5x7_V_data[(code - 0x20) * FONT_DATA_SIZE];


	if (column < SUBSCREEN_WIDTH)
		{
		// CHIP1 Left

		if (column != c1posx)
			{
			GLCD_NJU6450A_column_set(column);
			c1posx= column;
			}

		if (row != c1posy)
			{
			GLCD_NJU6450A_row_set(0, row);
			c1posy= row;
			}

		if (!invert)
			{
			for (x=0;x<(FONT_CHAR_WIDTH-1);x++)
				GLCD_NJU6450A_data_write(0, *char_fontdta++);

			GLCD_NJU6450A_data_write(0, 0x00);
			}
		else
			{
			for (x=0;x<(FONT_CHAR_WIDTH-1);x++)
				GLCD_NJU6450A_data_write(0, 0xFF ^ *char_fontdta++);

			GLCD_NJU6450A_data_write(0, 0xFF);
			}

		c1posx+= FONT_CHAR_WIDTH;

		} // CHIP1 Left
	else
		{
		// CHIP2 Right

		if (column != c2posx)
			{
			GLCD_NJU6450A_column_set(column);
			c2posx= column;
			}

		if (row != c2posy)
			{
			GLCD_NJU6450A_row_set(1, row);
			c2posy= row;
			}

		if (!invert)
			{
			for (x=0;x<(FONT_CHAR_WIDTH-1);x++)
				GLCD_NJU6450A_data_write(1, *char_fontdta++);

			GLCD_NJU6450A_data_write(1, 0x00);
			}
		else
			{
			for (x=0;x<(FONT_CHAR_WIDTH-1);x++)
				GLCD_NJU6450A_data_write(1, 0xFF ^ *char_fontdta++);

			GLCD_NJU6450A_data_write(1, 0xFF);
			}

		c2posx+= FONT_CHAR_WIDTH;

		} // CHIP2 Right

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



	if ((code < 0x20) || (code > 0x7E)
		code= 0x00;
	else
		code-= 0x20


	while (line < 2)
		{

		char_fontdta= &font_5x7_V_data[code * FONT_DATA_SIZE];
		column_t= column;

		for (x=0;x<FONT_CHAR_WIDTH;x++)
			{
			
			if (column_t < SUBSCREEN_WIDTH)
				{
				// CHIP1 Left

				if (column_t != c1posx)
					{
					GLCD_NJU6450A_column_set(column_t);
					c1posx= column_t;
					}

				if (row != c1posy)
					{
					GLCD_NJU6450A_row_set(0, row);
					c1posy= row;
					}


				if (x == (FONT_CHAR_WIDTH -1))
					data2write= 0x00;
				else
					{
					data2write= (line == 0) ? font2size_tab[*char_fontdta & 0x0F] : font2size_tab[*char_fontdta >> 4];
					char_fontdta++;
					}

				GLCD_NJU6450A_data_write(0, data2write);
				GLCD_NJU6450A_data_write(0, data2write);

				c1posx+= 2;
				column_t+= 2;

				} // CHIP1 Left
			else
				{
				// CHIP2 Right

				if (column_t != c2posx)
					{
					GLCD_NJU6450A_column_set(column_t);
					c2posx= column_t;
					}

				if (row != c2posy)
					{
					GLCD_NJU6450A_row_set(1, row);
					c2posy= row;
					}


				if (x == (FONT_CHAR_WIDTH -1))
					data2write= 0x00;
				else
					{
					data2write= (line == 0) ? font2size_tab[*char_fontdta & 0x0F] : font2size_tab[*char_fontdta >> 4];
					char_fontdta++;
					}

				GLCD_NJU6450A_data_write(1, data2write);
				GLCD_NJU6450A_data_write(1, data2write);

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

	c1posx= 0xFF;
	c1posy= 0xFF;

	c2posx= 0xFF;
	c2posy= 0xFF;

	posx_end= posx + width;
	posy_end= posy + height;

	if ((posx >= SCREEN_WIDTH) || (posx_end >= SCREEN_WIDTH))
		return;

	if ((posy >= (SCREEN_HEIGHT / SCREEN_LINES_PER_ROW)) || (posy_end >= (SCREEN_HEIGHT / SCREEN_LINES_PER_ROW)))
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
					{
					GLCD_NJU6450A_column_set(x);
					c1posx= x;
					}

				if (posy != c1posy)
					{
					GLCD_NJU6450A_row_set(0, posy);
					c1posy= posy;
					}

				c1posx+= 1;

				GLCD_NJU6450A_data_write(0, data);
				}
			else
				{
				// CHIP2 Right

				if (x != c2posx)
					{
					GLCD_NJU6450A_column_set(x);
					c2posx= x;
					}

				if (posy != c2posy)
					{
					GLCD_NJU6450A_row_set(1, posy);
					c2posy= posy;
					}

				c2posx+= 1;

				GLCD_NJU6450A_data_write(1, data);
				}

			x+= 1;

			} // while (x < posx_end)

		posy++;

		} // while (posy < posy_end)


	c1posx= 0xFF;
	c1posy= 0xFF;

	c2posx= 0xFF;
	c2posy= 0xFF;

	}

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------


//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

