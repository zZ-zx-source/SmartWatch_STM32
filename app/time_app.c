#include "stm32f10x.h"                  // Device header
#include "time_app.h"
#include "OLED.h"
#include "time_setting.h"
#include "alarm.h"
#include "countdown.h"

/* 时间选项列表的项目 */
static const char *time_items[] = {
    "Time Setting",
    "Alarm",
    "Countdown Timer"
};

#define TIME_ITEM_COUNT  3

static uint8_t selected_item = 0;   // 当前高亮项索引

void TimeApp_Init(void)
{
    selected_item = 0;
	Alarm_Init();
	Countdown_Init();
}

/**
 * 绘制指定页面（由 Menu_Update 调用）
 * page  当前页面状态
 */
void TimeApp_Draw(page_state_t page)
{	
    switch (page)
    {
        case PAGE_TIME:
        {
            // 绘制三个选项
            for (int i = 0; i < TIME_ITEM_COUNT; i++)
            {
                OLED_ShowString(0, 16 + i * 16, (char *)time_items[i], OLED_8X16);
            }
            // 高亮当前选中行
            OLED_ReverseArea(0, 16 + selected_item * 16, 128, 16);
            break;
        }
        case PAGE_TIME_SETTING:       TimeSetting_Draw();break;
        case PAGE_ALARM:              Alarm_Draw();break;
        case PAGE_COUNTDOWN:          Countdown_Draw();break;
        default:
			break;
	}
}

/**
 * 处理时间相关页面的按键
 * key  按键值 (1/2/3)
 * currentPage  当前页面
 */
page_state_t TimeApp_HandleKey(uint8_t key, page_state_t currentPage)
{
    // ---------- 选项列表界面 ----------
    if (currentPage == PAGE_TIME)
    {
        if (key == 2)   // 向下选择
        {
            selected_item = (selected_item + 1) % TIME_ITEM_COUNT;
            return PAGE_TIME;   // 仍在列表，但选项变化，需重绘
        }
        else if (key == 3)   // 确认进入
        {
            switch (selected_item)
            {
                case 0: return PAGE_TIME_SETTING;
                case 1: return PAGE_ALARM;
                case 2: return PAGE_COUNTDOWN;
                default: return PAGE_TIME;
            }
        }
        else if (key == 1)   // 返回菜单（由外部 Menu_Update 也可处理，但这里统一返回）
        {
            return PAGE_MENU;
        }
    }
	// ---------- 子界面 (Time Setting / Alarm / Countdown) ----------
	else if (currentPage == PAGE_TIME_SETTING)
	{
		return TimeSetting_HandleKey(key);
	}
	else if (currentPage == PAGE_ALARM)
	{
		return Alarm_HandleKey(key);
	}
	else if (currentPage == PAGE_COUNTDOWN)
	{
		return Countdown_HandleKey(key);
	}
	
    return currentPage;   // 未处理，页面不变
}
