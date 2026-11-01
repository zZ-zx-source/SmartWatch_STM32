#include "stm32f10x.h"                  // Device header
#include "power.h"
#include "key.h"
#include "OLED.h"
#include "Delay.h"
#include "auto_sleep.h"   // 用于重置自动息屏计时
#include "menu.h"
#include "led.h"
#include "buzzer.h"

//extern volatile uint8_t second_tick_flag;   // 来自 timer.c

/**
 * 硬件电源自锁初始化
 * 该函数必须在 main 函数最开头调用，锁住系统电源，防止松手断电
 */
void Power_Init(void)
{
    /* ========== 锁住电源 (PA4 = CTL) ========== */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); // 开启GPIOA时钟
    
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;      // 推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;             // PA4 (CTL)
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    //NPN 三极管，必须输出高电平才能导通并锁住电源！
    GPIO_SetBits(GPIOA, GPIO_Pin_4); 
}

/**
 * 长按Key3进入/退出模拟关机
 */
void Power_HandleLongPress(void)
{
    if (Key_ReadLongPress() != 3) return;   // 无长按，直接返回

    /* 1. 如果处于息屏状态，先唤醒屏幕，让用户看到提示 */
    if (AutoSleep_IsSleeping())
    {
        AutoSleep_Wakeup(); 
        Menu_Update(0);    
        Delay_ms(300);    
    }
	
    /* 2. 显示关机提示 */
    OLED_Clear();
    OLED_ShowString(28, 24, "POWER OFF", OLED_8X16);
    OLED_Update();

    /* 3. 立刻强制关闭所有可能正在工作的外设！ */
    LED_SetState(LED_OFF);
    Buzzer_StopAlarm();
	
    /* 4. 关闭 OLED 屏幕 */
    OLED_WriteCommand(0xAE);   

    /* 5. 等待用户松手（如果不松手，系统会在这里稍微等一下，但外设已经关了） */
    while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_15) == 0);

    /* 6. 拉低 CTL (PA4)，彻底切断电源！*/
    GPIO_ResetBits(GPIOA, GPIO_Pin_4); 

    /* 7. 死循环兜底（如果由于充电宝供电导致断电不彻底，就卡在这里，功耗极低） */
    while (1)
    {
        // 这里什么都不用做，因为电已经断了。
        // 如果没断（例如插着充电宝），就让它永远停在这里，等待彻底断电或复位。
    }
}
