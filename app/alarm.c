#include "stm32f10x.h"                  // Device header
#include "alarm.h"
#include "myRTC.h"
#include "OLED.h"
#include "buzzer.h"
#include <stdio.h>

/* 闹钟设定值 */
static uint8_t alarm_hour = 22;
static uint8_t alarm_min = 0;

/* 闹钟状态：0 = 未激活（Start），1 = 已激活（Cancel） */
static uint8_t alarm_enabled = 0;

/* 选中位置：0=小时, 1=分钟, 2=Start/Cancel */
static uint8_t selected_field = 0;

/* 闹钟是否正在鸣叫 */
static uint8_t is_ringing = 0;

void Alarm_Init(void)
{
    alarm_hour = 22;
    alarm_min = 0;
    alarm_enabled = 0;
    selected_field = 0;
    is_ringing = 0;
}

/* 进入闹钟界面时调用*/
void Alarm_Enter(void)
{
    selected_field = 0;   // 默认选中小时位
}

void Alarm_Draw(void)
{
    char buf[16];
    sprintf(buf, "%02d:%02d", alarm_hour, alarm_min);
    OLED_ShowString(24, 16, buf, OLED_16X32);   // 闹钟时间

    if (alarm_enabled)
        OLED_ShowString(0, 48, "Cancel", OLED_8X16);
    else
        OLED_ShowString(0, 48, "Start", OLED_8X16);

    /* 高亮选中字段 */
    if (selected_field == 0)        // 小时
        OLED_ReverseArea(24, 16, 32, 32);           // 高亮前两个字符
    else if (selected_field == 1)   // 分钟
        OLED_ReverseArea(72, 16, 32, 32);           // 高亮后两个字符（假设 ':' 不计）
    else if (selected_field == 2)   // Start/Cancel
        OLED_ReverseArea(0, 48, 48, 16);            // 高亮该单词区域
}

page_state_t Alarm_HandleKey(uint8_t key)
{
    if (key == 1)   // 返回列表
    {
		if (key == 1)
		{
        // 直接返回，不停止闹钟
			return PAGE_TIME;
		}
        return PAGE_TIME;
    }
    else if (key == 2)   // 切换选中字段
    {
        selected_field = (selected_field + 1) % 3;
        return PAGE_ALARM;
    }
    else if (key == 3)   // 修改值或切换 Start/Cancel
    {
        switch (selected_field)
        {
            case 0:   // 小时
                alarm_hour = (alarm_hour + 1) % 24;
                break;
            case 1:   // 小时
                alarm_min = (alarm_min + 1) % 60;
                break;
            case 2:   // Start/Cancel
                if (alarm_enabled)
                {
                    // 当前是 Cancel，切换为 Start，关闭闹钟
                    alarm_enabled = 0;
                    if (is_ringing)
                    {
                        Buzzer_StopAlarm();
                        is_ringing = 0;
                    }
                }
                else
                {
                    // 当前是 Start，切换为 Cancel，启用闹钟
                    alarm_enabled = 1;
                }
                break;
        }
        return PAGE_ALARM;
    }
    return PAGE_ALARM;
}

/*每秒检查闹钟是否触发（由主循环 second_tick_flag 处理）*/
void Alarm_CheckSecondTick(void)
{
    if (!alarm_enabled) return;
    if (is_ringing) return;   // 已经响铃中，不重复启动

    uint8_t current_hour = MyRTC_Time[3];
    uint8_t current_min  = MyRTC_Time[4];
    uint8_t current_sec  = MyRTC_Time[5];

    // 仅在整分钟的第一秒触发
    if (current_sec == 0 && current_hour == alarm_hour && current_min == alarm_min)
    {
        Buzzer_StartAlarm();
        is_ringing = 1;
    }
}
