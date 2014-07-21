#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#define KEY_ACTION_NONE						0x00
#define KEY_ACTION_SHORT_PRESSED			0x01
#define KEY_ACTION_LONG_PRESSED				0x02
#define KEY_ACTION_REPEATEDLY_PRESSED		0x03


typedef struct
	{
	uint8_t key_code;
	uint8_t key_action;

	} key_s;


//------------------------------------------------------------------------------

void keyboard_init();
void keyboard_run();

void keyboard_key_add(uint8_t key_code, uint32_t peripheral_addr, uint16_t key_pin);
bool keyboard_key_get(key_s *key);


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#endif // __KEYBOARD_H__
