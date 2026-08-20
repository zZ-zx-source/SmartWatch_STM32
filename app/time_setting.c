#include "stm32f10x.h"                  // Device header
#include "time_setting.h"
#include "myRTC.h"
#include "OLED.h"
#include <stdio.h>

/* 本地副本：年、月、日、时、分、秒 */
static uint16_t setting_values[6];
static uint8_t selected_row = 0;   // 0~5

/*进入时间设置界面时加载当前 RTC 时间*/
void TimeSetting_Enter(void)
{
    for (int i = 0; i < 6; i++)
	{
        setting_values[i] = MyRTC_Time[i];
	}
    selected_row = 0;
}

/*绘制时间设置界面（分两页，每页三行）*/
void TimeSetting_Draw(void)
{
    uint8_t page_start = (selected_row / 3) * 3;   // 0 或 3
    uint8_t y = 16;   // 状态栏下方第一行

    for (int i = 0; i < 3; i++)
    {
        uint8_t row = page_start + i;
        char buf[16];

        // 月份、日期、时分秒补零
        if (row == 0)        sprintf(buf, "year:%d", setting_values[0]);
        else if (row == 1)   sprintf(buf, "month:%02d", setting_values[1]);
        else if (row == 2)   sprintf(buf, "day:%02d", setting_values[2]);
        else if (row == 3)   sprintf(buf, "hour:%02d", setting_values[3]);
        else if (row == 4)   sprintf(buf, "minute:%02d", setting_values[4]);
        else                 sprintf(buf, "second:%02d", setting_values[5]);

        OLED_ShowString(0, y + i * 16, buf, OLED_8X16);
    }

    // 高亮当前选中行（相对于当前页的位置）
    uint8_t visible_pos = selected_row - page_start;
    OLED_ReverseArea(0, y + visible_pos * 16, 128, 16);
}

/**
 * 处理时间设置界面的按键
 * 新的页面状态（PAGE_TIME 或 PAGE_TIME_SETTING）
 */
page_state_t TimeSetting_HandleKey(uint8_t key)
{
    if (key == 1)   // 返回，保存修改
    {
        for (int i = 0; i < 6; i++)
		{
            MyRTC_Time[i] = setting_values[i];
		}
        MyRTC_SetTime();          // 更新 RTC 计数器
        return PAGE_TIME;
    }
    else if (key == 2)   // 移动选择行
    {
        selected_row = (selected_row + 1) % 6;
        return PAGE_TIME_SETTING;
    }
    else if (key == 3)   // 增加当前选中项的值
    {
        switch (selected_row)
        {
            case 0: // year
                setting_values[0]++;
                if (setting_values[0] > 2030) setting_values[0] = 2026;
                break;
            case 1: // month
                setting_values[1]++;
                if (setting_values[1] > 12) setting_values[1] = 1;
                break;
            case 2: // day
                setting_values[2]++;
                if (setting_values[2] > 31) setting_values[2] = 1;
                break;
            case 3: // hour
                setting_values[3]++;
                if (setting_values[3] > 23) setting_values[3] = 0;
                break;
            case 4: // minute
                setting_values[4]++;
                if (setting_values[4] > 59) setting_values[4] = 0;
                break;
            case 5: // second
                setting_values[5]++;
                if (setting_values[5] > 59) setting_values[5] = 0;
                break;
        }
        return PAGE_TIME_SETTING;
    }
    return PAGE_TIME_SETTING;   // 其他按键忽略
}
