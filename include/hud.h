#ifndef __HUD_H__
#define __HUD_H__

#include "gfc_types.h"
#include "gfc_shape.h"
#include "gfc_vector.h""
#include "gf2d_sprite.h"

typedef struct Hud_s{
	Sprite* buttonSprite;
	void (*button_update)(struct Hud_s* self);
}Hud;

void init_start_buttons();

void draw_all_huds();

void start_button1_update(Uint32 ms, int mx, int my);

void start_button2_update(Uint32 ms, int mx, int my);

void start_button3_update(Uint32 ms, int mx, int my);

void edit_button_update(Uint32 ms, int mx, int my);

void enable_start_menu();

void disable_start_menu();



#endif