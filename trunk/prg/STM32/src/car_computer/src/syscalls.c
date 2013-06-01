/**************************************************************************//*****
 * @file     stdio.c
 * @brief    Implementation of newlib syscall
 ********************************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/unistd.h>

#include "stm32f10x.h"
#include "serial_port.h"
#include "platform_config.h"


#undef errno
extern int errno;
extern int  _end;

caddr_t _sbrk ( int incr )
{
  static unsigned char *heap = NULL;
  unsigned char *prev_heap;

  if (heap == NULL) {
    heap = (unsigned char *)&_end;
  }
  prev_heap = heap;

  heap += incr;

  return (caddr_t) prev_heap;
}
/*
int link(char *old, char *new) {
return -1;
}
*/
int _close(int file)
{
  return -1;
}

int _fstat(int file, struct stat *st)
{
  st->st_mode = S_IFCHR;
  return 0;
}

int _isatty(int file)
{
  return 1;
}

int _lseek(int file, int ptr, int dir)
{
  return 0;
}

int _read(int file, char *ptr, int len)
{
  return 0;
}

int _write(int file, char *ptr, int len)
	{
	int n;

	if (!ptr || (len < 1))
		return -1;

	switch (file)
		{

		case STDOUT_FILENO: //stdout
		case STDERR_FILENO: //stderr
			{
			len= serial_port_write(CONSOLE_USART, ptr, len);
			break;
			}

		default:
			{
			len= -1;
			break;
			}

		} // switch (file)

	return len;
	}

void abort(void)
{
  /* Abort called */
  while(1);
}
          
/* --------------------------------- End Of File ------------------------------ */
