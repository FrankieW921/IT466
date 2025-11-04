#include "simple_logger.h"
#include "gfc_input.h"

#include "player.h"
#include "camera_entity.h"

static Entity* thePlayer;

Entity* get_the_player() {
	if (!thePlayer) {
		slog("No static player to return");
		return NULL;
	}
	return thePlayer;
}

Entity* player_spawn(GFC_Vector3D position, GFC_Color color) {
	Entity* self;
	PlayerData* data;

	self = entity_new();
	if (!self)return;

	data = gfc_allocate_array(sizeof(PlayerData), 1);
	gfc_line_cpy(self->name, "Player");
	self->mesh = gf3d_mesh_load("models/enemy4/enemy4.obj");
	self->texture = gf3d_texture_load("models/enemy4/enemy4.png");
	self->color = color;
	self->position = position;
	self->rotation = gfc_vector3d(0, 0, -2 * GFC_PI);
	//entity defaults to scale of 1, 1, 1
	self->think = player_think;
	self->update = player_update;

	player_data_new(data);
	self->data = data;

	thePlayer = self; //assign static variable
	return self;
}

void player_think(Entity* self) {
	Uint32 mouseState;
	PlayerData* data;

	int mx, my;
	GFC_Vector2D direction2d;
	float move = 0; 
	float moveStep = .35;

	if (!self) return;
	//data = self->data;
	//if (!data) return;

	//self->velocity.x = 0;
	//self->velocity.y = 0;
	self->velocity.z = 0;

	if (gfc_input_command_down("panleft")) {
		self->rotation.z += .1;
	}
	if (gfc_input_command_down("panright")) {
		self->rotation.z -= .1;
	}

	direction2d = gfc_vector2d_from_angle(self->rotation.z);
	gfc_vector2d_normalize(&direction2d);
	if (gfc_input_command_down("moveforward")) {
		move += moveStep;
	}
	if (gfc_input_command_down("moveback")) {
		move -= moveStep;
	}
	if (move) {
		gfc_vector2d_scale(direction2d, direction2d, move);
		gfc_vector2d_add(self->velocity, self->velocity, direction2d);
	}
	move = 0;
	direction2d = gfc_vector2d_from_angle(self->rotation.z);
	gfc_vector2d_normalize(&direction2d);
	direction2d = gfc_vector2d_rotate(direction2d, GFC_HALF_PI);
	if (gfc_input_command_down("moveright")) {
		move -= moveStep;
	}
	if (gfc_input_command_down("moveleft")) {
		move += moveStep;
	}
	if (move) {
		gfc_vector2d_scale(direction2d, direction2d, move);
		gfc_vector2d_add(self->velocity, self->velocity, direction2d);
	}
	if (gfc_input_command_down("jump")) {
		self->velocity.z += 1;
	}
	if (gfc_input_command_down("crouch")) {
		self->velocity.z -= 1;
	}

	mouseState = SDL_GetMouseState(&mx, &my);
}

void player_update(Entity* self) {
	PlayerData* data;

	if (!self) return;
	//data = self->data;
	//if (!data) return;

	entity_move(self); 

	self->bounds.x = self->position.x;
	self->bounds.y = self->position.y;
	self->bounds.z = self->position.z;
}

void player_data_new(PlayerData* data) { //hardcode the stuff for now
	Head* head;
	Arm* arm;
	Body* body;
	Leg* leg;
	SJson* defArray, *partDef;

	//data = gfc_allocate_array(sizeof(PlayerData), 1); done in player spawn, change?
	data->leg = gfc_allocate_array(sizeof(Leg), 1); //player personal parts
	data->body = gfc_allocate_array(sizeof(Body), 1);
	data->arm = gfc_allocate_array(sizeof(Arm), 1);
	data->head = gfc_allocate_array(sizeof(Head), 1);

	leg = gfc_allocate_array(sizeof(Leg), 1); //general part pointers
	body = gfc_allocate_array(sizeof(Body), 1);
	arm = gfc_allocate_array(sizeof(Arm), 1);
	head = gfc_allocate_array(sizeof(Head), 1);

	data->heads = sj_load("defs/player/arms.def");
	data->arms = sj_load("defs/player/arms.def");
	data->bodies = sj_load("defs/player/bodies.def");
	data->legs = sj_load("defs/player/legs.def");

	defArray = sj_object_get_value(data->heads, "heads");
	partDef = sj_array_get_nth(defArray, 0);
	

	defArray = sj_object_get_value(data->arms, "arms");


	defArray = sj_object_get_value(data->bodies, "bodies");


	defArray = sj_object_get_value(data->legs, "legs");

	data->headInventory = gfc_list_new();
	data->armInventory = gfc_list_new();
	data->bodyInventory = gfc_list_new();
	data->legInventory = gfc_list_new();
}

void player_set_head(Head* currentHead, SJson* selectedHead) {
	const char* meshPath;
	const char* texturePath;
	if (!currentHead) {
		return;
	}
	strcpy(currentHead->name, sj_object_get_value_as_string(selectedHead, "name"));
	sj_object_get_value_as_int(selectedHead, "health", &currentHead->health);
	meshPath = sj_object_get_value_as_string(selectedHead, "mesh");
	texturePath = sj_object_get_value_as_string(selectedHead, "texture");

	/*
	if (currentHead->headSprite) {
		gf2d_sprite_free(currentHead->headSprite);
	}
	currentHead->headSprite = gf2d_sprite_load_all(
		imageString,
		32,
		32,
		0,
		0
	);
	*/
}

void player_draw(Entity* self, GFC_Vector3D lightPos, GFC_Color colorMod) {

}