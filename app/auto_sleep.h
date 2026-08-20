#ifndef _AUTO_SLEEP_H
#define _AUTO_SLEEP_H
#include <stdint.h>

#define SLEEP_TIMEOUT_SEC  20   // 20秒无操作进入息屏

/******函数声明*****/
void AutoSleep_Init(void);
void AutoSleep_Reset(void);
void AutoSleep_Tick(void);
uint8_t AutoSleep_IsSleeping(void);
void AutoSleep_EnterSleep(void);
void AutoSleep_Wakeup(void);

#endif
