#ifndef __MOTOR_PWM_H
#define __MOTOR_PWM_H

#include "stm32f10x.h"

#define PWM_MAX 7199

void Motor_PWM_Init(void);
void Motor_Set_PWM(int16_t pwm_l, int16_t pwm_r);

#endif
