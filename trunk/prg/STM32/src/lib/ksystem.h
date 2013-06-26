#ifndef KSYSTEM_H_
#define KSYSTEM_H_

//------------------------------------------------------------------------------

#include "ktypes.h"

//------------------------------------------------------------------------------

typedef struct
	{
	unsigned char active;
	int value_usec;
	int interval_usec;
	void (*callback)(void *);
	void *callback_param;
	int nrepeat;	// liczba powtórzeñ, 0 - w nieskoñczonoœæ; dla interval_usec > 0

// private
	int remain_usec;

	} ktimer_spec_s;

//------------------------------------------------------------------------------

#define KTIMERLST_NREPEAT_MAX	10

typedef struct
	{

	int value_usec[KTIMERLST_NREPEAT_MAX];
	void (*callback)(void *);
	void *callback_param;
	int nrepeat;

// private
	int phase;

	} ktimerlst_spec_s;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------



void msleep(int msec);
void usleep_sthr(int usec); // jednoczeœnie wywo³ywaæ tylko z jednego w¹tku


void ktimer_init(); // timer TIM2, dla fCPU=72MHZ, minimalny KTIMER_INTERVAL to 10 us, przy ni¿szych nie wyrabia sie
void ktimer_create(ktimer_spec_s *ktimer_spec);

void ktimerlst_init();
void ktimerlst_create(ktimerlst_spec_s *ktimerlst_spec);


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#endif // KSYSTEM_H_

