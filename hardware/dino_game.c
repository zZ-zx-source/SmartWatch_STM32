#include "stm32f10x.h"                  // Device header
#include "dino_game.h"
#include "OLED.h"
#include <stdio.h>     //sprintf
#include <stdlib.h>    // rand
#include <math.h>      // sin

/* ---------------- 游戏状态 ---------------- */
typedef struct {
    uint8_t minX, minY, maxX, maxY;
} Rect;

static uint8_t exit_request = 0;   // 请求退出标志
static uint8_t game_active = 0;    // 游戏是否激活（当前是否在游戏页）

static int score = 0;
static uint16_t ground_pos = 0;
static uint16_t barrier_pos = 0;
static uint8_t barrier_type = 0;
static uint8_t barrier_active = 1;      // 障碍物是否在屏幕上

static uint8_t dino_jump_flag = 0;      // 0:奔跑, 1:跳跃
static uint16_t jump_t = 0;
static uint8_t jump_height = 0;         // 当前跳跃高度（像素）

static uint8_t game_over_flag = 0;      // 1: 碰撞发生，等待延时返回
static uint16_t game_over_timer = 0;    // Game Over 停留计时 (单位:10ms)

static Rect dino_rect, barrier_rect;

/* ---------------- 内部函数声明 ---------------- */
static void Update_Rects(void);
static uint8_t Check_Collision(void);

/* ---------------- 初始化 ---------------- */
void DinoGame_Init(void)
{
    score = 0;
    ground_pos = 0;
    barrier_pos = 128;   //初始让障碍物完全在屏幕外（x=127-128=-1?）
    barrier_active = 1;
    barrier_type = 0;
    dino_jump_flag = 0;
    jump_t = 0;
    jump_height = 0;
    game_over_flag = 0;
    game_over_timer = 0;
    exit_request = 0;
    game_active = 0;
    srand(1234);   // 随机种子（可改为使用 RTC 计数器）
}

void DinoGame_Enter(void)
{
    DinoGame_Init();      // 进入游戏时重置所有状态
	game_active = 1;      // 标记游戏正在运行
    exit_request = 0;     // 清除退出请求
}

/* ---------------- 绘制函数（绘制当前帧）---------------- */
void DinoGame_Draw(void)
{
//    // 清屏（全屏重绘）
//    OLED_Clear();//不需要，因为在Menu_Update 已经统一执行了 OLED_Clear()

    // 显示得分（右上角，y 下移至 20）
	OLED_ShowNum(98, 20, score, 5, OLED_6X8);

    // 2. 绘制地面（滚动效果：直接画一整条移动的地面）
    //    简化：用 OLED_ShowImage 画一个固定长度的地面，根据 ground_pos 偏移绘制
    //    这里用一个 16x8 的地面条纹重复绘制或使用 ShowImage 偏移
    for (int x = - (ground_pos % 16); x < 128; x += 16)
    {
        OLED_ShowImage(x, GROUND_Y, 16, GROUND_HEIGHT, Ground);
    }

    // 3. 绘制障碍物（仙人掌）
    if (barrier_active)
    {
        OLED_ShowImage(127 - barrier_pos, BARRIER_Y, BARRIER_WIDTH, BARRIER_HEIGHT,
                       (barrier_type == 0) ? Cactus1 : (barrier_type == 1) ? Cactus2 : Cactus3);
    }

    // 5. 绘制小恐龙
    if (dino_jump_flag)
    {
        OLED_ShowImage(DINO_X, DINO_Y_GROUND - jump_height, DINO_WIDTH, DINO_HEIGHT, Dino_Jump);
    }
    else
    {
        // 奔跑动画：两帧交替，按 ground_pos 奇偶决定
        if ((ground_pos / 8) % 2 == 0)
            OLED_ShowImage(DINO_X, DINO_Y_GROUND, DINO_WIDTH, DINO_HEIGHT, Dino_Run1);
        else
            OLED_ShowImage(DINO_X, DINO_Y_GROUND, DINO_WIDTH, DINO_HEIGHT, Dino_Run2);
    }

    // 6. 如果 Game Over，显示提示
    if (game_over_flag)
    {
        OLED_ShowString(24, 24, "GAME OVER", OLED_8X16);
    }

    OLED_Update();

    // 更新矩形区域供碰撞检测
    Update_Rects();
}

/* ---------------- 按键处理（处理按键，返回新页面） ---------------- */
page_state_t DinoGame_HandleKey(uint8_t key)
{
    if (key == 1)
    {
		game_active = 0;          //停止游戏逻辑
        exit_request = 0;
        return PAGE_GAME;         // 用户主动退出（返回列表）
    }
    else if (key == 3)
    {
        // 跳跃键：只有在地面且游戏未结束时才响应
        if (!game_over_flag && dino_jump_flag == 0)
        {
            dino_jump_flag = 1;
            jump_t = 0;          // 重新开始跳跃计时
        }
        // 如果游戏结束标志正在倒计时，按 Key3 可立即返回
        if (game_over_flag)
        {
            game_active = 0;
            exit_request = 0;
            return PAGE_GAME;     // 碰撞后按Key3立即返回
        }
    }
    return PAGE_DINO;   // 页面不变，继续游戏
}

/* ---------------- 定时更新（每10ms由TIM2中断调用） ---------------- */
void DinoGame_Tick(void)
{
	if (!game_active) return;   // 不在游戏页时直接返回

    if (exit_request) return;   // 已经请求退出，不再更新
	
    if (game_over_flag)
    {
        game_over_timer++;
        if (game_over_timer >= 150)   // 1.5秒后自动返回
        {
            game_over_flag = 0;
            game_over_timer = 0;
			exit_request = 1;         // 设置退出请求，等待主循环处理
		}
        return;
    }

    // 更新地面位置
    static uint8_t ground_count = 0;
    ground_count++;
    if (ground_count >= 2)   // 每20ms移动一次，速度适中
    {
        ground_count = 0;
        ground_pos++;
        if (ground_pos >= 256) ground_pos = 0;
    }

	static uint8_t barrier_speed_count = 0;
	// 更新障碍物位置（每 2 次 tick 移动一次，即 20ms 移动 1 像素）
	barrier_speed_count++;
	if (barrier_speed_count >= 2)   // 2 = 20ms，想更慢可改为 3 或 4
	{
		barrier_speed_count = 0;
		barrier_pos++;
		if (barrier_pos >= 143)
		{
			barrier_pos = 0;
			barrier_active = 1;
			barrier_type = rand() % 3;
		}
	}

    // 跳跃物理
    if (dino_jump_flag)
    {
        jump_t++;
		if (jump_t <= JUMP_TOTAL_TICKS)
		{
			// 使用半正弦波：jump_t=0时高度0，jump_t=JUMP_TOTAL_TICKS/2时最高，jump_t=JUMP_TOTAL_TICKS时回到0
			jump_height = (uint8_t)(30 * sin(3.14159 * jump_t / JUMP_TOTAL_TICKS));
		}
        else
        {
            dino_jump_flag = 0;   // 跳跃结束
            jump_t = 0;
            jump_height = 0;
        }
    }

    // 分数增加
    static uint8_t score_count = 0;
    score_count++;
    if (score_count >= 10)   // 每100ms加1分
    {
        score_count = 0;
        score++;
    }

    // 碰撞检测
    if (!game_over_flag && barrier_active)
    {
        Update_Rects();
        if (Check_Collision())
        {
            game_over_flag = 1;
            game_over_timer = 0;
            // 可选：触发蜂鸣器短响，但为了不干扰，先不做
        }
    }
}

/* ---------------- 碰撞检测 ---------------- */
static void Update_Rects(void)
{
    dino_rect.minX = DINO_X;
    dino_rect.maxX = DINO_X + DINO_WIDTH;
    dino_rect.minY = DINO_Y_GROUND - jump_height;
    dino_rect.maxY = dino_rect.minY + DINO_HEIGHT;

    barrier_rect.minX = 127 - barrier_pos;
    barrier_rect.maxX = barrier_rect.minX + BARRIER_WIDTH;
    barrier_rect.minY = BARRIER_Y;
    barrier_rect.maxY = BARRIER_Y + BARRIER_HEIGHT;
}

static uint8_t Check_Collision(void)
{
    if ((dino_rect.maxX > barrier_rect.minX) &&
        (dino_rect.minX < barrier_rect.maxX) &&
        (dino_rect.maxY > barrier_rect.minY) &&
        (dino_rect.minY < barrier_rect.maxY))
    {
        return 1;   // 碰撞
    }
    return 0;
}

uint8_t DinoGame_IsExitRequested(void)
{
    return exit_request;   // 返回1表示需要返回上一级
}

/*设置游戏是否激活*/
void DinoGame_SetActive(uint8_t active)
{
    game_active = active;
	if (!active)
    {
        exit_request = 0;   // 关键：停止游戏时同时清除退出请求标志
    }
}
