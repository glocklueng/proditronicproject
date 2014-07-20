/*
 * cmd_interpreter.h
 *
 *  Created on: 05-12-2013
 *      Author: Tomek
 */

#ifndef CMD_INTERPRETER_H_
#define CMD_INTERPRETER_H_

//-----------------------------------------------------------------------------



#if defined (__STM32__)
	#include "stm32f10x.h"
#endif

#include "ktypes.h"



#define CONSOLE_BUFFER_SIZE		33




//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


void cmd_interpreter_init();
void cmd_interpreter_cmd_append(k_uchar *cmdstr, void *cmdfunc);


void prvCmdInterpreterTask(void *pvParameters);




//-----------------------------------------------------------------------------

#endif /* CMD_INTERPRETER_H_ */
