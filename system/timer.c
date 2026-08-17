#include "stm32f10x.h"                  // Device header
#include "timer.h"
#include "key.h"
#include "buzzer.h"
#include "countdown.h"
#include "flashlight.h"
#include "dino_game.h"
#include "menu.h"
#include "battery.h"
#include "motion_wakeup.h"
#include "emoji.h"

/*内部函数声明*/
static void Timer2_Init(void);
static void Timer3_Init(void);

void Timer_Init(void)
{
	Timer2_Init();
	Timer3_Init();
}

static void Timer2_Init(void)
{
	/*开启时钟*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	
	/*配置时钟源*/
	TIM_InternalClockConfig(TIM2);
	
	/*时基单元初始化*/
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = 1000 - 1;  //定时频率=CK_PSC/(PSC+1)/(ARR+1)
	TIM_TimeBaseInitStructure.TIM_Prescaler = 720 - 1;//        =72MHz/720/1000=100Hz=10ms
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStructure);
	
	/*中断输出配置*/
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
	
	/*NVIC中断分组*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	/*NVIC配置*/
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
	
	/*TIM使能*/
	TIM_Cmd(TIM2,ENABLE);
}

static void Timer3_Init(void)
{
	/*开启时钟*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
	
	/*配置时钟源*/
	TIM_InternalClockConfig(TIM3);
	
	/*时基单元初始化*/
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = 10000 - 1;  //定时频率=CK_PSC/(PSC+1)/(ARR+1)
	TIM_TimeBaseInitStructure.TIM_Prescaler = 7200 - 1;//        =72MHz/7200/10000=1Hz=1s
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);
	
	/*中断输出配置*/
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
	
	/*NVIC中断分组*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	/*NVIC配置*/
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;//抢占优先级高于TIM2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
	
	/*TIM使能*/
	TIM_Cmd(TIM3,ENABLE);
}
/*内部函数声明*/
static void Timer2_Tasks(void);

/*TIM2 10MS中断*/
void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM2,TIM_IT_Update) == SET)
	{
		TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
		Timer2_Tasks();   // 执行所有10ms级任务
	}
}

volatile uint8_t second_tick_flag = 0;   // 全局，在主循环中被消费

/*TIM3 秒中断*/
void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update) == SET){
		TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
		second_tick_flag = 1;   // 告诉主循环：1 秒到了
	}
}

volatile uint8_t emoji_tick_flag = 0;

/*TIM2 10ms 任务分发函数（内部使用）*/
static void Timer2_Tasks(void)
{
    Key_Tick();               // 按键扫描
    Buzzer_Tick();            // 蜂鸣器状态机
    Countdown_Tick10ms();     // 倒计时闪烁动画
    Flashlight_Tick10ms();    // 手电筒开关动画
    DinoGame_Tick();          // 小恐龙游戏状态
    Menu_Tick();              // 菜单滚动动画

    /* 电池采样 100ms 一次 */
    static uint8_t bat_tick = 0;
    if (++bat_tick >= BATTERY_SAMPLE_INTERVAL_MS / TIM2_TICK_MS)
    {
        bat_tick = 0;
        Battery_Update();
    }

    /* 抬手唤醒检测 100ms 一次 */
    static uint8_t motion_tick = 0;
    if (++motion_tick >= MOTION_CHECK_INTERVAL_MS / TIM2_TICK_MS)
    {
        motion_tick = 0;
        Motion_SetFlag();
    }

    /* 表情动画帧 80ms 一帧 */
    static uint8_t emoji_timer = 0;
    if (++emoji_timer >= EMOJI_FRAME_INTERVAL_MS / TIM2_TICK_MS)
    {
        emoji_timer = 0;
        emoji_tick_flag = 1;
    }
}
