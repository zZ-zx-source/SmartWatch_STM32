#include "stm32f10x.h"                  // Device header
#include "key.h"

// 每个按键的状态
typedef struct {
    uint8_t state;             // 0:空闲, 1:消抖中, 2:按下等待释放, 3:释放消抖
    uint8_t count;             // 当前状态计时（以 TICK_MS 为单位）
    uint8_t pressed;           // 是否检测到一次有效按下（释放后置1，等待读取）
    uint8_t value;             // 该按键对应的返回键值（1/2/3）
	uint16_t hold_count;       // 按下持续时间计数，单位10ms
    uint8_t long_press;        // 是否发生长按
	uint8_t long_press_done;   // 长按已处理标记
} Key_Control;

static Key_Control keys[3];
static uint8_t key_ready = 0;       // 是否有未读取的按键
static uint8_t key_value = 0;       // 最近一次按键值

void Key_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = KEY1_PIN | KEY2_PIN | KEY3_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	// 初始化状态
    keys[0].value = 1; keys[0].state = 0;
    keys[1].value = 2; keys[1].state = 0;
    keys[2].value = 3; keys[2].state = 0;
    key_ready = 0;
}

/* 单个按键状态机(内部使用) */
static void Key_Process(Key_Control *k, uint8_t current_level)
{
    switch (k->state) {
        case 0:   // 空闲，检测按下
            if (current_level == 0) {   // 低电平，可能按下
                k->state = 1;
                k->count = 0;
				k->hold_count = 0;
				k->long_press = 0;   // 清除上次可能残留的长按标志
				k->long_press_done = 0;   // 清除标记，允许新一次长按
            }
            break;

        case 1:   // 消抖中
            k->count++;
            if (k->count >= (DEBOUNCE_MS / TICK_MS)) {
                if (GPIO_ReadInputDataBit(KEY_PORT, k->value == 1 ? KEY1_PIN :
                                          (k->value == 2 ? KEY2_PIN : KEY3_PIN)) == 0) {
                    // 确认按下
                    k->state = 2;      // 等待释放
                } else {
                    k->state = 0;      // 抖动，回空闲
                }
                k->count = 0;
            }
            break;

        case 2:   // 等待释放
			if (current_level == 0)   // 仍按下
			{
				if (!k->long_press_done)   // 如果长按还未被处理
				{
					k->hold_count++;
					if (k->hold_count >= 200)   // 达到2秒
					{
						k->long_press = 1;       // 标记长按
						k->long_press_done = 1;  // 设置为已处理，防止重复设置
						k->hold_count = 0;       // 清零，不再增加
					}
				}
				// 如果已处理，则不再计数，等待释放
			}
			else   // 释放
			{
				k->state = 3;   // 进入释放消抖
				k->count = 0;
			}
			break;

        case 3:   // 释放消抖
            k->count++;
            if (k->count >= (DEBOUNCE_MS / TICK_MS)) {
                if (GPIO_ReadInputDataBit(KEY_PORT, k->value == 1 ? KEY1_PIN :
                                          (k->value == 2 ? KEY2_PIN : KEY3_PIN)) == 1) {
					// 确认释放
                    if (k->long_press)
                    {
						// 长按，跳过else分支，不生成短按键值，但设置长按标志
						// 可以在外部用函数查询
						// 这里不清除 k->long_press，由主循环处理
					}
					else
					{
						// 短按
						k->pressed = 1;
						key_ready = 1;
						key_value = k->value;
					}
                }
                k->state = 0;          // 回到空闲
                k->count = 0;
				k->hold_count = 0;   // 清除按住计时
				k->long_press_done = 0;   // 清除标记
            }
            break;
    }
}

/* 查询是否有长按事件（仅Key3） */
uint8_t Key_ReadLongPress(void)
{
    if (keys[2].long_press)   // keys[2] 对应 Key3
    {
        keys[2].long_press = 0;   // 清除标志
        return 3;   // 返回按键编号
    }
    return 0;    // 无长按事件
}

/* 每10ms由TIM2中断调用 */
void Key_Tick(void)
{
    uint8_t levels[3];
    levels[0] = GPIO_ReadInputDataBit(KEY_PORT, KEY1_PIN);
    levels[1] = GPIO_ReadInputDataBit(KEY_PORT, KEY2_PIN);
    levels[2] = GPIO_ReadInputDataBit(KEY_PORT, KEY3_PIN);

    for (int i = 0; i < 3; i++) {
        Key_Process(&keys[i], levels[i]);
    }
}

/* 主循环获取键值（非阻塞） */
uint8_t Key_Read(void)
{
    uint8_t ret = 0;
    if (key_ready) {
        ret = key_value;
        key_ready = 0;
        key_value = 0;
    }
    return ret;
}
