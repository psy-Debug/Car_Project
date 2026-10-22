#include "motor_pwm.h"

void Motor_PWM_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 | RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    TIM_TimeBaseStructure.TIM_Period = 7199;
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;
    TIM_OCInitStructure.TIM_Pulse = 0;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

    TIM_OC1Init(TIM1, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_OC4Init(TIM1, &TIM_OCInitStructure);
    TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);

    TIM_CtrlPWMOutputs(TIM1, ENABLE);
    TIM_Cmd(TIM1, ENABLE);
}

void Motor_Set_PWM(int16_t pwm_l, int16_t pwm_r)
{
    if(pwm_l > PWM_MAX) pwm_l = PWM_MAX;
    if(pwm_l < -PWM_MAX) pwm_l = -PWM_MAX;
    if(pwm_r > PWM_MAX) pwm_r = PWM_MAX;
    if(pwm_r < -PWM_MAX) pwm_r = -PWM_MAX;

    TIM_SetCompare1(TIM1, (pwm_l >= 0) ? pwm_l : -pwm_l);
    TIM_SetCompare4(TIM1, (pwm_r >= 0) ? pwm_r : -pwm_r);
}
