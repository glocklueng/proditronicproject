/*
 * rtc.c
 *
 *  Created on: 17-12-2013
 *      Author: Tomek
 */
//-----------------------------------------------------------------------------


#include "stm32f10x.h"

#include "ktypes.h"
#include "stm32rtc.h"

//-----------------------------------------------------------------------------

#define RCC_OFFSET                (RCC_BASE - PERIPH_BASE)
#define BDCR_OFFSET               (RCC_OFFSET + 0x20)
#define BDCR_ADDRESS              (PERIPH_BASE + BDCR_OFFSET)
#define RTCEN_BitNumber           0x0F

//-----------------------------------------------------------------------------

void stm32rtc_configure();

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

int stm32rtc_init()
	{

	if (!((*(__IO uint32_t *)BDCR_ADDRESS) & (1 << RTCEN_BitNumber)))
		{

		stm32rtc_configure();
		stm32rtc_set(0);

		}
	else
		{

		/* Enable PWR and BKP clocks */
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

		/* Allow access to BKP Domain */
		PWR_BackupAccessCmd(ENABLE);

		// Wait for RTC registers synchronization
		RTC_WaitForSynchro();

	    // Enable the RTC Second
	    RTC_ITConfig(RTC_IT_SEC, ENABLE);

	    // Wait until last write operation on RTC registers has finished
	    RTC_WaitForLastTask();

		}

	return 0;
	}

//-----------------------------------------------------------------------------

void stm32rtc_configure()
	{

	/* Enable PWR and BKP clocks */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

	/* Allow access to BKP Domain */
	PWR_BackupAccessCmd(ENABLE);

	/* Reset Backup Domain */
	BKP_DeInit();

	/* Enable LSE */
	RCC_LSEConfig(RCC_LSE_ON);
	/* Wait till LSE is ready */
	while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
	{}

	/* Select LSE as RTC Clock Source */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

	/* Enable RTC Clock */
	RCC_RTCCLKCmd(ENABLE);

	/* Wait for RTC registers synchronization */
	RTC_WaitForSynchro();

	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();

	/* Enable the RTC Second */
	RTC_ITConfig(RTC_IT_SEC, ENABLE);

	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();

	/* Set RTC prescaler: set RTC period to 1sec */
	RTC_SetPrescaler(32767); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */

	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();

	}

//-----------------------------------------------------------------------------

time_t stm32rtc_get()
	{
	return (time_t)RTC_GetCounter();
	}

//-----------------------------------------------------------------------------

void stm32rtc_set(time_t currtime)
	{
	// Wait until last write operation on RTC registers has finished
	RTC_WaitForLastTask();

	// Change the current time
	RTC_SetCounter((uint32_t)currtime);

	// Wait until last write operation on RTC registers has finished
	RTC_WaitForLastTask();
	}

//-----------------------------------------------------------------------------

void led_switch()
	{
	static char ledstate= 0;

	if (ledstate)
		{
		ledstate= 0;
		GPIO_ResetBits(GPIOB , GPIO_Pin_1);
		}
	else
		{
		ledstate= 1;
		GPIO_SetBits(GPIOB , GPIO_Pin_1);
		}

	}

//-----------------------------------------------------------------------------

void RTC_IRQHandler(void)
	{

	if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
		{
		/* Clear the RTC Second interrupt */
		RTC_ClearITPendingBit(RTC_IT_SEC);

		led_switch();

		/* Wait until last write operation on RTC registers has finished */
		RTC_WaitForLastTask();
		}
	}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


