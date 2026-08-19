#ifndef _MOTION_WAKEUP_H
#define _MOTION_WAKEUP_H

#define MOTION_CHECK_INTERVAL_MS  100    //每100ms中断检测一次

/******函数声明*****/
void Motion_Init(void);
void Motion_SetFlag(void);
void Motion_Process(void);

#endif
