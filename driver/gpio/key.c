#include "stm32f10x.h"
#include "delay.h"

void Key_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	GPIO_InitTypeDef GPIO_Initstructrue;
	GPIO_Initstructrue.GPIO_Mode=GPIO_Mode_IPU;
	GPIO_Initstructrue.GPIO_Pin=GPIO_Pin_1|GPIO_Pin_11;
	GPIO_Initstructrue.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_Initstructrue);
}

uint8_t Key_GetNum(void)
{
	uint8_t Key_Num=0;
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1)==0)
	{
		Delay_ms(20);
		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1)==0);
		Delay_ms(20);
		Key_Num=1;
	}
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11)==0)
	{
		Delay_ms(20);
		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11)==0);
		Delay_ms(20);
		Key_Num=2;
	}
	return Key_Num;
}
