#include "simple_logger.h"
#include "gfc_input.h"

#include "gf2d_font.h"

#include "player.h"
#include "world.h"
#include "camera_entity.h"

static Entity* thePlayer;

static Uint8 partSwapCooldown = 0;

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
	player_do_max_health(self);

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
	data = self->data;
	if (!data) return;

	if (partSwapCooldown > 0) {
		partSwapCooldown -= 1;
	}

	//rotate player
	if (gfc_input_command_down("panleft")) {
		self->rotation.z += .1;
	}
	if (gfc_input_command_down("panright")) {
		self->rotation.z -= .1;
	}
	//x and y movement of player
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
	//player jumping/flying
	if (data->movementState == MS_FLYING) data->movementState = MS_FALLING;
	if (gfc_input_command_down("jump")) {
		data->movementState = MS_FLYING;
	}

	//change z velocity based on MS
	switch (data->movementState) {
		case MS_ON_GROUND:
			self->velocity.z = 0;
			break;
		case MS_FALLING:
			self->velocity.z -= .05;
			if (self->velocity.z < -3) self->velocity.z = -3;
			break;
		case MS_FLYING:
			self->velocity.z += .03;
			if (self->velocity.z > 2) self->velocity.z = 2;
			break;
	}

	if (gfc_input_command_down("nextHead") && partSwapCooldown == 0) {
		player_next_head(self);
		partSwapCooldown = 60;
	}
	if (gfc_input_command_down("nextArm") && partSwapCooldown == 0) {
		player_next_arm(self);
		partSwapCooldown = 60;
	}
	if (gfc_input_command_down("nextBody") && partSwapCooldown == 0) {
		player_next_body(self);
		partSwapCooldown = 60;
	}
	if (gfc_input_command_down("nextLeg") && partSwapCooldown == 0) {
		player_next_leg(self);
		partSwapCooldown = 60;
	}
	if (gfc_input_command_down("nextGun") && partSwapCooldown == 0) {
		player_next_gun(self);
		partSwapCooldown = 60;
	}
	if (gfc_input_command_down("nextShoulder") && partSwapCooldown == 0) {
		player_next_shoulder(self);
		partSwapCooldown = 60;
	}

	mouseState = SDL_GetMouseState(&mx, &my);
}

void player_update(Entity* self) {
	PlayerData* data;

	if (!self) return;
	//data = self->data;
	//if (!data) return;

	player_move(self); 

	self->bounds.x = self->position.x;
	self->bounds.y = self->position.y;
	self->bounds.z = self->position.z;
}

void player_move(Entity* self) {
	GFC_Box bounds;
	PlayerData* data;
	GFC_Vector3D positionPre, positionPost, contact;
	GFC_Vector2D direction2d;
	if (!self) return;
	data = self->data;
	if (!data) return;

	direction2d = gfc_vector2d_from_angle(self->rotation.z);
	gfc_vector2d_normalize(&direction2d);
	gfc_vector3d_copy(positionPre, self->position);
	gfc_vector3d_add(positionPost, self->position, self->velocity);
	
	switch(data->movementState){
		case MS_ON_GROUND:
			if (world_edge_test(get_the_world(), positionPre, positionPost, &contact)){ //wall touch
				self->position.z = entity_floor_check(self) + .01;
				//slog("CONTACT: %f %f %f", contact.x, contact.y, contact.z);
			}
			else if (positionPre.z - entity_floor_check(self) > .02) { //walked off a ledge
				data->movementState = MS_FALLING;
				self->position.x = positionPost.x;
				self->position.y = positionPost.y;
			}
			else { //not touching wall
				gfc_vector3d_copy(self->position, positionPost);
				//self->position.z = entity_floor_check(self) + .01;
			}
			break;
		case MS_FALLING:
			if (world_edge_test(get_the_world(), positionPre, positionPost, &contact)) {
				if (contact.z == entity_floor_check(self)) { //you touched the ground, skips a frame of movement i think but its okay
					data->movementState = MS_ON_GROUND;
					self->position.z = entity_floor_check(self) + .01;
				}
				else { //you touched a wall while falling
					self->position.z += self->velocity.z;
				}
			}
			else if (entity_floor_check(self) == -99999) { //no floor below
				gfc_vector3d_copy(self->position, positionPost);
				if (entity_roof_check(self) < 99999) { //check that there's a cieling to snap to
					self->position.z = entity_roof_check(self) + .01;
					data->movementState = MS_ON_GROUND;
				}
			}
			else { //freefall
				gfc_vector3d_copy(self->position, positionPost);
			}
			break;
		case MS_FLYING:
			if (world_edge_test(get_the_world(), positionPre, positionPost, &contact)) {
				if (contact.z == entity_roof_check(self)) { //touched the roof
					//no movement
					self->position.z = entity_roof_check(self) - .1;
				}
				else if (contact.z == entity_floor_check(self)) { //was falling when you started to fly and the momentum still carried you to touch the ground
					self->position.z = entity_floor_check(self) + .1;
					self->velocity.z = 0;
				}
				else { //touched a wall while flying
					self->position.z += self->velocity.z;
				}
			}
			else { //freefly
				gfc_vector3d_copy(self->position, positionPost);
			}
			break;
	}
	/*
	if (world_edge_test(get_the_world(), positionPre, positionPost, &contact)) {
		slog("CONTACT: %f %f %f", contact.x, contact.y, contact.z);
	}
	else {
		gfc_vector3d_copy(self->position, positionPost);
	}
	*/
	gfc_vector2d_scale(self->velocity, self->velocity, .90);
	if (self->velocity.x < .05 && self->velocity.x > -.05)self->velocity.x = 0;
	if (self->velocity.y < .05 && self->velocity.y > -.05)self->velocity.y = 0;

	gfc_box_cpy(bounds, self->bounds); //start of collision checking
	gfc_vector3d_add(bounds, bounds, self->velocity);
}

void player_data_new(PlayerData* data) { //hardcode the stuff for now
	Head* headG; //generics for setting
	Arm* armG;
	Body* bodyG;
	Leg* legG;
	SJson* defArray, *part;

	data->movementState = MS_ON_GROUND;

	data->ui = gfc_allocate_array(sizeof(PlayerUI), 1);
	data->ui->enabled = 0;
	data->ui->selectedCategory = 0; //0 for Heads by default
		
	data->leg = gfc_allocate_array(sizeof(Leg), 1); //player personal parts
	data->body = gfc_allocate_array(sizeof(Body), 1);
	data->arm = gfc_allocate_array(sizeof(Arm), 1);
	data->head = gfc_allocate_array(sizeof(Head), 1);
	data->gun = gfc_allocate_array(sizeof(Weapon), 1);
	data->shoulder = gfc_allocate_array(sizeof(Weapon), 1);

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
	for (int i = 0; i < 3; i++) { //3 is the size of the defs
		part = sj_array_get_nth(defArray, i);
		player_add_head(data, part);
	}
	data->head = gfc_list_get_nth(data->headInventory, 0);

	defArray = sj_object_get_value(data->arms, "arms");
	for (int i = 0; i < 3; i++) {
		part = sj_array_get_nth(defArray, i);
		player_add_arm(data, part);
	}
	data->arm = gfc_list_get_nth(data->armInventory, 0);

	defArray = sj_object_get_value(data->bodies, "bodies");
	for (int i = 0; i < 3; i++) {
		part = sj_array_get_nth(defArray, i);
		player_add_body(data, part);
	}
	data->body = gfc_list_get_nth(data->bodyInventory, 0);

	defArray = sj_object_get_value(data->legs, "legs");
	for (int i = 0; i < 3; i++) {
		part = sj_array_get_nth(defArray, i);
		player_add_leg(data, part);
	}
	data->leg = gfc_list_get_nth(data->legInventory, 0);

	defArray = sj_object_get_value(data->guns, "guns");
	for (int i = 0; i < 3; i++) {
		part = sj_array_get_nth(defArray, i);
		player_add_gun(data, part);
	}
	data->gun = gfc_list_get_nth(data->gunInventory, 0);

	defArray = sj_object_get_value(data->shoulders, "shoulders");
	for (int i = 0; i < 3; i++) {
		part = sj_array_get_nth(defArray, i);
		player_add_shoulder(data, part);
	}
	data->shoulder = gfc_list_get_nth(data->shoulderInventory, 0);
	player_ui_update(data);
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

void player_ui_update(PlayerData* data) {
	char* numBuffer[10];
	if (!data) return;
	switch (data->ui->selectedCategory) {
		case 0:
			strcpy(data->ui->partDescription1, "Heads");
			strcpy(data->ui->partDescription2, "Part: ");
			strcat(data->ui->partDescription2, data->head->name);
			strcpy(data->ui->partDescription3, "AP: ");
			snprintf(numBuffer, sizeof(numBuffer), "%d", data->head->health);
			strcat(data->ui->partDescription3, numBuffer);
			strcpy(data->ui->partDescription4, "");
			break;
		case 1:
			break;
		case 2:
			break;
		case 3:
			break;
		case 4:
			break;
		case 5:
			break;
	}
}

void player_ui_draw() { //use static player instance to be easily accesible in game.c
	PlayerData* data;
	if (!thePlayer) return;
	data = thePlayer->data;
	if (!data) return;
	
	gf2d_font_draw_line_tag(data->ui->partDescription1, FT_H4, GFC_COLOR_WHITE, gfc_vector2d(10, 500));
	gf2d_font_draw_line_tag(data->ui->partDescription2, FT_H4, GFC_COLOR_WHITE, gfc_vector2d(10, 530));
	gf2d_font_draw_line_tag(data->ui->partDescription3, FT_H4, GFC_COLOR_WHITE, gfc_vector2d(10, 560));
	gf2d_font_draw_line_tag(data->ui->partDescription4, FT_H4, GFC_COLOR_WHITE, gfc_vector2d(10, 590));
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

void player_next_head(Entity* self) {
	PlayerData* data;
	if (!self)return;
	data = self->data;
	if (!data)return;

	data->headIndex += 1;
	if (data->headIndex >= data->headIndexMax) {
		data->headIndex = 0;
	}
	data->head = gfc_list_get_nth(data->headInventory, data->headIndex);
}

void player_next_arm(Entity* self) {
	PlayerData* data;
	if (!self)return;
	data = self->data;
	if (!data)return;

	data->armIndex += 1;
	if (data->armIndex >= data->armIndexMax) {
		data->armIndex = 0;
	}
	data->arm = gfc_list_get_nth(data->armInventory, data->armIndex);
}

void player_next_body(Entity* self) {
	PlayerData* data;
	if (!self)return;
	data = self->data;
	if (!data)return;

	data->bodyIndex += 1;
	if (data->bodyIndex >= data->bodyIndexMax) {
		data->bodyIndex = 0;
	}
	data->body = gfc_list_get_nth(data->bodyInventory, data->bodyIndex);
}

void player_next_leg(Entity* self) {
	PlayerData* data;
	if (!self)return;
	data = self->data;
	if (!data)return;

	data->legIndex += 1;
	if (data->legIndex >= data->legIndexMax) {
		data->legIndex = 0;
	}
	data->leg = gfc_list_get_nth(data->legInventory, data->legIndex);
}

void player_next_gun(Entity* self) {
	PlayerData* data;
	if (!self)return;
	data = self->data;
	if (!data)return;

	data->gunIndex += 1;
	if (data->gunIndex >= data->gunIndexMax) {
		data->gunIndex = 0;
	}
	data->gun = gfc_list_get_nth(data->gunInventory, data->gunIndex);
}

void player_next_shoulder(Entity* self) {
	PlayerData* data;
	if (!self)return;
	data = self->data;
	if (!data)return;

	data->shoulderIndex += 1;
	if (data->shoulderIndex >= data->shoulderIndexMax) {
		data->shoulderIndex = 0;
	}
	data->shoulder = gfc_list_get_nth(data->shoulderInventory, data->shoulderIndex);
}

void player_do_max_health(Entity* self) {
	PlayerData* data;
	int countHealth = 0;
	if (!self)return;
	data = self->data;
	if (!data)return;
	countHealth += data->head->health;
	countHealth += data->arm->health;
	countHealth += data->body->health;
	countHealth += data->leg->health;
	data->maxHealth = countHealth;
	if (data->currentHealth > data->maxHealth) {
		data->currentHealth = data->maxHealth;
	}
	slog("New Player Max Health: %i", data->maxHealth);
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

void player_add_arm(PlayerData* pData, SJson* armToAdd) {
	const char* meshPath;
	const char* texturePath;
	Arm* arm;
	if (!pData || !armToAdd) return NULL;
	arm = gfc_allocate_array(sizeof(Arm), 1);

	strcpy(arm->name, sj_object_get_value_as_string(armToAdd, "name"));
	sj_object_get_value_as_int(armToAdd, "health", &arm->health);
	meshPath = sj_object_get_value_as_string(armToAdd, "mesh");
	texturePath = sj_object_get_value_as_string(armToAdd, "texture");
	arm->armMesh = gf3d_mesh_load(meshPath);
	arm->armTexture = gf3d_texture_load(texturePath);
	gfc_list_append(pData->armInventory, arm);
	pData->armIndexMax = (Uint8)gfc_list_count(pData->armInventory);
}

void player_add_body(PlayerData* pData, SJson* bodyToAdd) {
	const char* meshPath;
	const char* texturePath;
	Body* body;
	if (!pData || !bodyToAdd) return NULL;
	body = gfc_allocate_array(sizeof(Body), 1);

	strcpy(body->name, sj_object_get_value_as_string(bodyToAdd, "name"));
	sj_object_get_value_as_int(bodyToAdd, "health", &body->health);
	meshPath = sj_object_get_value_as_string(bodyToAdd, "mesh");
	texturePath = sj_object_get_value_as_string(bodyToAdd, "texture");
	body->bodyMesh = gf3d_mesh_load(meshPath);
	body->bodyTexture = gf3d_texture_load(texturePath);
	gfc_list_append(pData->bodyInventory, body);
	pData->bodyIndexMax = (Uint8)gfc_list_count(pData->bodyInventory);
}

void player_add_leg(PlayerData* pData, SJson* legToAdd) {
	const char* meshPath;
	const char* texturePath;
	Leg* leg;
	if (!pData || !legToAdd) return NULL;
	leg = gfc_allocate_array(sizeof(Leg), 1);

	strcpy(leg->name, sj_object_get_value_as_string(legToAdd, "name"));
	sj_object_get_value_as_int(legToAdd, "health", &leg->health);
	meshPath = sj_object_get_value_as_string(legToAdd, "mesh");
	texturePath = sj_object_get_value_as_string(legToAdd, "texture");
	leg->legMesh = gf3d_mesh_load(meshPath);
	leg->legTexture = gf3d_texture_load(texturePath);
	gfc_list_append(pData->legInventory, leg);
	pData->legIndexMax = (Uint8)gfc_list_count(pData->legInventory);
}

void player_add_gun(PlayerData* pData, SJson* weaponToAdd) {
	const char* meshPath;
	const char* texturePath;
	Weapon* weapon;
	if (!pData || !weaponToAdd) return NULL;
	weapon = gfc_allocate_array(sizeof(Weapon), 1);

	strcpy(weapon->name, sj_object_get_value_as_string(weaponToAdd, "name"));
	sj_object_get_value_as_int(weaponToAdd, "damage", &weapon->damage);
	sj_object_get_value_as_int(weaponToAdd, "cooldown", &weapon->cooldown);
	meshPath = sj_object_get_value_as_string(weaponToAdd, "mesh");
	texturePath = sj_object_get_value_as_string(weaponToAdd, "texture");
	weapon->weaponMesh = gf3d_mesh_load(meshPath);
	weapon->weaponTexture = gf3d_texture_load(texturePath);
	gfc_list_append(pData->gunInventory, weapon);
	pData->gunIndexMax = (Uint8)gfc_list_count(pData->gunInventory);
}

void player_add_shoulder(PlayerData* pData, SJson* weaponToAdd) {
	const char* meshPath;
	const char* texturePath;
	Weapon* weapon;
	if (!pData || !weaponToAdd) return NULL;
	weapon = gfc_allocate_array(sizeof(Weapon), 1);

	strcpy(weapon->name, sj_object_get_value_as_string(weaponToAdd, "name"));
	sj_object_get_value_as_int(weaponToAdd, "damage", &weapon->damage);
	sj_object_get_value_as_int(weaponToAdd, "cooldown", &weapon->cooldown);
	meshPath = sj_object_get_value_as_string(weaponToAdd, "mesh");
	texturePath = sj_object_get_value_as_string(weaponToAdd, "texture");
	weapon->weaponMesh = gf3d_mesh_load(meshPath);
	weapon->weaponTexture = gf3d_texture_load(texturePath);
	gfc_list_append(pData->shoulderInventory, weapon);
	pData->shoulderIndexMax = (Uint8)gfc_list_count(pData->shoulderInventory);
}