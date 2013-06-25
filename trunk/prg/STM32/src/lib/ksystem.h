#ifndef KSYSTEM_H_
#define KSYSTEM_H_

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

typedef struct
	{

	unsigned char active;

	int value_usec;
	int interval_usec;

	void (*callback)(void *);
	void *callback_param;

	int nrepeat;	// liczba powt�rze�, 0 - w niesko�czono��; dla interval_usec > 0


// private

	int remain_usec;


	} ktimer_spec_s;



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------



void msleep(int msec);
void usleep_sthr(int usec); // jednocze�nie wywo�ywa� tylko z jednego w�tku

void ktimer_init();
void ktimer_create(ktimer_spec_s *ktimer_spec);



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#endif // KSYSTEM_H_

