#ifndef _EMOJI_H
#define _EMOJI_H

#define EMOJI_FRAME_INTERVAL_MS  80    //每80ms中断检测一次

/******函数声明*****/
void Emoji_Init(void);
void Detail_Emoji(void);
void Emoji_UpdateFrame(void);

#endif
