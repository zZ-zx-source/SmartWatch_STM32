#include "stm32f10x.h"                  // Device header
#include "dino_game.h"
#include "OLED.h"
#include <stdio.h>     //sprintf
#include <stdlib.h>    // rand
#include <math.h>      // sin

/* ---------------- ��Ϸ״̬ ---------------- */
typedef struct {
    uint8_t minX, minY, maxX, maxY;
} Rect;

static uint8_t exit_request = 0;   // �����˳���־
static uint8_t game_active = 0;    // ��Ϸ�Ƿ񼤻��ǰ�Ƿ�����Ϸҳ��

static int score = 0;
static uint16_t ground_pos = 0;
static uint16_t barrier_pos = 0;
static uint8_t barrier_type = 0;
static uint8_t barrier_active = 1;      // �ϰ����Ƿ�����Ļ��

static uint8_t dino_jump_flag = 0;      // 0:����, 1:��Ծ
static uint16_t jump_t = 0;
static uint8_t jump_height = 0;         // ��ǰ��Ծ�߶ȣ����أ�

static uint8_t game_over_flag = 0;      // 1: ��ײ�������ȴ���ʱ����
static uint16_t game_over_timer = 0;    // Game Over ͣ����ʱ (��λ:10ms)

static Rect dino_rect, barrier_rect;

/* ---------------- �ڲ��������� ---------------- */
static void Update_Rects(void);
static uint8_t Check_Collision(void);

/* ---------------- ��ʼ�� ---------------- */
void DinoGame_Init(void)
{
    score = 0;
    ground_pos = 0;
    barrier_pos = 128;   //��ʼ���ϰ�����ȫ����Ļ�⣨x=127-128=-1?��
    barrier_active = 1;
    barrier_type = 0;
    dino_jump_flag = 0;
    jump_t = 0;
    jump_height = 0;
    game_over_flag = 0;
    game_over_timer = 0;
    exit_request = 0;
    game_active = 0;
    srand(1234);   // ������ӣ��ɸ�Ϊʹ�� RTC ��������
}

void DinoGame_Enter(void)
{
    DinoGame_Init();      // ������Ϸʱ��������״̬
	game_active = 1;      // �����Ϸ��������
    exit_request = 0;     // ����˳�����
}

/* ---------------- ���ƺ��������Ƶ�ǰ֡��---------------- */
void DinoGame_Draw(void)
{
//    // ������ȫ���ػ棩
//    OLED_Clear();//����Ҫ����Ϊ��Menu_Update �Ѿ�ͳһִ���� OLED_Clear()

    // ��ʾ�÷֣����Ͻǣ�y ������ 20��
	OLED_ShowNum(98, 20, score, 5, OLED_6X8);

    // 2. ���Ƶ��棨����Ч����ֱ�ӻ�һ�����ƶ��ĵ��棩
    //    �򻯣��� OLED_ShowImage ��һ���̶����ȵĵ��棬���� ground_pos ƫ�ƻ���
    //    ������һ�� 16x8 �ĵ��������ظ����ƻ�ʹ�� ShowImage ƫ��
    for (int x = - (ground_pos % 16); x < 128; x += 16)
    {
        OLED_ShowImage(x, GROUND_Y, 16, GROUND_HEIGHT, Ground);
    }

    // 3. �����ϰ�������ƣ�
    if (barrier_active)
    {
        OLED_ShowImage(127 - barrier_pos, BARRIER_Y, BARRIER_WIDTH, BARRIER_HEIGHT,
                       (barrier_type == 0) ? Cactus1 : (barrier_type == 1) ? Cactus2 : Cactus3);
    }

    // 5. ����С����
    if (dino_jump_flag)
    {
        OLED_ShowImage(DINO_X, DINO_Y_GROUND - jump_height, DINO_WIDTH, DINO_HEIGHT, Dino_Jump);
    }
    else
    {
        // ���ܶ�������֡���棬�� ground_pos ��ż����
        if ((ground_pos / 8) % 2 == 0)
            OLED_ShowImage(DINO_X, DINO_Y_GROUND, DINO_WIDTH, DINO_HEIGHT, Dino_Run1);
        else
            OLED_ShowImage(DINO_X, DINO_Y_GROUND, DINO_WIDTH, DINO_HEIGHT, Dino_Run2);
    }

    // 6. ��� Game Over����ʾ��ʾ
    if (game_over_flag)
    {
        OLED_ShowString(24, 24, "GAME OVER", OLED_8X16);
    }

    OLED_Update();

    // ���¾���������ײ���
    Update_Rects();
}

/* ---------------- ��������������������������ҳ�棩 ---------------- */
page_state_t DinoGame_HandleKey(uint8_t key)
{
    if (key == 1)
    {
		game_active = 0;          //ֹͣ��Ϸ�߼�
        exit_request = 0;
        return PAGE_GAME;         // �û������˳��������б���
    }
    else if (key == 3)
    {
        // ��Ծ����ֻ���ڵ�������Ϸδ����ʱ����Ӧ
        if (!game_over_flag && dino_jump_flag == 0)
        {
            dino_jump_flag = 1;
            jump_t = 0;          // ���¿�ʼ��Ծ��ʱ
        }
        // �����Ϸ������־���ڵ���ʱ���� Key3 ����������
        if (game_over_flag)
        {
            game_active = 0;
            exit_request = 0;
            return PAGE_GAME;     // ��ײ��Key3��������
        }
    }
    return PAGE_DINO;   // ҳ�治�䣬������Ϸ
}

/* ---------------- ��ʱ���£�ÿ10ms��TIM2�жϵ��ã� ---------------- */
void DinoGame_Tick(void)
{
	if (!game_active) return;   // ������Ϸҳʱֱ�ӷ���

    if (exit_request) return;   // �Ѿ������˳������ٸ���
	
    if (game_over_flag)
    {
        game_over_timer++;
        if (game_over_timer >= 150)   // 1.5����Զ�����
        {
            game_over_flag = 0;
            game_over_timer = 0;
			exit_request = 1;         // �����˳����󣬵ȴ���ѭ������
		}
        return;
    }

    // ���µ���λ��
    static uint8_t ground_count = 0;
    ground_count++;
    if (ground_count >= 2)   // ÿ20ms�ƶ�һ�Σ��ٶ�����
    {
        ground_count = 0;
        ground_pos++;
        if (ground_pos >= 256) ground_pos = 0;
    }

	static uint8_t barrier_speed_count = 0;
	// �����ϰ���λ�ã�ÿ 2 �� tick �ƶ�һ�Σ��� 20ms �ƶ� 1 ���أ�
	barrier_speed_count++;
	if (barrier_speed_count >= 2)   // 2 = 20ms��������ɸ�Ϊ 3 �� 4
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
            dino_jump_flag = 0;   // 跳跃结束            jump_t = 0;
            jump_height = 0;
        }
    }

    // ��������
    static uint8_t score_count = 0;
    score_count++;
    if (score_count >= 10)   // ÿ100ms��1��
    {
        score_count = 0;
        score++;
    }

    // ��ײ���
    if (!game_over_flag && barrier_active)
    {
        Update_Rects();
        if (Check_Collision())
        {
            game_over_flag = 1;
            game_over_timer = 0;
            // ��ѡ���������������죬��Ϊ�˲����ţ��Ȳ���
        }
    }
}

/* ---------------- ��ײ��� ---------------- */
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
        return 1;   // ��ײ
    }
    return 0;
}

uint8_t DinoGame_IsExitRequested(void)
{
    return exit_request;   // ����1��ʾ��Ҫ������һ��
}

/*������Ϸ�Ƿ񼤻�*/
void DinoGame_SetActive(uint8_t active)
{
    game_active = active;
	if (!active)
    {
        exit_request = 0;   // �ؼ���ֹͣ��Ϸʱͬʱ����˳������־
    }
}
