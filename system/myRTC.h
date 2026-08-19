#ifndef _MYRTC_H
#define _MYRTC_H
#include <stdint.h>

extern uint16_t MyRTC_Time[];
extern uint8_t MyRTC_Weekday;

/******函数声明*****/
void MyRTC_Init(void);
void MyRTC_SetTime(void);
void MyRTC_ReadTime(void);

#endif
