#ifndef __GLCD_NJU6450A_H__
#define __GLCD_NJU6450A_H__

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#define SCREEN_WIDTH			122
#define SCREEN_HEIGHT			32
#define SCREEN_LINES_PER_ROW	8

#define GLCD_X_OFFSET			1
#define GLCD_Y_OFFSET			0

#define SUBSCREEN_WIDTH			61


//------------------------------------------------------------------------------

void GLCD_NJU6450A_init();
void GLCD_NJU6450A_screen_clear();


void GLCD_NJU6450A_char_draw(unsigned char code, unsigned char column, unsigned char row, unsigned char attr);




void GLCD_NJU6450A_region_fill(unsigned char posx, unsigned char posy, unsigned char width, unsigned char height, unsigned char data);

//------------------------------------------------------------------------------

#endif // __GLCD_NJU6450A_H__

