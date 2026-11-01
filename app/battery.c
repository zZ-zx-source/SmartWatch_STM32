#include "stm32f10x.h"       // Device header           
#include "battery.h"
#include "AD.h"       // AD_GetValue    
#include "OLED.h"
#include "Delay.h"       

/* 滑动平均窗口，取最近10次ADC采样的平均值，让电量百分比显示更平稳*/
#define FILTER_SIZE  10

static uint16_t adc_buffer[FILTER_SIZE];// 存储最近10次ADC值
static uint8_t buf_index = 0;           // 当前要替换的缓冲索引（环形）
static uint32_t adc_sum = 0;            // 当前缓冲区所有值的总和

static uint8_t current_percent = 100;   // 当前电量百分比
static uint8_t last_bars = 3;           // 上一次显示的格数（0~3）

static uint8_t up_timer = 0;     // 涨电专用计时器
static uint8_t down_timer = 0;   // 掉电专用计时器
static uint8_t init_counter = 0;        

// 记录开机以来的绝对最高电压（用于判断是否在充电）
static uint32_t max_voltage_mv = 0;     
/* 满格电池图标 */
extern const uint8_t BatteryFull[];

uint8_t Calculate_Percent(uint32_t voltage_mv) {
    if (voltage_mv < 3400) return 0; 
    else if (voltage_mv >= 4150) return 100;
    else if (voltage_mv >= 3950) return 75 + (voltage_mv - 3950) * 25 / 200;
    else if (voltage_mv >= 3750) return 40 + (voltage_mv - 3750) * 35 / 200;
    else if (voltage_mv >= 3600) return 15 + (voltage_mv - 3600) * 25 / 150;
    else return (voltage_mv - 3400) * 15 / 200; 
}

void Battery_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_SetBits(GPIOA, GPIO_Pin_5); 
    
    init_counter = 0; 
    up_timer = 0;
    down_timer = 0;
    adc_sum = 0;
    buf_index = 0;
    max_voltage_mv = 0; 
    
    for(int i=0; i<FILTER_SIZE; i++) adc_buffer[i] = 0;
}

/**
 * 读取 ADC，更新电量百分比和电池图标（局部刷新）
 * 每 100ms 调用一次，由 TIM2 中断驱动
 */
void Battery_Update(void)
{
    GPIO_ResetBits(GPIOA, GPIO_Pin_5); 
    Delay_us(100);                     
    uint16_t raw = AD_GetValue();  
    GPIO_SetBits(GPIOA, GPIO_Pin_5); 

    // 第一阶段：开机前 1 秒，等待系统稳定
    if (init_counter < 10) {
        adc_buffer[buf_index] = raw;
        adc_sum += raw; 
        buf_index = (buf_index + 1) % FILTER_SIZE;
        
        init_counter++;
        if (init_counter == 10) {
            uint16_t avg = adc_sum / 10; 
            uint32_t voltage_mv = (uint32_t)avg * 5200 / 4095;
            current_percent = Calculate_Percent(voltage_mv);
            
            // 初始记录水位线，加 10mV 容差
            max_voltage_mv = voltage_mv + 10; 
            
            if (current_percent == 0)      last_bars = 0;
            else if (current_percent <= 33) last_bars = 1;
            else if (current_percent <= 66) last_bars = 2;
            else                            last_bars = 3;
        }
        return;
    }

    // 第二阶段：正常运行
    adc_sum -= adc_buffer[buf_index];
    adc_sum += raw;
    adc_buffer[buf_index] = raw;
    buf_index = (buf_index + 1) % FILTER_SIZE;
    uint16_t avg = adc_sum / FILTER_SIZE;

    uint32_t voltage_mv = (uint32_t)avg * 5200 / 4095;  
    uint8_t target_percent = Calculate_Percent(voltage_mv);

    // 刷新最高水位线（如果电压突破历史，说明真的在充）
    if (voltage_mv > max_voltage_mv) {
        max_voltage_mv = voltage_mv;
    }

    // 1. 涨电逻辑
    if (target_percent > current_percent) 
    {
        // 只有在逼近历史最高电压时，才认为是真充电
        if (voltage_mv >= max_voltage_mv - 15) {
            up_timer++;
            // ★ 缩短到 1.5 秒（15次），让充电涨得明显一点
            if (up_timer >= 10) {
                up_timer = 0;
                current_percent++;
            }
        } else {
            up_timer = 0; // 假回弹，不准涨
        }
        down_timer = 0; // 重置掉电计时器
    } 
    // 2. 掉电逻辑
    else if (target_percent < current_percent) 
    {
        down_timer++;
        // ★ 延长到 5 秒（50次），死死防住按键掉压
        if (down_timer >= 50) { 
            down_timer = 0;
            current_percent--;
        }
        up_timer = 0; // 重置涨电计时器
    } 
    else 
    {
        up_timer = 0; 
        down_timer = 0;
    }

    if (current_percent == 0)      last_bars = 0;
    else if (current_percent <= 33) last_bars = 1;
    else if (current_percent <= 66) last_bars = 2;
    else                            last_bars = 3;
}

/*返回当前电量百分比（供状态栏调用）*/
uint8_t Battery_GetPercent(void)
{
    return current_percent;
}
