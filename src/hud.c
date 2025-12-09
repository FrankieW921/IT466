#include "simple_logger.h"

#include "hud.h"
#include "game.h"

static Hud* startButton1;
static Hud* startButton2;
static Hud* startButton3;
static Hud* editButton;

static Uint8 doDrawPartsHuds;

static Uint8 startMenu = 1;


void init_start_buttons() {
	startButton1 = gfc_allocate_array(sizeof(Hud), 1);
	startButton2 = gfc_allocate_array(sizeof(Hud), 1);
	startButton3 = gfc_allocate_array(sizeof(Hud), 1);
	editButton = gfc_allocate_array(sizeof(Hud), 1);

	startButton1->buttonSprite = gf2d_sprite_load_image("images/start1.png");
	startButton2->buttonSprite = gf2d_sprite_load_image("images/start2.png");
	startButton3->buttonSprite = gf2d_sprite_load_image("images/start3.png");
	editButton->buttonSprite = gf2d_sprite_load_image("images/edit.png");

	startButton1->button_update = start_button1_update;
	startButton2->button_update = start_button2_update;
	startButton3->button_update = start_button3_update;
	editButton->button_update = edit_button_update;
}

void draw_all_huds() {
	Uint32 mouseState;
	int mx, my;
	mouseState = SDL_GetMouseState(&mx, &my);

	start_button1_update(mouseState, mx, my);
	start_button2_update(mouseState, mx, my);
	start_button3_update(mouseState, mx, my);
	edit_button_update(mouseState, mx, my);
}

void start_button1_update(Uint32 ms, int mx, int my) {
	if (!startButton1) return;
	gf2d_sprite_draw_image(startButton1->buttonSprite, gfc_vector2d(0, 0));
	if ((ms & 1) && (mx >= 0 && mx < 200) && (my >= 0 && my < 100) && (startMenu == 1)) {
		start_game(1);
	}
}

void start_button2_update(Uint32 ms, int mx, int my) {
	if (!startButton2) return;
	gf2d_sprite_draw_image(startButton2->buttonSprite, gfc_vector2d(0, 100));
	if ((ms & 1) && (mx >= 200 && mx < 400) && (my >= 100 && my < 200) && (startMenu == 1)) {
		start_game(2);
	}
}

void start_button3_update(Uint32 ms, int mx, int my) {
	if (!startButton3) return;
	gf2d_sprite_draw_image(startButton3->buttonSprite, gfc_vector2d(0, 200));
	if ((ms & 1) && (mx >= 400 && mx < 600) && (my >= 200 && my < 300) && (startMenu == 1)) {
		start_game(3);
	}
}

void edit_button_update(Uint32 ms, int mx, int my) {
	if (!editButton) return;
	gf2d_sprite_draw_image(editButton->buttonSprite, gfc_vector2d(0, 300));
	if ((ms & 1) && (mx >= 600 && mx < 800) && (my >= 300 && my < 400) && (startMenu == 1)) {
		start_edit();
	}
}

void enable_start_menu() {
	startMenu = 1;
}

void disable_start_menu() {
	startMenu = 0;
}