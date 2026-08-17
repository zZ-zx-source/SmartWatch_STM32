#include "stm32f10x.h"                  // Device header
#include "buzzer.h"

/* 蜂鸣器状态机 */
static uint8_t alarm_active = 0;    // 1：闹钟正在鸣叫
static uint8_t beep_phase = 0;      // 0~3
static uint8_t beep_tick = 0;       // 计数，单位 10ms

void Buzzer_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	GPIO_ResetBits(GPIOB, GPIO_Pin_1);   // 初始关闭
}

/*启动闹钟鸣叫*/
void Buzzer_StartAlarm(void)
{
    alarm_active = 1;
    beep_phase = 0;
    beep_tick = 0;
    GPIO_SetBits(GPIOB, GPIO_Pin_1);     // 开始响
}

/*停止鸣叫*/
void Buzzer_StopAlarm(void)
{
    alarm_active = 0;
    GPIO_ResetBits(GPIOB, GPIO_Pin_1);   // 关闭蜂鸣器
}

/**
 * 每 10ms 调用一次（由 TIM2 中断触发）
 * 实现非阻塞的闹钟鸣叫模式：响100ms，停100ms，响100ms，停700ms，循环
 */
void Buzzer_Tick(void)
{
    if (!alarm_active) return;

    beep_tick++;
    switch (beep_phase)
    {
        case 0:   // 响 100ms
            if (beep_tick >= 10)   // 10 * 10ms = 100ms
            {
                beep_tick = 0;
                GPIO_ResetBits(GPIOB, GPIO_Pin_1);  // 停止
                beep_phase = 1;
            }
            break;
        case 1:   // 停 100ms
            if (beep_tick >= 10)
            {
                beep_tick = 0;
                GPIO_SetBits(GPIOB, GPIO_Pin_1);    // 再响
                beep_phase = 2;
            }
            break;
        case 2:   // 响 100ms
            if (beep_tick >= 10)
            {
                beep_tick = 0;
                GPIO_ResetBits(GPIOB, GPIO_Pin_1);  // 停止
                beep_phase = 3;
            }
            break;
        case 3:   // 停 700ms
            if (beep_tick >= 70)   // 70 * 10ms = 700ms
            {
                beep_tick = 0;
                GPIO_SetBits(GPIOB, GPIO_Pin_1);    // 重新开始
                beep_phase = 0;
            }
            break;
    }
}
