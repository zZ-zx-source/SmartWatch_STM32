#ifndef _ALARM_H
#define _ALARM_H
#include "menu.h"          // 需要 page_state_t

/******函数声明*****/
void Alarm_Init(void);
void Alarm_Enter(void);
void Alarm_Draw(void);
page_state_t Alarm_HandleKey(uint8_t key);
void Alarm_CheckSecondTick(void);

#endif
