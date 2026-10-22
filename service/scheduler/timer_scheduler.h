#ifndef __TIMER_SCHEDULER_H
#define __TIMER_SCHEDULER_H

#include "stm32f10x.h"

extern volatile uint8_t PID_Task_Flag;

void Timer4_Init(void);

#endif
