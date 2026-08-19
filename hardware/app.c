#include "stm32f10x.h"                  // Device header
#include "app.h"

#include "OLED.h"
#include "key.h"
#include "buzzer.h"
#include "menu.h"
#include "timer.h"
#include "myRTC.h"
#include "AD.h"
#include "battery.h"
#include "motion_wakeup.h"
#include "auto_sleep.h"
#include "alarm.h"
#include "countdown.h"
#include "power.h"

extern volatile uint8_t second_tick_flag;

/* 初始化所有模块 */
void App_Init(void)
{
    OLED_Init();
    Key_Init();
	Buzzer_Init();
    Menu_Init();
    Timer_Init();
    MyRTC_Init();

    AD_Init();       // 初始化 ADC 外设（电池电量检测等）
    Battery_Init();

    Motion_Init();
    AutoSleep_Init();// 可放在其他初始化之后
}

/* 主循环处理函数 */
void App_Process(void)
{
    // 长按开关机（会阻塞，关机时只等待长按开机）
    Power_HandleLongPress();

    // 处理秒钟中断标志位，执行每秒处理函数
    if (second_tick_flag)
    {
        second_tick_flag = 0;
        MyRTC_ReadTime();         // 更新 MyRTC_Time 数组（年、月、日、时、分、秒）
        Alarm_CheckSecondTick();  // 闹钟检查
        Countdown_SecondTick();   // 倒计时
        AutoSleep_Tick();         // 自动息屏计时
    }

    uint8_t keynum = Key_Read();

    // 动作唤醒处理（即使在息屏状态下也检测）
    Motion_Process();

    if (AutoSleep_IsSleeping())
    {
        if (keynum)
        {
            AutoSleep_Wakeup();// 任意按键唤醒，但不响应本次按键
            Menu_Update(0);    // 刷新屏幕
        }
    }
    else
    {
        if (keynum)
        {
            AutoSleep_Reset(); // 正常状态有按键，重置无操作计时
        }
        Menu_Update(keynum);   // 正常按键处理
    }
}
