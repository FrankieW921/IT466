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
	//self->mesh = gf3d_mesh_load("models/enemy4/enemy4.obj");
	//self->texture = gf3d_texture_load("models/enemy4/enemy4.png");
	self->color = color;
	self->position = position;
	self->rotation = gfc_vector3d(0, 0, 0);
	//entity defaults to scale of 1, 1, 1
	self->think = player_think;
	self->update = player_update;
	self->draw = player_draw;

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
	SJson* defArray, *part;

	//data = gfc_allocate_array(sizeof(PlayerData), 1); done in player spawn, change?
	data->leg = gfc_allocate_array(sizeof(Leg), 1); //player personal parts
	data->body = gfc_allocate_array(sizeof(Body), 1);
	data->arm = gfc_allocate_array(sizeof(Arm), 1);
	data->head = gfc_allocate_array(sizeof(Head), 1);
	data->gun = gfc_allocate_array(sizeof(Weapon), 1);
	data->shoulder = gfc_allocate_array(sizeof(Weapon), 1);

	leg = gfc_allocate_array(sizeof(Leg), 1); //general part pointers (might not need);
	body = gfc_allocate_array(sizeof(Body), 1);
	arm = gfc_allocate_array(sizeof(Arm), 1);
	head = gfc_allocate_array(sizeof(Head), 1);

	data->headInventory = gfc_list_new();
	data->armInventory = gfc_list_new();
	data->bodyInventory = gfc_list_new();
	data->legInventory = gfc_list_new();
	data->gunInventory = gfc_list_new();
	data->shoulderInventory = gfc_list_new();

	data->heads = sj_load("defs/player/heads.def");
	data->arms = sj_load("defs/player/arms.def");
	data->bodies = sj_load("defs/player/bodies.def");
	data->legs = sj_load("defs/player/legs.def");
	data->guns = sj_load("defs/player/guns.def");
	data->shoulders = sj_load("defs/player/shoulders.def");

	defArray = sj_object_get_value(data->heads, "heads");
	part = sj_array_get_nth(defArray, 2);
	player_set_head(data->head, part); 

	defArray = sj_object_get_value(data->arms, "arms");
	part = sj_array_get_nth(defArray, 2);
	player_set_arm(data->arm, part);

	defArray = sj_object_get_value(data->bodies, "bodies");
	part = sj_array_get_nth(defArray, 1);
	player_set_body(data->body, part);

	defArray = sj_object_get_value(data->legs, "legs");
	part = sj_array_get_nth(defArray, 2);
	player_set_leg(data->leg, part);

	defArray = sj_object_get_value(data->guns, "guns");
	part = sj_array_get_nth(defArray, 1);
	player_set_weapon(data->gun, part);

	defArray = sj_object_get_value(data->shoulders, "shoulders");
	part = sj_array_get_nth(defArray, 2);
	player_set_weapon(data->shoulder, part);
}

void player_draw(Entity* self, GFC_Vector3D lightPos, GFC_Color colorMod) {
	PlayerData* pData;
	GFC_Matrix4 modelMat;

	if (!self) return NULL;
	pData = self->data;
	if (!pData) return NULL;

	gfc_matrix4_from_vectors(modelMat, self->position, self->rotation, self->scale);
	gf3d_mesh_draw(pData->head->headMesh, modelMat, self->color, pData->head->headTexture, lightPos, colorMod);
	gf3d_mesh_draw(pData->arm->armMesh, modelMat, self->color, pData->arm->armTexture, lightPos, colorMod);
	gf3d_mesh_draw(pData->body->bodyMesh, modelMat, self->color, pData->body->bodyTexture, lightPos, colorMod);
	gf3d_mesh_draw(pData->leg->legMesh, modelMat, self->color, pData->leg->legTexture, lightPos, colorMod);
	gf3d_mesh_draw(pData->gun->weaponMesh, modelMat, self->color, pData->gun->weaponTexture, lightPos, colorMod);
	gf3d_mesh_draw(pData->shoulder->weaponMesh, modelMat, self->color, pData->shoulder->weaponTexture, lightPos, colorMod);
}

void player_set_head(Head* currentHead, SJson* selectedHead) {
	const char* meshPath;
	const char* texturePath;
	if (!currentHead || !selectedHead) {
		return;
	}
	strcpy(currentHead->name, sj_object_get_value_as_string(selectedHead, "name"));
	sj_object_get_value_as_int(selectedHead, "health", &currentHead->health);
	meshPath = sj_object_get_value_as_string(selectedHead, "mesh");
	texturePath = sj_object_get_value_as_string(selectedHead, "texture");
	if (currentHead->headMesh) {
		gf3d_mesh_free(currentHead->headMesh);
	}
	if (currentHead->headTexture) {
		gf3d_texture_free(currentHead->headTexture);
	}
	currentHead->headMesh = gf3d_mesh_load(meshPath);
	currentHead->headTexture =  gf3d_texture_load(texturePath);
}

void player_set_arm(Arm* currentArm, SJson* selectedArm) {
	const char* meshPath;
	const char* texturePath;
	if (!currentArm || !selectedArm) {
		return;
	}
	strcpy(currentArm->name, sj_object_get_value_as_string(selectedArm, "name"));
	sj_object_get_value_as_int(selectedArm, "health", &currentArm->health);
	meshPath = sj_object_get_value_as_string(selectedArm, "mesh");
	texturePath = sj_object_get_value_as_string(selectedArm, "texture");
	if (currentArm->armMesh) {
		gf3d_mesh_free(currentArm->armMesh);
	}
	if (currentArm->armTexture) {
		gf3d_texture_free(currentArm->armTexture);
	}
	currentArm->armMesh = gf3d_mesh_load(meshPath);
	currentArm->armTexture = gf3d_texture_load(texturePath);
}

void player_set_body(Body* currentBody, SJson* selectedBody) {
	const char* meshPath;
	const char* texturePath;
	if (!currentBody || !selectedBody) {
		return;
	}
	strcpy(currentBody->name, sj_object_get_value_as_string(selectedBody, "name"));
	sj_object_get_value_as_int(selectedBody, "health", &currentBody->health);
	meshPath = sj_object_get_value_as_string(selectedBody, "mesh");
	texturePath = sj_object_get_value_as_string(selectedBody, "texture");
	if (currentBody->bodyMesh) {
		gf3d_mesh_free(currentBody->bodyMesh);
	}
	if (currentBody->bodyTexture) {
		gf3d_texture_free(currentBody->bodyTexture);
	}
	currentBody->bodyMesh = gf3d_mesh_load(meshPath);
	currentBody->bodyTexture = gf3d_texture_load(texturePath);
}

void player_set_leg(Leg* currentLeg, SJson* selectedLeg) {
	const char* meshPath;
	const char* texturePath;
	if (!currentLeg || !selectedLeg) {
		return;
	}
	strcpy(currentLeg->name, sj_object_get_value_as_string(selectedLeg, "name"));
	sj_object_get_value_as_int(selectedLeg, "health", &currentLeg->health);
	meshPath = sj_object_get_value_as_string(selectedLeg, "mesh");
	texturePath = sj_object_get_value_as_string(selectedLeg, "texture");
	if (currentLeg->legMesh) {
		gf3d_mesh_free(currentLeg->legMesh);
	}
	if (currentLeg->legTexture) {
		gf3d_texture_free(currentLeg->legTexture);
	}
	currentLeg->legMesh = gf3d_mesh_load(meshPath);
	currentLeg->legTexture = gf3d_texture_load(texturePath);
}

void player_set_weapon(Weapon* currentWeapon, SJson* selectedWeapon) {
	const char* meshPath;
	const char* texturePath;
	if (!currentWeapon || !selectedWeapon) return;

	strcpy(currentWeapon->name, sj_object_get_value_as_string(selectedWeapon, "name"));
	sj_object_get_value_as_int(selectedWeapon, "damage", &currentWeapon->damage);
	sj_object_get_value_as_int(selectedWeapon, "cooldown", &currentWeapon->cooldown);
	meshPath = sj_object_get_value_as_string(selectedWeapon, "mesh");
	texturePath = sj_object_get_value_as_string(selectedWeapon, "texture");
	if (currentWeapon->weaponMesh) {
		gf3d_mesh_free(currentWeapon->weaponMesh);
	}
	if (currentWeapon->weaponTexture) {
		gf3d_texture_free(currentWeapon->weaponTexture);
	}
	currentWeapon->weaponMesh = gf3d_mesh_load(meshPath);
	currentWeapon->weaponTexture = gf3d_texture_load(texturePath);
}

void player_add_head(PlayerData* pData, SJson* headToAdd) {
	const char* meshPath;
	const char* texturePath;
	Head* head;
	if (!pData || !headToAdd) return NULL;
	head = gfc_allocate_array(sizeof(Head), 1);

	strcpy(head->name, sj_object_get_value_as_string(headToAdd, "name"));
	sj_object_get_value_as_int(headToAdd, "health", &head->health);
	meshPath = sj_object_get_value_as_string(headToAdd, "mesh");
	texturePath = sj_object_get_value_as_string(headToAdd, "texture");
	head->headMesh = gf3d_mesh_load(meshPath);
	head->headTexture = gf3d_texture_load(texturePath);
	gfc_list_append(pData->headInventory, head);
	pData->headIndexMax = (Uint8)gfc_list_count(pData->headInventory);
}