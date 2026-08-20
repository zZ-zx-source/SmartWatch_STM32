#ifndef _TIME_SETTING_H
#define _TIME_SETTING_H
#include "menu.h"          // 需要 page_state_t

/******函数声明*****/
void TimeSetting_Enter(void);
void TimeSetting_Draw(void);
page_state_t TimeSetting_HandleKey(uint8_t key);

#endif
