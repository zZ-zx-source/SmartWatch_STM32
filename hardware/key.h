#ifndef _KEY_H
#define _KEY_H
#include <stdint.h>

#define KEY1_PIN GPIO_Pin_13
#define KEY2_PIN GPIO_Pin_14
#define KEY3_PIN GPIO_Pin_15
#define KEY_PORT GPIOB

#define DEBOUNCE_MS 20  // 消抖时间
#define TICK_MS 10      // 定时中断周期

/******函数声明*****/
void Key_Init(void);
void Key_Tick(void);
uint8_t Key_Read(void);
uint8_t Key_ReadLongPress(void);

#endif
