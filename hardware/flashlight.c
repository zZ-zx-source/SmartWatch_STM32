#include "stm32f10x.h"                  // Device header
#include "flashlight.h"
#include "OLED.h"
#include "led.h"

static uint8_t is_on = 0;

/* 动画相关 */
static uint8_t anim_counter = 0;   // 反色持续时间 (单位:10ms)
static uint8_t anim_active = 0;    // 1: 正在反色动画中
static uint8_t anim_ready = 0;     // 1: 动画结束，待执行状态翻转

/*保证led初始状态为熄灭*/
void Flashlight_Init(void)
{
	is_on = 0;
	LED_Init();
	LED_SetState(LED_OFF);
	anim_counter = 0;
    anim_active = 0;
    anim_ready = 0;
}

/*根据is_on显示ON或者OFF*/
void Detail_Flashlight(void)
{
	if(is_on)
	{
		OLED_ShowString(36,24,"ON ",OLED_16X32);//多写一个空格清掉旧残影
	}
	else
	{
		OLED_ShowString(36,24,"OFF",OLED_16X32);
	}
	// 如果正在反色动画中，对文字区域取反
    if (anim_active && anim_counter > 0)
    {
        OLED_ReverseArea(36,24,48,32);   // "OFF" 或 "ON " 宽度均为 3字符×16=48像素
    }
}

/* 处理手电筒界面按键
 * 0 按键未处理 ；1按键已处理，状态改变
*/
uint8_t Flashlight_ProcessKey(uint8_t keynum)
{
	// 动画结束后执行状态翻转
    if (anim_ready)
    {
        anim_ready = 0;
        is_on = !is_on;
        if (is_on)
            LED_SetState(LED_ON);
        else
            LED_SetState(LED_OFF);
        return 1;   // 状态已变，请求重绘
    }
	
	if(keynum != 3) return 0;//只关注key3键，其余按键返回0
	
	// 启动动画：反色 200ms（20×10ms）
    if (!anim_active)
    {
        anim_counter = 20;   // 200ms
        anim_active = 1;
        return 1;            // 请求重绘以显示反色
    }
	return 0;   // 动画进行中，忽略按键
}

/*由TIM2中断每10ms调用*/
void Flashlight_Tick10ms(void)
{
    if (anim_counter > 0)
    {
        anim_counter--;
        if (anim_counter == 0)
        {
            anim_active = 0;
            anim_ready = 1;   // 通知主循环执行翻转
        }
    }
}
