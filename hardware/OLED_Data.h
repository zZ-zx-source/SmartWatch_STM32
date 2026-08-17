#ifndef __OLED_DATA_H
#define __OLED_DATA_H

#include <stdint.h>

/*字符集定义*/
/*以下两个宏定义只可解除其中一个的注释*/
#define OLED_CHARSET_UTF8			//定义字符集为UTF8
//#define OLED_CHARSET_GB2312		//定义字符集为GB2312

/*字模基本单元*/
typedef struct 
{
	
#ifdef OLED_CHARSET_UTF8			//定义字符集为UTF8
	char Index[5];					//汉字索引，空间为5字节
#endif
	
#ifdef OLED_CHARSET_GB2312			//定义字符集为GB2312
	char Index[3];					//汉字索引，空间为3字节
#endif
	
	uint8_t Data[32];				//字模数据
} ChineseCell_t;

/*ASCII字模数据声明*/
extern const uint8_t OLED_F8x16[][16];
extern const uint8_t OLED_F6x8[][6];

extern const uint8_t OLED_F16x32[][64];

/*汉字字模数据声明*/
extern const ChineseCell_t OLED_CF16x16[];

/*图像数据声明*/
extern const uint8_t Diode[];
extern const uint8_t BatteryFull[];
extern const uint8_t Clock[];
extern const uint8_t FlashLight[];
extern const uint8_t Game[];
extern const uint8_t Emoji[];

extern const uint8_t Emoji_Frame0[];
extern const uint8_t Emoji_Frame1[];
extern const uint8_t Emoji_Frame2[];
extern const uint8_t Emoji_Frame3[];
extern const uint8_t Emoji_Frame4[];
extern const uint8_t Emoji_Frame5[];
extern const uint8_t Emoji_Frame6[];
extern const uint8_t Emoji_Frame7[];
extern const uint8_t Emoji_Frame8[];
extern const uint8_t Emoji_Frame9[];
extern const uint8_t Emoji_Frame10[];
extern const uint8_t Emoji_Frame11[];
extern const uint8_t Emoji_Frame12[];
extern const uint8_t Emoji_Frame13[];
extern const uint8_t Emoji_Frame14[];
extern const uint8_t Emoji_Frame15[];
extern const uint8_t Emoji_Frame16[];
extern const uint8_t Emoji_Frame17[];
extern const uint8_t Emoji_Frame18[];
extern const uint8_t Emoji_Frame19[];
extern const uint8_t Emoji_Frame20[];
extern const uint8_t Emoji_Frame21[];
extern const uint8_t Emoji_Frame22[];
extern const uint8_t Emoji_Frame23[];
extern const uint8_t Emoji_Frame24[];
extern const uint8_t Emoji_Frame25[];
extern const uint8_t Emoji_Frame26[];
extern const uint8_t Emoji_Frame27[];
extern const uint8_t Emoji_Frame28[];
extern const uint8_t Emoji_Frame29[];

extern const uint8_t Dino_Run1[];      // 小恐龙奔跑帧1
extern const uint8_t Dino_Run2[];      // 小恐龙奔跑帧2
extern const uint8_t Dino_Jump[];      // 小恐龙跳跃帧
extern const uint8_t Cactus1[];        // 仙人掌1
extern const uint8_t Cactus2[];        // 仙人掌2
extern const uint8_t Cactus3[];        // 仙人掌3
extern const uint8_t Ground[];         // 地面图（16x8）

#endif


/*****************江协科技|版权所有****************/
/*****************jiangxiekeji.com*****************/
