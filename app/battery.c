#include "stm32f10x.h"                  // Device header
#include "battery.h"
#include "AD.h"           // AD_GetValue
#include "OLED.h"

/* 滑动平均窗口，取最近10次ADC采样的平均值，让电量百分比显示更平稳*/
#define FILTER_SIZE  10

static uint16_t adc_buffer[FILTER_SIZE];// 存储最近10次ADC值
static uint8_t buf_index = 0;           // 当前要替换的缓冲索引（环形）
static uint32_t adc_sum = 0;            // 当前缓冲区所有值的总和
static uint8_t buf_filled = 0;          // 标记缓冲区是否已填满

static uint8_t current_percent = 100;   // 当前电量百分比
static uint8_t last_bars = 3;           // 上一次显示的格数（0~3）

/* 满格电池图标 */
extern const uint8_t BatteryFull[];

void Battery_Init(void)
{
    // 预填滤波缓冲区
    for (int i = 0; i < FILTER_SIZE; i++)
    {
        adc_buffer[i] = AD_GetValue();  // 读取一次ADC
        adc_sum += adc_buffer[i];       // 累加求和
    }
    buf_filled = 1;                     // 缓冲区已满
    current_percent = 100;
    last_bars = 3;
}

/**
 * 读取 ADC，更新电量百分比和电池图标（局部刷新）
 * 每 100ms 调用一次，由 TIM2 中断驱动
 */
void Battery_Update(void)
{
    uint16_t raw = AD_GetValue();  // 新采样值

    // 滑动平均
    adc_sum -= adc_buffer[buf_index];                  // 减去最旧值
    adc_sum += raw;                                    // 加上新值
    adc_buffer[buf_index] = raw;                       // 更新缓冲区
    buf_index = (buf_index + 1) % FILTER_SIZE;         // 移动索引，形成环形
    if (!buf_filled && buf_index == 0) buf_filled = 1;
    uint16_t avg = adc_sum / FILTER_SIZE;              // 计算平均值

    // 映射到 0~100%（根据实际分压修改）
    uint8_t percent = (uint32_t)avg * 100 / 4095;
    if (percent > 100) percent = 100;

    // 计算应显示的格数
    uint8_t bars;
    if (percent == 0)      bars = 0;
    else if (percent <= 33) bars = 1;
    else if (percent <= 66) bars = 2;
    else                    bars = 3;

    // 避免重复刷新
    if (percent == current_percent && bars == last_bars) return;

    current_percent = percent;
    last_bars = bars;
}

/*返回当前电量百分比（供状态栏调用）*/
uint8_t Battery_GetPercent(void)
{
    return current_percent;
}
