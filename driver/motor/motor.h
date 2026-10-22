#ifndef __MOTOR_CONTROL_H
#define __MOTOR_CONTROL_H

#include "stm32f10x.h"
#include "motor_if.h"

#define MOTOR_MAX_PWM  7200

extern const motor_driver_t motor_driver;

void Motor_Drive_Init(void);
void Motor_Set_Final_Output(int16_t speed_L, int16_t speed_R);

#endif
