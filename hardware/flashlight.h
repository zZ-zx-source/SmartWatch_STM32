#ifndef _FLASHLIGHT_H
#define _FLASHLIGHT_H
#include <stdint.h>

/******º¯ÊýÉùÃ÷*****/
void Flashlight_Init(void);
void Detail_Flashlight(void);
uint8_t Flashlight_ProcessKey(uint8_t keynum);
void Flashlight_Tick10ms(void);

#endif
