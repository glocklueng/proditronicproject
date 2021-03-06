#ifndef __LCD_INTERFACE_H__
#define __LCD_INTERFACE_H__

#include <stdint.h>

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#define TPGUI_ITEM_ATTRIB_INVERT		0x01
#define TPGUI_ITEM_ATTRIB_BLINKING		0x02
#define TPGUI_ITEM_ATTRIB_DOUBLESIZE	0x04


typedef struct
	{

	void (*init)();
	void (*screen_clear)();
	void (*display_on)();
	void (*display_off)();

	void (*char_draw)(unsigned char code, unsigned char column, unsigned char row, unsigned char attr);
	void (*region_fill)(unsigned char posx, unsigned char posy, unsigned char width, unsigned char height, unsigned char data);
	void (*bmp_draw)(unsigned char posx, unsigned char posy, unsigned char width, unsigned char height, unsigned char attr, unsigned char *bmpptr);

	void (*contrast_set)(uint8_t value);
	void (*backlight_set)(uint8_t value);


	unsigned char width;
	unsigned char height;

	unsigned char cols;
	unsigned char rows;

	} lcd_handler_s;



//------------------------------------------------------------------------------

#endif // __LCD_INTERFACE_H__

