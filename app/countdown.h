#ifndef _COUNTDOWN_H
#define _COUNTDOWN_H
#include "menu.h"          // 需要 page_state_t

/******函数声明*****/
void Countdown_Init(void);
void Countdown_Enter(void);
void Countdown_Draw(void);
page_state_t Countdown_HandleKey(uint8_t key);
void Countdown_SecondTick(void);
void Countdown_Tick10ms(void);

#endif
