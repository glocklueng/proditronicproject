#include "stm32f10x.h"


#include <FreeRTOS.h>
#include <task.h>
#include "semphr.h"


#include "ksystem.h"
#include "wdlist.h"

//------------------------------------------------------------------------------

#define KTIMER_INTERVAL			10 // usec

//------------------------------------------------------------------------------

xSemaphoreHandle ktimer_sem;
wdlist_s ktimer_timer_list;
wdlist_s ktimer_oneshot_timer_list;

TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure_ktimerlst;
ktimerlst_spec_s *ktimerlst_spec_ptr= NULL;


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

inline void msleep(int msec)
	{
	vTaskDelay(msec / portTICK_RATE_MS);
	}

//------------------------------------------------------------------------------

void usleep_sthr(int usec)
	{
	}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/*
void ktimer_init()
	{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;

	uint16_t PrescalerValue;

	vSemaphoreCreateBinary(ktimer_sem);
	wdlist_init(&ktimer_timer_list);
	wdlist_init(&ktimer_oneshot_timer_list);


// ustaw timer

	PrescalerValue= (uint16_t)(SystemCoreClock / 100000) - 1; // 1MHz

	// TIM2 configuration
	TIM_TimeBaseStructure.TIM_Period = KTIMER_INTERVAL-1; // 10us
	TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);


	// Output Compare Timing Mode configuration: Channel1
	TIM_OCStructInit(&TIM_OCInitStructure);
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
	TIM_OCInitStructure.TIM_Pulse = 0x0;
	TIM_OC1Init(TIM2, &TIM_OCInitStructure);

	// Immediate load of TIM2,TIM3 and TIM4 Prescaler values
	TIM_PrescalerConfig(TIM2, PrescalerValue, TIM_PSCReloadMode_Immediate);

	// Clear TIM2, TIM3 and TIM4 update pending flags
	TIM_ClearFlag(TIM2, TIM_FLAG_Update);

	// Enable TIM2, TIM3 and TIM4 Update interrupts
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

	// TIM2, TIM3 and TIM4 enable counters
	TIM_Cmd(TIM2, ENABLE);

	}

//------------------------------------------------------------------------------

void ktimer_create(ktimer_spec_s *ktimer_spec)
	{

	if (!ktimer_spec)
		return;

	ktimer_spec->active= 0x01;
	ktimer_spec->remain_usec= ktimer_spec->value_usec;

	xSemaphoreTake(ktimer_sem, 0);

	if (ktimer_spec->interval_usec == 0)
		wdlist_append(&ktimer_oneshot_timer_list, (void *)ktimer_spec);
	else
		wdlist_append(&ktimer_timer_list, (void *)ktimer_spec);

	xSemaphoreGive(ktimer_sem);

	}

//------------------------------------------------------------------------------

void TIM2_IRQHandler_xxx(void)
	{
	static wdlist_entry_s *wdlist_entry;
	static wdlist_entry_s *wdlist_entry_next;
	static ktimer_spec_s *ktimer_spec;
	static signed portBASE_TYPE xHigherPriorityTaskWoken= pdFALSE;


	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);


	xSemaphoreTakeFromISR(ktimer_sem, &xHigherPriorityTaskWoken);


// one shot timer

	wdlist_entry= ktimer_oneshot_timer_list.first_entry;
	while (wdlist_entry)
		{
		wdlist_entry_next= wdlist_entry->next;
		ktimer_spec= (ktimer_spec_s *)wdlist_entry->data;

		if (ktimer_spec->remain_usec > KTIMER_INTERVAL)
			ktimer_spec->remain_usec-= KTIMER_INTERVAL;
		else
			{
			// time elapsed

			if (ktimer_spec->callback != NULL)
				ktimer_spec->callback(ktimer_spec->callback_param);

			wdlist_entry->data= NULL;
			wdlist_delete(&ktimer_oneshot_timer_list, wdlist_entry);
			}

		wdlist_entry= wdlist_entry_next;
		}


// interval timer

	wdlist_entry= ktimer_timer_list.first_entry;
	while (wdlist_entry)
		{
		wdlist_entry_next= wdlist_entry->next;
		ktimer_spec= (ktimer_spec_s *)wdlist_entry->data;

		if (ktimer_spec->active)
			{

			if (ktimer_spec->remain_usec > KTIMER_INTERVAL)
				ktimer_spec->remain_usec-= KTIMER_INTERVAL;
			else
				{
				// time elapsed

				if (ktimer_spec->callback)
					ktimer_spec->callback(ktimer_spec->callback_param);

				ktimer_spec->remain_usec= ktimer_spec->interval_usec;

				if (ktimer_spec->nrepeat > 1)
					ktimer_spec->nrepeat-= 1;
				else
				if (ktimer_spec->nrepeat == 1)
					{
					wdlist_entry->data= NULL;
					wdlist_delete(&ktimer_timer_list, wdlist_entry);
					}
                    
				}


			} // if (ktimer_spec->active)

		wdlist_entry= wdlist_entry_next;
		}


	xSemaphoreGiveFromISR(ktimer_sem, &xHigherPriorityTaskWoken);

	}
*/
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void ktimerlst_init()
	{

	TIM_OCInitTypeDef TIM_OCInitStructure;
	uint16_t PrescalerValue;


// ustaw timer

	PrescalerValue= (uint16_t)(SystemCoreClock / 100000) - 1; // 1MHz

	// TIM2 configuration
	TIM_TimeBaseStructure_ktimerlst.TIM_Period= 65000; // nie ma znaczenia, ustawiane w ktimerlst_create
	TIM_TimeBaseStructure_ktimerlst.TIM_Prescaler= PrescalerValue;
	TIM_TimeBaseStructure_ktimerlst.TIM_ClockDivision= 0;
	TIM_TimeBaseStructure_ktimerlst.TIM_CounterMode= TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure_ktimerlst);


	// Immediate load of TIM2,TIM3 and TIM4 Prescaler values
	TIM_PrescalerConfig(TIM2, PrescalerValue, TIM_PSCReloadMode_Immediate);


	// Output Compare Timing Mode configuration: Channel1
	TIM_OCStructInit(&TIM_OCInitStructure);
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
	TIM_OCInitStructure.TIM_Pulse = 0x0;
	TIM_OC1Init(TIM2, &TIM_OCInitStructure);


	TIM_SelectOnePulseMode(TIM2, TIM_OPMode_Single);


	// Enable TIM2, TIM3 and TIM4 Update interrupts
	TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);

	// TIM2, TIM3 and TIM4 enable counters
	TIM_Cmd(TIM2, DISABLE);

	}

//------------------------------------------------------------------------------

void ktimerlst_create(ktimerlst_spec_s *ktimerlst_spec)
	{
	int x;

	ktimerlst_spec_ptr= ktimerlst_spec;

	if (!ktimerlst_spec)
		return;

	if ((ktimerlst_spec_ptr->nrepeat < 1) || (ktimerlst_spec_ptr->nrepeat > KTIMERLST_NREPEAT_MAX))
		return ;

	for (x=0;x<ktimerlst_spec_ptr->nrepeat;x++)
		if (ktimerlst_spec->value_usec[x] == 0)
			ktimerlst_spec->value_usec[x]= 10;


	ktimerlst_spec_ptr->phase= 0;


	TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE); // !!!


	// TIM2 configuration
	TIM_TimeBaseStructure_ktimerlst.TIM_Period= ktimerlst_spec->value_usec[0];
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure_ktimerlst);

	// Clear TIM2, TIM3 and TIM4 update pending flags
	TIM_ClearFlag(TIM2, TIM_FLAG_Update);

	// Enable TIM2, TIM3 and TIM4 Update interrupts
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE); // !!!

	// TIM2, TIM3 and TIM4 enable counters
	TIM_Cmd(TIM2, ENABLE);

	}


//------------------------------------------------------------------------------

void TIM2_IRQHandler(void)
	{

	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);


	ktimerlst_spec_ptr->callback(ktimerlst_spec_ptr);
	ktimerlst_spec_ptr->phase++;


	if (ktimerlst_spec_ptr->phase < ktimerlst_spec_ptr->nrepeat)
		{

		TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE); // !!!

		// TIM2 configuration
		TIM_TimeBaseStructure_ktimerlst.TIM_Period= ktimerlst_spec_ptr->value_usec[ktimerlst_spec_ptr->phase];
		TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure_ktimerlst);

		// Clear TIM2, TIM3 and TIM4 update pending flags
		TIM_ClearFlag(TIM2, TIM_FLAG_Update);

		// Enable TIM2, TIM3 and TIM4 Update interrupts
		TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE); // !!!

		// TIM2, TIM3 and TIM4 enable counters
		TIM_Cmd(TIM2, ENABLE);

		}


	}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

