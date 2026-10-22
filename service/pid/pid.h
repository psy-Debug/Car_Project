#ifndef __PID_H
#define __PID_H

#include "qmath.h"

void PID_Set_Vertical_Kp(float kp);
void PID_Set_Vertical_Kd(float kd);
void PID_Set_Speed_Kp(float kp);
void PID_Set_Speed_Ki(float ki);
void PID_Set_DeadZone(float voltage_val);
void PID_Set_Gravity_K(float k);
void PID_Set_Mech_Offset(float offset);

void PID_Init(void);

float PID_Calc_Vertical(float Expect_Angle, float Real_Angle, float Gyro_X, float target_offset);
float PID_Calc_Speed(float Target_Speed, float Current_Speed);
float PID_Calc_Gravity(float Real_Angle);
float PID_Calc_DeadZone_Compensate(float Voltage_In);
float kp(void);
float kd(void);
float G(void);
float Z(void);
float skp(void);
float ski(void);

#endif
