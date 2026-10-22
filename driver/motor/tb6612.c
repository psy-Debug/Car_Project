#include "tb6612.h"

void TB6612_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_5;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_SetBits(GPIOA, GPIO_Pin_5);
}

void TB6612_Set_STBY(uint8_t state)
{
    if (state) GPIO_SetBits(GPIOA, GPIO_Pin_5);
    else       GPIO_ResetBits(GPIOA, GPIO_Pin_5);
}

void TB6612_Set_Dir_L(uint8_t dir)
{
    if (dir) {
        GPIO_SetBits(GPIOB, GPIO_Pin_13);
        GPIO_ResetBits(GPIOB, GPIO_Pin_12);
    } else {
        GPIO_ResetBits(GPIOB, GPIO_Pin_13);
        GPIO_SetBits(GPIOB, GPIO_Pin_12);
    }
}

void TB6612_Set_Dir_R(uint8_t dir)
{
    if (dir) {
        GPIO_SetBits(GPIOB, GPIO_Pin_5);
        GPIO_ResetBits(GPIOB, GPIO_Pin_14);
    } else {
        GPIO_ResetBits(GPIOB, GPIO_Pin_5);
        GPIO_SetBits(GPIOB, GPIO_Pin_14);
    }
}
