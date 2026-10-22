#ifndef _DELAY_H_
#define _DELAY_H_

#include "stm32f10x.h"

void Delay_Init(void);
void Delay(uint32_t ms);
uint32_t GetTick(void);
uint64_t GetUs(void);
void DelayUs(uint32_t us);

#endif
