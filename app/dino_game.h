#ifndef _DINO_GAME_H
#define _DINO_GAME_H
#include "menu.h"          // 需要 page_state_t

/* ---------------- 常量定义 ---------------- */
#define DINO_X          0
#define DINO_Y_GROUND   44      // 小恐龙正常位置Y
#define DINO_WIDTH      16
#define DINO_HEIGHT     18

#define BARRIER_Y       44
#define BARRIER_WIDTH   16
#define BARRIER_HEIGHT  18

#define GROUND_Y        56      // 地面条带起始Y（假设地面图高8像素，则Y=56）
#define GROUND_HEIGHT   8

#define JUMP_TOTAL_TICKS  80   // 跳跃总持续时间，单位：10ms（80 = 800ms）

/******函数声明*****/
void DinoGame_Init(void);
void DinoGame_Enter(void);
void DinoGame_Draw(void);
page_state_t DinoGame_HandleKey(uint8_t key);
void DinoGame_Tick(void);
uint8_t DinoGame_IsExitRequested(void);
void DinoGame_SetActive(uint8_t active);

#endif
