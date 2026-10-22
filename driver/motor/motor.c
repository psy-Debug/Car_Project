#include "motor.h"
#include "tb6612.h"
#include "motor_pwm.h"
#include <stdlib.h>

void Motor_Drive_Init(void)
{
    Motor_PWM_Init();
    TB6612_Init();
    TB6612_Set_STBY(1);
}

void Motor_Set_Final_Output(int16_t speed_L, int16_t speed_R)
{
    if (speed_L >= 0)
        TB6612_Set_Dir_L(1);
    else
        TB6612_Set_Dir_L(0);

    if (speed_R >= 0)
        TB6612_Set_Dir_R(1);
    else
        TB6612_Set_Dir_R(0);

    if (speed_L > MOTOR_MAX_PWM)  speed_L = MOTOR_MAX_PWM;
    if (speed_L < -MOTOR_MAX_PWM) speed_L = -MOTOR_MAX_PWM;
    if (speed_R > MOTOR_MAX_PWM)  speed_R = MOTOR_MAX_PWM;
    if (speed_R < -MOTOR_MAX_PWM) speed_R = -MOTOR_MAX_PWM;

    Motor_Set_PWM(speed_L, speed_R);
}

/* ---- interface implementation ---- */
static int motor_init(void) {
    Motor_Drive_Init();
    return 0;
}

static void motor_set_output(int16_t pwm_l, int16_t pwm_r) {
    Motor_Set_Final_Output(pwm_l, pwm_r);
}

const motor_driver_t motor_driver = {
    .init       = motor_init,
    .set_output = motor_set_output,
};
