#ifndef _TIME_APP_H
#define _TIME_APP_H
#include "menu.h"          // 需要 page_state_t

/******函数声明*****/
void TimeApp_Init(void);
void TimeApp_Draw(page_state_t page);
page_state_t TimeApp_HandleKey(uint8_t key, page_state_t currentPage);

#endif
