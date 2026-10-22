#ifndef __BATTERY_H
#define __BATTERY_H

#include "stm32f10x.h"
#include "power_if.h"

extern const power_driver_t battery_driver;

void Battery_ADC_Init(void);
float Battery_Get_Voltage(void);

#endif
