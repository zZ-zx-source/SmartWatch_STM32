#include "stm32f10x.h"                  // Device header
#include "emoji.h"
#include "OLED.h"

#define EMOJI_FRAME_COUNT  30

static const uint8_t *frame_list[EMOJI_FRAME_COUNT] = {
    Emoji_Frame0,
    Emoji_Frame1,
    Emoji_Frame2,
    Emoji_Frame3,
	Emoji_Frame4,
	Emoji_Frame5,
	Emoji_Frame6,
	Emoji_Frame7,
	Emoji_Frame8,
	Emoji_Frame9,
	Emoji_Frame10,
	Emoji_Frame11,
	Emoji_Frame12,
	Emoji_Frame13,
	Emoji_Frame14,
	Emoji_Frame15,
	Emoji_Frame16,
	Emoji_Frame17,
	Emoji_Frame18,
	Emoji_Frame19,
	Emoji_Frame20,
	Emoji_Frame21,
	Emoji_Frame22,
	Emoji_Frame23,
	Emoji_Frame24,
	Emoji_Frame25,
	Emoji_Frame26,
	Emoji_Frame27,
	Emoji_Frame28,
    Emoji_Frame29
};

static uint8_t current_frame = 0;

void Emoji_Init(void) {
    current_frame = 0;
}

void Detail_Emoji(void) {
    OLED_ShowImage(0, 0, 64, 64, frame_list[current_frame]);
}

void Emoji_UpdateFrame(void) {
    current_frame = (current_frame + 1) % EMOJI_FRAME_COUNT;
}
