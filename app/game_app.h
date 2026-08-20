#ifndef _GAME_APP_H
#define _GAME_APP_H
#include "menu.h"          // 需要 page_state_t

/******函数声明*****/
void GameApp_Init(void);
void GameApp_Draw(page_state_t page);
page_state_t GameApp_HandleKey(uint8_t key, page_state_t currentPage);

#endif
