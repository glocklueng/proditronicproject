#ifndef KSYSTEM_H_
#define KSYSTEM_H_

//------------------------------------------------------------------------------

typedef struct
	{

	unsigned char active;

	int value_usec;
	int interval_usec;

	void (*callback)(void);


// private

	int remain_usec;


	} ktimer_spec_s;



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------



void msleep(int msec);
void usleep_sthr(int usec); // jednoczeœnie wywo³ywaæ tylko z jednego w¹tku

void ktimer_init();
void ktimer_create(ktimer_spec_s *ktimer_spec);



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#endif // KSYSTEM_H_

