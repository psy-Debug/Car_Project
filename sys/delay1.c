#include "delay1.h"

__IO uint32_t ulTicks;

static uint8_t delay_initialized_flag = 0;
static float us_per_mini_tick;

void Delay_Init(void)
{
	if(!delay_initialized_flag)
	{
		delay_initialized_flag = 1;

		RCC_ClocksTypeDef clockinfo = {0};
		uint32_t tmp;

		SysTick->CTRL &= ~SysTick_CTRL_ENABLE;

		ulTicks = 0;

		RCC_GetClocksFreq(&clockinfo);

		SysTick->CTRL |= SysTick_CTRL_TICKINT;

		SCB->SHP[7] = 0;

		tmp =  clockinfo.HCLK_Frequency / 1000;
		if(tmp > 0x00ffffff)
		{
			tmp = tmp / 8;
			SysTick->CTRL &= ~SysTick_CTRL_CLKSOURCE;
		}
		else
		{
			SysTick->CTRL |= SysTick_CTRL_CLKSOURCE;
		}
		SysTick->LOAD = tmp - 1;

		SysTick->CTRL |= SysTick_CTRL_ENABLE;

		us_per_mini_tick = 1000.0 / ((SysTick->LOAD & 0x00ffffff) + 1);
	}
}

void Delay(uint32_t Delay)
{
	Delay_Init();

	uint64_t expiredTime = ulTicks + Delay;

	while(ulTicks <  expiredTime){}
}

uint32_t GetTick(void)
{
	Delay_Init();

	return ulTicks;
}

uint64_t GetUs(void)
{
	Delay_Init();

	uint64_t tick;
	uint32_t mini_tick;

	__disable_irq();

	while(1)
	{
		tick = ulTicks;
		mini_tick = SysTick->VAL;

		if(SysTick->CTRL & SysTick_CTRL_COUNTFLAG)
		{
			ulTicks++;
		}
		else
		{
			break;
		}
	}

	__enable_irq();

	tick *= 1000;
	tick += (uint32_t)((SysTick->LOAD - mini_tick) * us_per_mini_tick);

	return tick;
}

void DelayUs(uint32_t us)
{
	Delay_Init();

	uint64_t expiredTime = GetUs() + us + 1;

	while(GetUs() < expiredTime);
}
