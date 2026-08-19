#include "stm32f10x.h"                  // Device header
#include "menu.h"
#include "OLED.h"
#include "battery.h"
#include "flashlight.h"
#include "emoji.h"
#include "myRTC.h"
#include "time_app.h"
#include "time_setting.h"
#include "alarm.h"
#include "countdown.h"
#include "game_app.h"
#include "dino_game.h"

void Menu_Init(void)
{
	Flashlight_Init();
	Emoji_Init();
	TimeApp_Init();
	GameApp_Init();
	//后续可初始化其他功能
}

static page_state_t currentPage = PAGE_MAIN;
static const char *week_str[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

/*状态栏绘制（内部使用）*/
static void Menu_StatusBar(void)
{
    uint8_t hour   = (uint8_t)MyRTC_Time[3];
    uint8_t minute = (uint8_t)MyRTC_Time[4];
	uint8_t second = (uint8_t)MyRTC_Time[5];
    const char *week = week_str[MyRTC_Weekday];   // 获取星期缩写

    if (currentPage == PAGE_MAIN)
    {
        uint16_t year  = MyRTC_Time[0];
        uint8_t  month = MyRTC_Time[1];
        uint8_t  day   = MyRTC_Time[2];

        OLED_Printf(0, 5, OLED_6X8, "%04d-%02d-%02d", year, month, day);
        OLED_Printf(63, 5, OLED_6X8, "%s", week);
        OLED_Printf(85, 5, OLED_6X8, "%3d%%", Battery_GetPercent());
		OLED_ShowBattery(Battery_GetPercent());
    }
	else if (currentPage == PAGE_EMOJI)
	{
		OLED_Printf(85, 5, OLED_6X8, "%3d%%", Battery_GetPercent());
		OLED_ShowBattery(Battery_GetPercent());
		OLED_Printf(65, 20, OLED_8X16, "%02d:%02d:%02d", hour, minute, second);
		OLED_Printf(85, 40, OLED_8X16, "%s", week);
	}
    else
    {
        OLED_Printf(0, 5, OLED_6X8, "%02d:%02d:%02d", hour, minute, second);
        OLED_Printf(63, 5, OLED_6X8, "%s", week);
        OLED_Printf(85, 5, OLED_6X8, "%3d%%", Battery_GetPercent());
		OLED_ShowBattery(Battery_GetPercent());
    }
}

/************************************主界面***************************************/
/*显示主页时间(内部使用）*/
static void Menu_Main(void)
{
	uint8_t hour   = (uint8_t)MyRTC_Time[3];
    uint8_t minute = (uint8_t)MyRTC_Time[4];
    uint8_t second = (uint8_t)MyRTC_Time[5];
	
    OLED_Printf(22, 24, OLED_16X32, "%02d:%02d", hour, minute);
	OLED_Printf(104, 38, OLED_8X16, "%02d", second);
}

/***********************************菜单界面**************************************/
static const uint8_t *icon_list[] = {Clock,FlashLight,Game,Emoji};
static const uint8_t icon_count = 4;
static uint8_t menu_index = 0;                     //当前高亮（中间）的图标
static uint8_t prev_menu_index = 0;                //保存进入详情前的菜单高亮索引
static uint8_t menu_mode = MENU_MODE_PREVIEW;      //当前菜单模式
static uint8_t prev_menu_mode = MENU_MODE_PREVIEW; //保存进入详情前的菜单模式

/* 菜单滚动动画 */
static uint8_t menu_scroll_anim_active = 0;   // 1：动画进行中
static int16_t menu_scroll_offset = 0;        // 当前偏移量（像素）
static uint8_t menu_anim_type = 0;            // 0:无动画, 1:普通滚动, 2:从两图标到三图标

/*显示菜单界面(轮滑)(内部使用）*/
static void Menu_Select(void)
{
    if(menu_mode == MENU_MODE_PREVIEW || menu_mode == MENU_MODE_ACTIVE)
    {
        // 两图标模式：不变
        OLED_ShowImage(50,28,32,32,icon_list[0]);
        OLED_ShowImage(95,28,32,32,icon_list[1]);
        if(menu_mode == MENU_MODE_ACTIVE)
        {
            OLED_ReverseArea(48,28,36,34);
        }
    }
    else // MENU_MODE_SCROLL
    {
        uint8_t left_idx   = (menu_index - 1 + icon_count) % icon_count;
        uint8_t center_idx = menu_index;
        uint8_t right_idx  = (menu_index + 1) % icon_count;
        uint8_t next_right_idx = (menu_index + 2) % icon_count;

        if (menu_scroll_anim_active)
        {
            int16_t off = menu_scroll_offset;

            if (menu_anim_type == 2)  // 特殊动画：从两图标到三图标
			{
				// 只绘制中间、右、下一个右图标，不绘制左图标（左槽位由中间图标滑入）
				OLED_ShowImage(50 - off, 28, 32, 32, icon_list[center_idx]);
				OLED_ShowImage(95 - off, 28, 32, 32, icon_list[right_idx]);
				OLED_ShowImage(95 + MENU_ICON_SLOT_WIDTH - off, 28, 32, 32, icon_list[next_right_idx]);

				OLED_ReverseArea(48, 28, 36, 34);
			}
			else  // 普通滚动动画
			{	
				// 保持原有四图标绘制
				OLED_ShowImage(5 - off, 28, 32, 32, icon_list[left_idx]);
				OLED_ShowImage(50 - off, 28, 32, 32, icon_list[center_idx]);
				OLED_ShowImage(95 - off, 28, 32, 32, icon_list[right_idx]);
				OLED_ShowImage(95 + MENU_ICON_SLOT_WIDTH - off, 28, 32, 32, icon_list[next_right_idx]);

				OLED_ReverseArea(48, 28, 36, 34);
			}
        }
        else
        {
            // 无动画：正常三图标绘制
            OLED_ShowImage(5, 28, 32, 32, icon_list[left_idx]);
            OLED_ShowImage(50, 28, 32, 32, icon_list[center_idx]);
            OLED_ShowImage(95, 28, 32, 32, icon_list[right_idx]);
            OLED_ReverseArea(48, 28, 36, 34);
        }
    }
}

/*菜单滚动动画推进（每10ms由TIM2中断调用）*/
void Menu_Tick(void)
{
    if (menu_scroll_anim_active)
    {
        menu_scroll_offset += MENU_ANIM_STEP;
        if (menu_scroll_offset >= MENU_ICON_SLOT_WIDTH)
        {
            menu_scroll_offset = 0;
            menu_scroll_anim_active = 0;

            if (menu_anim_type == 1)   // 普通滚动：索引加1
            {
                menu_index = (menu_index + 1) % icon_count;
            }
            else if (menu_anim_type == 2)  // 特殊动画：索引变为1（中间图标为FlashLight）
            {
                menu_index = 1;
            }
            menu_anim_type = 0;
        }
    }
}

extern volatile uint8_t emoji_tick_flag;

/*菜单更新函数：处理按键 + 全屏重绘*/
void Menu_Update(uint8_t keynum)
{
	// 动画帧推进（针对表情页）
    if (emoji_tick_flag)
    {
        emoji_tick_flag = 0;
        if (currentPage == PAGE_EMOJI)
        {
            Emoji_UpdateFrame();   // 切换下一帧
        }
    }
	
	 // ========== 时间模块处理（列表 + 子界面） ==========
	if (currentPage >= PAGE_TIME && currentPage <= PAGE_COUNTDOWN)
	{
		page_state_t newPage = TimeApp_HandleKey(keynum, currentPage);
		if (newPage != currentPage)
		{
			currentPage = newPage;
			if (currentPage == PAGE_TIME_SETTING)
			{
				TimeSetting_Enter();   // 首次进入设置界面时加载当前时间
			}
			else if (currentPage == PAGE_ALARM)
			{
				Alarm_Enter();
			}
			else if (currentPage == PAGE_COUNTDOWN)
			{
				Countdown_Enter();
			}
		}
		goto draw;
	}

    // ========== 手电筒模块处理 ==========
    if (currentPage == PAGE_FLASHLIGHT)
    {
        if (keynum == 1)
        {
            currentPage = PAGE_MENU;
            menu_mode = prev_menu_mode;
            menu_index = prev_menu_index;
            goto draw;
        }
        if (Flashlight_ProcessKey(keynum))
        {
            goto draw;
        }
        goto draw;
    }
	
	// ========== 游戏模块处理（列表 + 具体游戏） ==========
    if (currentPage == PAGE_GAME || currentPage == PAGE_DINO)   // 后续添加其他游戏也包含进来
    {
		// 如果游戏请求退出，直接切换页面并停止游戏
		if (DinoGame_IsExitRequested())
		{
			DinoGame_SetActive(0);
			currentPage = PAGE_GAME;
			goto draw;
		}
		
        page_state_t newPage = GameApp_HandleKey(keynum, currentPage);
        if (newPage != currentPage)
        {
            currentPage = newPage;
        }
        goto draw;
    }

    // ========== 表情模块处理 ==========
    if (currentPage == PAGE_EMOJI)
    {
        if (keynum == 1)
        {
            currentPage = PAGE_MENU;
            menu_mode = prev_menu_mode;
            menu_index = prev_menu_index;
            goto draw;
        }
        goto draw;
    }
	
	/********************************************************/
	if(keynum == 1)
	{
		/*1.处理按键：主界面与菜单界面切换*/
		if(currentPage == PAGE_MAIN)
		{
			currentPage = PAGE_MENU;
			menu_mode = MENU_MODE_PREVIEW; //回到预览状态
			menu_index = 0;
		}
		else if(currentPage == PAGE_MENU)
		{
			currentPage = PAGE_MAIN;
		}
		//将来也可以定义该按键的其他用法
	}
	if(currentPage == PAGE_MENU) 
	{
		if(keynum == 2)
		{
			if(menu_mode == MENU_MODE_PREVIEW)
			{
				menu_mode = MENU_MODE_ACTIVE;   // 仅高亮，无滑动
			}
			else if(menu_mode == MENU_MODE_ACTIVE)
			{
				// 从两图标激活切换到三图标滚动，启动特殊动画
				menu_mode = MENU_MODE_SCROLL;
				menu_index = 0;                 // 动画起始：中间为 Clock (索引0)
				menu_scroll_anim_active = 1;
				menu_scroll_offset = 0;
				menu_anim_type = 2;             // 特殊动画类型
			}
			else if(menu_mode == MENU_MODE_SCROLL)
			{
				// 只有在动画未激活时才启动普通滚动动画
				if (!menu_scroll_anim_active)
				{
					menu_scroll_anim_active = 1;
					menu_scroll_offset = 0;
					menu_anim_type = 1;         // 普通滚动动画
				}
			}
		}
		else if(keynum == 3)
		{
			if(menu_mode == MENU_MODE_ACTIVE || menu_mode == MENU_MODE_SCROLL)
			{
				prev_menu_mode = menu_mode;
				prev_menu_index = (menu_mode == MENU_MODE_ACTIVE)? 0: menu_index;
				switch(prev_menu_index){
					case 0: currentPage = PAGE_TIME;break;
					case 1: currentPage = PAGE_FLASHLIGHT;break;
					case 2: currentPage = PAGE_GAME;break;
					case 3: currentPage = PAGE_EMOJI;break;
				}
			}
		}
	}
draw:
	/*2.全屏绘制：清屏+状态栏+内容*/
	OLED_Clear();
	Menu_StatusBar();
	switch(currentPage){
		case PAGE_MAIN:           Menu_Main();break;
		case PAGE_MENU:           Menu_Select();break;
		
		case PAGE_FLASHLIGHT:     Detail_Flashlight();break;
		case PAGE_EMOJI:          Detail_Emoji();break;	
		
		// 时间模块全部交给 TimeApp_Draw
        case PAGE_TIME:
        case PAGE_TIME_SETTING:
        case PAGE_ALARM:
        case PAGE_COUNTDOWN:
            TimeApp_Draw(currentPage);break;
		// 游戏模块全部交给 GameApp_Draw
		case PAGE_GAME:
		case PAGE_DINO:
			GameApp_Draw(currentPage);break;
	}
	/*3.刷新屏幕*/
	OLED_Update();
}
