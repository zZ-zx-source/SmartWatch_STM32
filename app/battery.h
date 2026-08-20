#ifndef _BATTERY_H
#define _BATTERY_H
#include <stdint.h>

#define BATTERY_SAMPLE_INTERVAL_MS  100   //每100ms中断检测一次

/******函数声明*****/
void Battery_Init(void);
void Battery_Update(void);
uint8_t Battery_GetPercent(void);

#endif
