#ifndef __TB6612_H
#define __TB6612_H

#include "stm32f10x.h"

void TB6612_Init(void);
void TB6612_Set_STBY(uint8_t state);
void TB6612_Set_Dir_L(uint8_t dir);
void TB6612_Set_Dir_R(uint8_t dir);

#endif
