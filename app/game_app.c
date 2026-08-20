#include "stm32f10x.h"                  // Device header
#include "game_app.h"
#include "OLED.h"
#include "dino_game.h"

/* 游戏列表项（目前仅一个，以后扩展） */
static const char *game_items[] = {
    "Chrome Dino",
    // "Snake",  // 将来添加游戏名即可
};

#define GAME_ITEM_COUNT  1   // 当前只有 1 个游戏

static uint8_t selected_game = 0;   // 当前高亮项索引

void GameApp_Init(void)
{
    selected_game = 0;
	DinoGame_Init();
    DinoGame_SetActive(0);  // 初始未激活
}

/**
 * 绘制游戏相关页面
 */
void GameApp_Draw(page_state_t page)
{
    switch (page)
    {
        case PAGE_GAME:
        {
            // 绘制游戏选项
            for (int i = 0; i < GAME_ITEM_COUNT; i++)
            {
                OLED_ShowString(0, 16 + i * 16, (char *)game_items[i], OLED_8X16);
            }
            // 高亮当前选中行
            OLED_ReverseArea(0, 16 + selected_game * 16, 128, 16);
            break;
        }
        case PAGE_DINO:       DinoGame_Draw();break;
        default:
            break;
    }
}

/*处理游戏模块的按键*/
page_state_t GameApp_HandleKey(uint8_t key, page_state_t currentPage)
{
    // ---------- 游戏列表界面 ----------
    if (currentPage == PAGE_GAME)
    {
        if (key == 2)   // 向下选择（单项目循环，效果就是停在当前项）
        {
            selected_game = (selected_game + 1) % GAME_ITEM_COUNT;
            return PAGE_GAME;   // 仍在列表，需要重绘
        }
        else if (key == 3)   // 确认进入
        {
            switch (selected_game)
            {
                case 0: 
					DinoGame_Enter();   // 初始化并激活游戏
					return PAGE_DINO;   // 然后跳转页面
                // 以后 case 1: return PAGE_SNAKE;
                default: return PAGE_GAME;
            }
        }
        else if (key == 1)   // 返回主菜单
        {
            return PAGE_MENU;
        }
    }
    // ---------- 具体游戏界面（Dino 等） ----------
    else if (currentPage == PAGE_DINO)
    {
		// 如果游戏请求退出，立即返回列表
		if (DinoGame_IsExitRequested())
		{
			DinoGame_SetActive(0);   // 停止游戏
			return PAGE_GAME;
		}
		
        page_state_t newPage = DinoGame_HandleKey(key);
		if (newPage != PAGE_DINO)
		{
			// 离开游戏时确保停止游戏
			DinoGame_SetActive(0);
		}
		return newPage;
    }

    return currentPage;   // 未处理，页面不变
}
