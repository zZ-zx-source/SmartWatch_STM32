#include "stm32f10x.h"                  // Device header
#include "led.h"

void LED_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	GPIO_SetBits(GPIOB,GPIO_Pin_12);
}

/**
 * 控制 LED (PB12) 亮灭(低电平触发)
 * state:  LED_ON（0 = 灭）和LED_OFF（1 = 亮 ）
 */
void LED_SetState(LED_State_t state)
{
	if(state == LED_ON)
	{
		GPIO_ResetBits(GPIOB,GPIO_Pin_12);
	}
	else
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_12);
	}
}

/*翻转 LED (PB12) 的亮灭状态*/
void LED_Turn(void)
{
	if(GPIO_ReadOutputDataBit(GPIOB,GPIO_Pin_12) == 0)
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_12);
	}
	else
	{
		GPIO_ResetBits(GPIOB,GPIO_Pin_12);
	}
}
