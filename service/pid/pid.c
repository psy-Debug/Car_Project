#include "pid.h"
#include <math.h>

static float V_Kp = 0, V_Kd = 0;
static float S_Kp = 0, S_Ki = 0;
static float DeadZone_V = 0;
static float Gravity_K = 0;
static float Mech_Offset = 0;
static float Speed_Integral = 0;
#define SPEED_INT_LIMIT  4.0f

void PID_Init(void) {
    V_Kp = 0; V_Kd = 0;
    S_Kp = 0; S_Ki = 0;
    DeadZone_V = 0;
    Gravity_K = 0;
    Mech_Offset = 0;
    Speed_Integral = 0;
}

void PID_Set_Vertical_Kp(float val) { V_Kp = val * 0.00074f; }
void PID_Set_Vertical_Kd(float val) { V_Kd = val * 0.00010f; }
void PID_Set_Speed_Kp(float val)    { S_Kp = val * 0.001f; }
void PID_Set_Speed_Ki(float val)    { S_Ki = val * 0.002f; }
void PID_Set_DeadZone(float val)    { DeadZone_V = val * 0.003f; }
void PID_Set_Gravity_K(float val)   { Gravity_K = val * 0.005f; }
void PID_Set_Mech_Offset(float val) { Mech_Offset = val; }

float PID_Calc_Vertical(float Expect_Angle, float Real_Angle, float Gyro_X, float target_offset) {
    float Error = Expect_Angle - (Real_Angle - Mech_Offset - target_offset);
    float Kp_Current;

    if (fabs(Error) < 3.0f) {
        Kp_Current = V_Kp;
    } else {
        Kp_Current = V_Kp * 1.0f;
    }

    float D_Term = V_Kd * Gyro_X;
    if (Error * Gyro_X < 0) {
        D_Term *= 0.6f;
    } else {
        D_Term *= 1.0f;
    }

    if (D_Term > 4000) D_Term = 4000;
    if (D_Term < -4000) D_Term = -4000;

    return Kp_Current * Error + D_Term;
}

float PID_Calc_Speed(float Target_Speed, float Current_Speed) {
    float Error = Current_Speed - Target_Speed;

    Speed_Integral += Error;
    if (Speed_Integral > SPEED_INT_LIMIT) Speed_Integral = SPEED_INT_LIMIT;
    if (Speed_Integral < -SPEED_INT_LIMIT) Speed_Integral = -SPEED_INT_LIMIT;

    return S_Kp * Error + S_Ki * Speed_Integral;
}

float PID_Calc_Gravity(float Real_Angle) {
    float rad = (Real_Angle - Mech_Offset) * 0.017453f;
    return Gravity_K * sin(rad);
}

float PID_Calc_DeadZone_Compensate(float Voltage_In) {
    if (Voltage_In > 0.01f) {
        return Voltage_In + DeadZone_V;
    } else if (Voltage_In < -0.01f) {
        return Voltage_In - DeadZone_V;
    }
    return 0;
}

float kp(void) { return V_Kp; }
float kd(void) { return V_Kd; }
float ski(void) { return S_Ki; }
float skp(void) { return S_Kp; }
float G(void)  { return Gravity_K; }
float Z(void)  { return DeadZone_V; }
