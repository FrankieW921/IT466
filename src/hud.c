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

	start_button1_update(mx, my);
	start_button2_update(mx, my);
	start_button3_update(mx, my);
	edit_button_update(mx, my);
}

void start_button1_update(int mx, int my) {
	if (!startButton1) return;
}

void start_button2_update(int mx, int my) {
	if (!startButton2) return;
}

void start_button3_update(int mx, int my) {
	if (!startButton3) return;
}

void edit_button_update(int mx, int my) {
	if (!editButton) return;
}