#include "stm32f10x.h"                  // Device header
#include "power.h"
#include "key.h"
#include "OLED.h"
#include "Delay.h"
#include "auto_sleep.h"   // 用于重置自动息屏计时
#include "menu.h"
#include "led.h"
#include "buzzer.h"

extern volatile uint8_t second_tick_flag;   // 来自 timer.c

/**
 * 长按Key3进入/退出模拟关机
 * 关机时关闭OLED并阻塞主循环，只保留按键扫描
 * 再次长按Key3唤醒，恢复显示和任务
 */
void Power_HandleLongPress(void)
{
    if (Key_ReadLongPress() != 3) return;   // 无长按，直接返回

	/* 如果处于息屏状态，先唤醒屏幕，让用户看到提示 */
    if (AutoSleep_IsSleeping())
    {
        AutoSleep_Wakeup(); // 打开OLED显示
        Menu_Update(0);    // 刷新显示，回到当前页面（如主页）
        Delay_ms(300);    // 短暂停顿，让用户看清界面
    }
	
    /* ---------- 关机流程 ---------- */
    OLED_Clear();
    OLED_ShowString(28, 24, "POWER OFF", OLED_8X16);
    OLED_Update();
    Delay_ms(500);

	// 关闭可能正在工作的外设
    LED_SetState(LED_OFF);
    Buzzer_StopAlarm();
	
    OLED_WriteCommand(0xAE);   // 关闭OLED显示

    // 等待Key3释放，避免松手后立即触发开机
    while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_15) == 0);

    /* ---------- 模拟关机循环：只检测长按开机 ---------- */
    while (1)
    {
        if (Key_ReadLongPress() == 3)
        {
            /* ---------- 开机流程：显示提示后直接软复位 ---------- */
			OLED_WriteCommand(0xAF);   // 开启OLED显示
			OLED_Clear();
			OLED_ShowString(28, 24, "POWER ON", OLED_8X16);
			OLED_Update();
			Delay_ms(300);             // 短暂显示 Power On

			NVIC_SystemReset();        // 软复位，重新运行程序，回到主界面（相当于按复位键？）
			while(1);                  // 防止复位失败时继续运行（一般不会执行到这里）
        }
        Delay_ms(10);   // 轻微延时，降低CPU占用（不影响按键中断）
    }
    // 开机后主循环会立即调用 Menu_Update 刷新界面
} 
