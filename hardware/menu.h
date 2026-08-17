#ifndef _MENU_H
#define _MENU_H
#include <stdint.h>

typedef enum{
	PAGE_MAIN = 0,
	PAGE_MENU,
	PAGE_TIME,        // 时间选项列表
	PAGE_TIME_SETTING,// Time Setting 子界面
    PAGE_ALARM,       // Alarm 子界面
    PAGE_COUNTDOWN,   // Countdown Timer 子界面
	PAGE_GAME,        // 游戏列表
    PAGE_DINO,        // Chrome Dino 游戏界面
	PAGE_FLASHLIGHT,  //手电筒详情
	PAGE_EMOJI        //动态表情详情
}page_state_t;

#define MENU_MODE_PREVIEW 0 //两图标，无高亮
#define MENU_MODE_ACTIVE 1  //两图标，中间反色（已激活但未展开）
#define MENU_MODE_SCROLL 2  //三图标循环，中间反色，可滚动

/* 菜单滚动动画参数 */
#define MENU_ICON_SLOT_WIDTH  48              // 槽位宽度（5->50->95 差45，取48方便）
#define MENU_ANIM_STEP        2               // 每10ms偏移2像素
#define MENU_ANIM_TOTAL_STEPS (MENU_ICON_SLOT_WIDTH / MENU_ANIM_STEP) // 12步

/******函数声明*****/
void Menu_Init(void);
void Menu_Tick(void);
void Menu_Update(uint8_t keynum);

#endif
