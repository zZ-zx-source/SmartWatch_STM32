#include "stm32f10x.h"                  // Device header
#include "countdown.h"
#include "OLED.h"
#include "buzzer.h"
#include <stdio.h>

/* 状态枚举 */
typedef enum {
    CD_IDLE = 0,
    CD_RUNNING,
    CD_PAUSED
} CD_State_t;

/* 用户设置的初始时间 (时,分,秒) */
static uint8_t set_h = 0, set_m = 0, set_s = 0;
/* 当前剩余总秒数 */
static uint32_t total_seconds = 0;
/* 运行状态 */
static CD_State_t state = CD_IDLE;

/* 选中区域: 0=时, 1=分, 2=秒, 3=Start/Pause, 4=Cancel */
static uint8_t selected_field = 0;

/* Cancel 闪烁动画计数器 (单位: 10ms) */
static uint16_t cancel_flash_counter = 0;

/* 倒计时是否已触发提醒 */
static uint8_t alarm_triggered = 0;

void Countdown_Init(void)
{
    set_h = 0; set_m = 0; set_s = 0;
    total_seconds = 0;
    state = CD_IDLE;
    selected_field = 0;
    cancel_flash_counter = 0;
    alarm_triggered = 0;
}

/*进入界面时调用*/
void Countdown_Enter(void)
{
    selected_field = 0;   // 默认选中时
}

void Countdown_Draw(void)
{
    char buf[16];
    uint32_t display_sec = total_seconds;
    uint8_t h = display_sec / 3600;
    uint8_t m = (display_sec % 3600) / 60;
    uint8_t s = display_sec % 60;
    sprintf(buf, "%02d:%02d:%02d", h, m, s);
    OLED_ShowString(0, 16, buf, OLED_16X32);

    // 左下角按钮
    if (state == CD_RUNNING)
        OLED_ShowString(0, 48, "Pause", OLED_8X16);
    else
        OLED_ShowString(0, 48, "Start", OLED_8X16);

    // 右下角按钮
    OLED_ShowString(80, 48, "Cancel", OLED_8X16);

    // 高亮选中区域
    switch (selected_field)
    {
        case 0: OLED_ReverseArea(0, 16, 32, 32); break;   // 时
        case 1: OLED_ReverseArea(48, 16, 32, 32); break;  // 分
        case 2: OLED_ReverseArea(96, 16, 32, 32); break;  // 秒
        case 3: OLED_ReverseArea(0, 48, 48, 16); break;   // Start/Pause
        case 4: // Cancel
            if (cancel_flash_counter > 0)
            {
                // 闪烁：每 200ms 切换反色
                if ((cancel_flash_counter / 20) % 2 == 0)
                    OLED_ReverseArea(80, 48, 48, 16);
                // else 不反色，显示正常文本
            }
            else
            {
                OLED_ReverseArea(80, 48, 48, 16);
            }
            break;
    }
}

page_state_t Countdown_HandleKey(uint8_t key)
{
    if (key == 1)
    {
        return PAGE_TIME;   // 返回列表，不停止倒计时
    }
    else if (key == 2)
    {
        selected_field = (selected_field + 1) % 5;
        return PAGE_COUNTDOWN;
    }
    else if (key == 3)
    {
        switch (selected_field)
        {
            case 0: // 时
                if (state == CD_IDLE)
                {
                    set_h = (set_h + 1) % 24;
                    total_seconds = set_h * 3600 + set_m * 60 + set_s;
                }
                break;
            case 1: // 分
                if (state == CD_IDLE)
                {
                    set_m = (set_m + 1) % 60;
                    total_seconds = set_h * 3600 + set_m * 60 + set_s;
                }
                break;
            case 2: // 秒
                if (state == CD_IDLE)
                {
                    set_s = (set_s + 1) % 60;
                    total_seconds = set_h * 3600 + set_m * 60 + set_s;
                }
                break;
            case 3: // Start/Pause
                if (state == CD_IDLE || state == CD_PAUSED)
                {
                    if (state == CD_IDLE)
                    {
                        // 载入设定时间（可能倒计时结束 total_seconds 为 0）
                        total_seconds = set_h * 3600 + set_m * 60 + set_s;
                        if (total_seconds == 0) break;   // 避免 0 秒开始
                    }
                    state = CD_RUNNING;
                    alarm_triggered = 0;
                    Buzzer_StopAlarm();   // 关闭可能响着的蜂鸣器
                }
                else if (state == CD_RUNNING)
                {
                    state = CD_PAUSED;
                }
                break;
            case 4: // Cancel
                // 任何时候按 Cancel 都可以停止蜂鸣器并复位
                if (state == CD_RUNNING || state == CD_PAUSED || (state == CD_IDLE && alarm_triggered))
                {
                    Buzzer_StopAlarm();
                    alarm_triggered = 0;
                    total_seconds = set_h * 3600 + set_m * 60 + set_s; // 复位到初始设定
                    state = CD_IDLE;
                    cancel_flash_counter = 100;   // 1 秒闪烁动画 (100 * 10ms)
                }
                break;
        }
        return PAGE_COUNTDOWN;
    }
    return PAGE_COUNTDOWN;
}

/* 由主循环每秒调用 */
void Countdown_SecondTick(void)
{
    if (state == CD_RUNNING)
    {
        if (total_seconds > 0)
        {
            total_seconds--;
            if (total_seconds == 0)
            {
                state = CD_IDLE;
                alarm_triggered = 1;
                Buzzer_StartAlarm();   // 非阻塞蜂鸣器
            }
        }
    }
}

/*由TIM2 10ms中断调用（用于Cancel动画）*/
void Countdown_Tick10ms(void)
{
    if (cancel_flash_counter > 0)
    {
        cancel_flash_counter--;
    }
}
