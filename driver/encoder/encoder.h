#ifndef __ENCODER_H
#define __ENCODER_H

#include "stm32f10x.h"
#include "encoder_if.h"

#define ENCODER_CALC_CONST  (1.0f / 13.0f / (244904.0f/12000.0f) * 6.2831853f * 1000000.0f)

extern const encoder_driver_t encoder_driver;

void App_Encoder_Init(void);
void Encoder_Init_L(void);
void Encoder_Init_R(void);
float Encoder_Get_Speed_L(void);
float Encoder_Get_Speed_R(void);
float Encoder_Get_angle_L(void);
float Encoder_Get_angle_R(void);
int64_t Encoder_Get_count_L(void);
int64_t Encoder_Get_count_R(void);

#endif
