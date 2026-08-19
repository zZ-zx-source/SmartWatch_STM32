#include "stm32f10x.h"                  // Device header
#include "auto_sleep.h"
#include "OLED.h"      // 使用 OLED_WriteCommand

static uint8_t sleep_counter = 0;   // 无操作秒数
static uint8_t is_sleeping = 0;     // 息屏状态

void AutoSleep_Init(void)
{
    sleep_counter = 0;
    is_sleeping = 0;
}

void AutoSleep_Reset(void)
{
    sleep_counter = 0;
}

/*每秒调用一次（由主循环 second_tick_flag 处理）*/
void AutoSleep_Tick(void)
{
    if (is_sleeping) return;   // 已经息屏，不再计时

    sleep_counter++;
    if (sleep_counter >= SLEEP_TIMEOUT_SEC)
    {
        sleep_counter = 0;
        AutoSleep_EnterSleep();
    }
}

uint8_t AutoSleep_IsSleeping(void)
{
    return is_sleeping;
}

void AutoSleep_EnterSleep(void)
{
    is_sleeping = 1;
    OLED_WriteCommand(0xAE);   // SSD1306 关闭显示
}

void AutoSleep_Wakeup(void)
{
    is_sleeping = 0;
    sleep_counter = 0;
    OLED_WriteCommand(0xAF);   // SSD1306 开启显示
    // 唤醒后需要刷新一次画面，主循环会调用 Menu_Update
}
