#include "encoder.h"
#include "delay1.h"

static volatile int64_t encoder_l = 0;
static volatile int64_t encoder_r = 0;
static volatile int8_t direction_l = 1;
static volatile int8_t direction_r = 1;
static volatile uint64_t t0_l = 0, t1_l = 0;
static volatile uint64_t t0_r = 0, t1_r = 0;

void App_Encoder_Init(void)
{
    Encoder_Init_L();
    Encoder_Init_R();
    uint64_t now = GetUs();
    t0_l = t1_l = now;
    t0_r = t1_r = now;
}

void Encoder_Init_L(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_ICInitTypeDef TIM_ICInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
    TIM_TimeBaseStructure.TIM_Prescaler = 71;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_BothEdge;
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStructure.TIM_ICFilter = 0x6;
    TIM_ICInit(TIM2, &TIM_ICInitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_ITConfig(TIM2, TIM_IT_CC1, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
}

void Encoder_Init_R(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_ICInitTypeDef TIM_ICInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
    TIM_TimeBaseStructure.TIM_Prescaler = 71;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_BothEdge;
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStructure.TIM_ICFilter = 0x6;
    TIM_ICInit(TIM3, &TIM_ICInitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_ITConfig(TIM3, TIM_IT_CC1, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
}

void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_CC1) != RESET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);
        t1_l = t0_l;
        t0_l = GetUs();
        uint8_t a = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);
        uint8_t b = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1);
        if((a == Bit_SET && b == Bit_RESET) || (a == Bit_RESET && b == Bit_SET))
        {
            encoder_l--;
            if(direction_l > 0) direction_l = -2;
            else direction_l = -1;
        }
        else
        {
            encoder_l++;
            if(direction_l < 0) direction_l = +2;
            else direction_l = 1;
        }
    }
}

void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_CC1) != RESET)
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_CC1);
        t1_r = t0_r;
        t0_r = GetUs();
        uint8_t a = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6);
        uint8_t b = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7);
        if((a == Bit_SET && b == Bit_RESET) || (a == Bit_RESET && b == Bit_SET))
        {
            encoder_r++;
            if(direction_r < 0) direction_r = +2;
            else direction_r = 1;
        }
        else
        {
            encoder_r--;
            if(direction_r > 0) direction_r = -2;
            else direction_r = -1;
        }
    }
}

float Encoder_Get_Speed_L(void)
{
    __disable_irq();
    int8_t direction_cpy = direction_l;
    uint64_t t0_cpy = t0_l;
    uint64_t t1_cpy = t1_l;
    __enable_irq();

    if(direction_cpy == +2 || direction_cpy == -2) return 0.0f;

    uint64_t now = GetUs();
    float T;

    if(now - t0_cpy > t0_cpy - t1_cpy)
        T = (float)(now - t0_cpy);
    else
        T = (float)(t0_cpy - t1_cpy);

    if (T > 500000.0f) return 0.0f;
    if (T < 1.0f) T = 1.0f;

    return (float)direction_cpy / T * ENCODER_CALC_CONST;
}

float Encoder_Get_Speed_R(void)
{
    __disable_irq();
    int8_t direction_cpy = direction_r;
    uint64_t t0_cpy = t0_r;
    uint64_t t1_cpy = t1_r;
    __enable_irq();

    if(direction_cpy == +2 || direction_cpy == -2) return 0.0f;

    uint64_t now = GetUs();
    float T;

    if(now - t0_cpy > t0_cpy - t1_cpy)
        T = (float)(now - t0_cpy);
    else
        T = (float)(t0_cpy - t1_cpy);

    if (T > 500000.0f) return 0.0f;
    if (T < 1.0f) T = 1.0f;

    return (float)direction_cpy / T * ENCODER_CALC_CONST;
}

float Encoder_Get_angle_L(void)
{
    return (float)encoder_l / 13.0f / (244904.0f/12000.0f) * 360.0f;
}

float Encoder_Get_angle_R(void)
{
    return (float)encoder_r / 13.0f / (244904.0f/12000.0f) * 360.0f;
}

int64_t Encoder_Get_count_L(void) { return encoder_l; }
int64_t Encoder_Get_count_R(void) { return encoder_r; }

/* ---- interface implementation ---- */
static int encoder_init(void) {
    App_Encoder_Init();
    return 0;
}

static void encoder_get_left(encoder_channel_t *ch) {
    ch->speed = Encoder_Get_Speed_L();
    ch->angle = Encoder_Get_angle_L();
    ch->count = Encoder_Get_count_L();
}

static void encoder_get_right(encoder_channel_t *ch) {
    ch->speed = Encoder_Get_Speed_R();
    ch->angle = Encoder_Get_angle_R();
    ch->count = Encoder_Get_count_R();
}

const encoder_driver_t encoder_driver = {
    .init      = encoder_init,
    .get_left  = encoder_get_left,
    .get_right = encoder_get_right,
};
