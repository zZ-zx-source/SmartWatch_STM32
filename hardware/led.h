#ifndef _LED_H
#define _LED_H
#include <stdint.h>

/**
 * LED 开关状态枚举
 * 硬件特性：LED 为低电平触发（0=亮，1=灭），
 */
typedef enum{
	LED_OFF = 0, //关闭（gpio输出高电平）
	LED_ON = 1   //开启（gpio输出低电平）
}LED_State_t;

/******函数声明*****/
void LED_Init(void);
void LED_SetState(LED_State_t state);
void LED_Turn(void);

#endif
