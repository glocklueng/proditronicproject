/*
 * cmd_interpreter.c
 *
 *  Created on: 05-12-2013
 *      Author: Tomek
 */

//-----------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "wdlist.h"
#include "serial_port.h"
#include "ksystem.h"

#include "cmd_interpreter.h"

//-----------------------------------------------------------------------------

unsigned char *cmd_buff_ptr;
unsigned char cmd_pos;

wdlist_s cmdInterpreter_list;

unsigned char *cmd_inter_strtok_del= " -:\t";


//-----------------------------------------------------------------------------

typedef struct
	{

	k_uchar *cmdstr;
	void (*funcptr)(unsigned char *param);

	} cmd_func_s;


//-----------------------------------------------------------------------------

int getCmdLine(unsigned char *cmd_buff);
void executeCmdLine(unsigned char *cmd_buff);


//-----------------------------------------------------------------------------

void cmd_interpreter_init()
	{
	wdlist_init(&cmdInterpreter_list);
	}

//-----------------------------------------------------------------------------

void cmd_interpreter_cmd_append(k_uchar *cmdstr, void *cmdfunc)
	{
	cmd_func_s *cmd_func;

	if (!cmdstr || !cmdfunc)
		return;

	cmd_func= (cmd_func_s *)malloc(sizeof(cmd_func_s));

	cmd_func->cmdstr= (k_uchar *)malloc(strlen(cmdstr)+1);
	strcpy(cmd_func->cmdstr, cmdstr);
	cmd_func->funcptr= cmdfunc;

	wdlist_append(&cmdInterpreter_list, (void *)cmd_func);
	}

//-----------------------------------------------------------------------------


void prvCmdInterpreterTask(void *pvParameters)
	{
	unsigned char cmd_buff[CONSOLE_BUFFER_SIZE];
	unsigned char *tokptr;

	msleep(3000);

	cmd_buff_ptr= cmd_buff;
	cmd_pos= 0;

	while (1)
		{

		if (getCmdLine(cmd_buff) != 1)
			continue;

		tokptr= strtok(cmd_buff, cmd_inter_strtok_del);

		if (tokptr)
			executeCmdLine(tokptr);

		} // while (1)

	}

//-----------------------------------------------------------------------------

int getCmdLine(unsigned char *cmd_buff)
	{
	int result;
	int x;

	if (!cmd_buff)
		return -1;


	if ((CONSOLE_BUFFER_SIZE - cmd_pos) <= 1)
		{
		cmd_pos= CONSOLE_BUFFER_SIZE - 1;
		cmd_buff_ptr= &cmd_buff[cmd_pos];
		}

	result= serial_port_read(1, cmd_buff_ptr, CONSOLE_BUFFER_SIZE - cmd_pos);

	if (((CONSOLE_BUFFER_SIZE - cmd_pos) == 1) && (result == 1))
		result= 0xFFFF;

	switch (result)
		{

		case -1:
			{
			cmd_pos= 0;
			cmd_buff_ptr= cmd_buff;
			break;
			}

		case 0:
			{
			cmd_pos= 0;
			cmd_buff_ptr= cmd_buff;
			break;
			}

		default:
			{

			if (result == 0xFFFF)
				{
				// ostatni znak
				if (*cmd_buff_ptr == 0x7F)
					{
					cmd_pos-= 1;
					cmd_buff_ptr-= 1;
					}
				else
				if (*cmd_buff_ptr == 0x0D)
					{
					*cmd_buff_ptr= 0x00;
					result= 0x0D0D;
					}
				}
			else
				{

				printf("%c", *cmd_buff_ptr);
				fflush(stdout);

				for (x=0;x<result;x++)
					{

					if (*cmd_buff_ptr == 0x7F)
						{
						if (cmd_pos != 0)
							{
							cmd_pos-= 1;
							cmd_buff_ptr-= 1;
							}
						}
					else
					if (*cmd_buff_ptr == 0x0D)
						{
						*cmd_buff_ptr= 0x00;
						result= 0x0D0D;
						break;
						}
					else
						{
						cmd_pos+= 1;
						cmd_buff_ptr+= 1;
						}

					} // for (x=0;x<result;x++)
				}

			break;
			}

		} // switch (result)

	if (result == 0x0D0D)
		{
		cmd_buff_ptr= cmd_buff;
		cmd_pos= 0;

		return 1;
		}
	else
		return 0;

	}

//-----------------------------------------------------------------------------

void executeCmdLine(unsigned char *cmd_buff)
	{
	wdlist_entry_s *entry;
	cmd_func_s *cmd_func_tmp;
	cmd_func_s *cmd_func_ptr= NULL;
	unsigned char *tokptr;

	entry= cmdInterpreter_list.first_entry;
	while (entry)
		{
		cmd_func_tmp= (cmd_func_s *)entry->data;

		if (!strcmp(cmd_func_tmp->cmdstr, cmd_buff))
			{
			cmd_func_ptr= cmd_func_tmp;
			break;
			}

		entry= entry->next;
		}

	if (cmd_func_ptr)
		{
		tokptr= strtok(NULL, cmd_inter_strtok_del);
		cmd_func_ptr->funcptr(tokptr);
		}

	}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

