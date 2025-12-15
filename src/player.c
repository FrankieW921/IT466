#include "simple_logger.h"
#include "gfc_input.h"

#include "gf2d_font.h"

#include "player.h"
#include "world.h"
#include "camera_entity.h"
#include "projectile.h"
#include "game.h"

static Entity* thePlayer;

static Uint8 partSwapCooldown = 0;
static Uint8 fuelRecharge = 0;
static Uint8 boostCooldown = 0;
static Uint8 selectedWorld = 0;

static Uint8 lockedOn = 0;
static Entity* targetedEntity = NULL;

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
	self->bounds = gfc_box(position.x-2, position.y-2, position.z, 4, 4, 11.5);
	//entity defaults to scale of 1, 1, 1
	self->think = player_think;
	self->update = player_update;
	self->draw = player_draw;
	self->type = ET_Player;
	self->free = player_free;

	player_data_new(data);
	self->data = data;
	player_do_max_health(self);
	data->currentHealth = data->maxHealth;

	thePlayer = self; //assign static variable
	return self;
}

Entity* editor_spawn(GFC_Vector3D position, GFC_Color color) {
	Entity* self; 

	self = entity_new();
	if (!self) return;

	gfc_line_cpy(self->name, "Editor");
	self->color = color;
	self->position = position;
	self->rotation = gfc_vector3d(0, 0, 0);
	self->bounds = gfc_box(position.x - 2.5, position.y - 2.5, position.z, 5, 5, 5);
	self->mesh = gf3d_mesh_load("models/player/editor.obj");
	self->texture = gf3d_texture_load("models/player/editor.png");
	self->think = editor_think;
	self->update = editor_update;

	self->type = ET_Player;

	thePlayer = self;
	selectedWorld = 0;
	return self;
}

void player_think(Entity* self) {
	Uint32 mouseState;
	PlayerData* data;

	int mx, my;
	GFC_Vector2D direction2d;
	Uint8 partChanged = 0;
	float move = 0; 
	float moveStep = 0;
	GFC_Vector3D shootPosition, shootVelocity;

	if (!self) return;
	data = self->data;
	if (!data) return;

	moveStep = data->leg->speed;

	if (partSwapCooldown > 0) {
		partSwapCooldown -= 1;
	}

	if (boostCooldown > 0) {
		boostCooldown -= 1;
		if (boostCooldown > 40) {
			moveStep *= 3; //maintain boost for 20 frames
		}
	}

	if (data->fireCooldoown > 0) {
		data->fireCooldoown -= 1;
	}

	//rotate player
	if (gfc_input_command_down("panleft")) {
		self->rotation.z += (.25 * moveStep);
	}
	if (gfc_input_command_down("panright")) {
		self->rotation.z -= (.25 * moveStep);
	}

	//check if player quickboosted to increase moveStep
	if (gfc_input_command_down("boost") && boostCooldown == 0 && data->currentFuel >= 70) {
		moveStep *= 3;
		boostCooldown = 60;
		data->currentFuel -= 70;
		fuelRecharge = 75;
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
		if (data->currentFuel > 0) {
			data->movementState = MS_FLYING;
			data->currentFuel -= 5;
			if (data->currentFuel < 0) data->currentFuel = 0;
		}
		fuelRecharge = 75; //player MUST let go of space
	}

	//change z velocity based on MS, fuel recharge decreases on ground and falling
	switch (data->movementState) {
		case MS_ON_GROUND:
			self->velocity.z = 0;
			if (fuelRecharge > 0) fuelRecharge-=1;
			break;
		case MS_FALLING:
			self->velocity.z -= .05;
			if (self->velocity.z < -3) self->velocity.z = -3;
			if (fuelRecharge > 0) fuelRecharge-=1;
			break;
		case MS_FLYING:
			self->velocity.z += .03;
			if (self->velocity.z > 2) self->velocity.z = 2;
			break;
	}

	if (fuelRecharge == 0) {
		data->currentFuel += 10;
		if (data->currentFuel > data->maxFuel) {
			data->currentFuel = data->maxFuel;
		}
	}

	if (gfc_input_command_down("nextHead") && partSwapCooldown == 0 && data->ui->enabled == 1) {
		if (data->ui->selectedCategory == 0) {
			player_next_head(self);
		}
		else {
			data->ui->selectedCategory = 0;
		}
		player_ui_update(data);
		partSwapCooldown = 60;
		partChanged = 1;
	}
	if (gfc_input_command_down("nextArm") && partSwapCooldown == 0 && data->ui->enabled == 1) {
		if (data->ui->selectedCategory == 1) {
			player_next_arm(self);
		}
		else {
			data->ui->selectedCategory = 1;
		}
		player_ui_update(data);
		partSwapCooldown = 60;
		partChanged = 1;
	}
	if (gfc_input_command_down("nextBody") && partSwapCooldown == 0 && data->ui->enabled == 1) {
		if (data->ui->selectedCategory == 2) {
			player_next_body(self);
		}
		else {
			data->ui->selectedCategory = 2;
		}
		player_ui_update(data);
		partSwapCooldown = 60;
		partChanged = 1;
	}
	if (gfc_input_command_down("nextLeg") && partSwapCooldown == 0 && data->ui->enabled == 1) {
		if (data->ui->selectedCategory == 3) {
			player_next_leg(self);
		}
		else {
			data->ui->selectedCategory = 3;
		}
		player_ui_update(data);
		partSwapCooldown = 60;
		partChanged = 1;
	}
	if (gfc_input_command_down("nextGun") && partSwapCooldown == 0 && data->ui->enabled == 1) {
		if (data->ui->selectedCategory == 4) {
			player_next_gun(self);
		}
		else {
			data->ui->selectedCategory = 4;
		}
		player_ui_update(data);
		partSwapCooldown = 60;
		partChanged = 1;
	}
	if (gfc_input_command_down("nextShoulder") && partSwapCooldown == 0 && data->ui->enabled == 1) {
		if (data->ui->selectedCategory == 5) {
			player_next_shoulder(self);
		}
		else {
			data->ui->selectedCategory = 5;
		}
		player_ui_update(data);
		partSwapCooldown = 60;
		partChanged = 1;
	}
	if (gfc_input_command_down("partUIToggle") && partSwapCooldown == 0) {
		if (data->ui->enabled == 1) {
			data->ui->enabled = 0;
		}
		else if (data->ui->enabled == 0) {
			data->ui->enabled = 1;
		}
		partSwapCooldown = 60;
		partChanged = 1;
	}
	if (partChanged == 1) {
		player_do_max_health(self);
	}

	if (gfc_input_command_down("devHeal")) {
		data->currentHealth = data->maxHealth;
	}
	
	targetedEntity = camera_target_lock();
	if (targetedEntity == NULL) {
		lockedOn = 0;
	}
	else {
		lockedOn = 1;
	}

	if (gfc_input_command_down("shoot") && data->fireCooldoown == 0) {
		gfc_vector3d_copy(shootPosition,self->position);
		shootPosition.z += 6;

		if (lockedOn) { //reticle has caught an enemy in this think iteration, maffs
			if (strcmp(targetedEntity->name, "Muscle Tracer") == 0) {
				shootVelocity = gfc_vector3d(targetedEntity->position.x - shootPosition.x, targetedEntity->position.y - shootPosition.y, targetedEntity->position.z - shootPosition.z + 6);
			}
			else if (strcmp(targetedEntity->name, "Tank") == 0 || strcmp(targetedEntity->name, "Turret") == 0) {
				shootVelocity = gfc_vector3d(targetedEntity->position.x - shootPosition.x, targetedEntity->position.y - shootPosition.y, targetedEntity->position.z - shootPosition.z + 2);
			}
			else {
				shootVelocity = gfc_vector3d(targetedEntity->position.x - shootPosition.x, targetedEntity->position.y - shootPosition.y, targetedEntity->position.z - shootPosition.z);
			}
			
			gfc_vector3d_normalize(&shootVelocity);
			projectile_spawn(data->gunIndex, shootPosition, shootVelocity);
		}
		else { //freefire, only viable implementation with my camera since the target cam position x and y are always the same as the player's
			gfc_vector3d_negate(shootVelocity, get_view_vector());
			projectile_spawn(data->gunIndex, shootPosition, shootVelocity);
		}
		data->fireCooldoown = data->gun->cooldown;
	}
	
	mouseState = SDL_GetMouseState(&mx, &my);

	self->collideEntities = entity_collide_all(self);
}

void player_update(Entity* self) {
	PlayerData* data;
	Entity* ent;
	ProjectileData* projData;

	if (!self) return;
	data = self->data;
	if (!data) return;

	player_move(self); 
	self->bounds.x = self->position.x - 2.25;
	self->bounds.y = self->position.y - 1.75;
	self->bounds.z = self->position.z;

	if (self->collideEntities) {
		for (int i = 0; i < gfc_list_get_count(self->collideEntities); i++) {
			ent = gfc_list_get_nth(self->collideEntities, i);
			if (ent->type == ET_Enemy_Projectile) {
				slog("COLLIDING WITH ENEMY PROJECTILE");
				projData = ent->data;
				data->currentHealth -= projData->damage;
				projectile_free(ent);
			}
		}
	}
	gfc_list_clear(self->collideEntities);
	if (data->currentHealth <= 0) main_menu();
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
				self->position.z = positionPre.z;
				self->position.x = positionPost.x;
				self->position.y = positionPost.y;
			}
			else { //not touching wall
				gfc_vector3d_copy(self->position, positionPost);
			}
			break;
		case MS_FALLING:
			if (world_edge_test(get_the_world(), positionPre, positionPost, &contact)) {
				if (contact.z == entity_floor_check(self)) { //you touched the ground
					data->movementState = MS_ON_GROUND;
					self->position.z = entity_floor_check(self) + .01;
				}
				else if (contact.z == entity_roof_check(self)) { //let go of fly and hit the cieling
					self->position.z = entity_roof_check(self) - .1;
					self->velocity.z = 0;
				}
				else { //you touched a wall while falling
					self->position.z += self->velocity.z;
				}
			}
			else if (entity_floor_check(self) == -99999) { //no floor below
				//gfc_vector3d_copy(self->position, positionPost);
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
	gfc_vector2d_scale(self->velocity, self->velocity, .90);
	if (self->velocity.x < .05 && self->velocity.x > -.05)self->velocity.x = 0;
	if (self->velocity.y < .05 && self->velocity.y > -.05)self->velocity.y = 0;

	gfc_box_cpy(bounds, self->bounds); //start of collision checking
	gfc_vector3d_add(bounds, bounds, self->velocity);
}

void editor_think(Entity* self) {
	Uint32 mouseState;

	int mx, my;
	GFC_Vector2D direction2d;
	Uint8 partChanged = 0;
	float move = 0;
	float moveStep = 0;

	if (!self) return;

	moveStep = .25;
	if (partSwapCooldown > 0) { //we're going to use this as our enemy placement/mission change cooldown
		partSwapCooldown -= 1;
	}
	self->velocity = gfc_vector3d(0, 0, 0);

	//rotate
	if (gfc_input_command_down("panleft")) {
		self->rotation.z += (.25 * moveStep);
	}
	if (gfc_input_command_down("panright")) {
		self->rotation.z -= (.25 * moveStep);
	}
	//movement
	direction2d = gfc_vector2d_from_angle(self->rotation.z);
	gfc_vector2d_normalize(&direction2d);
	if (gfc_input_command_down("moveforward")) {
		move += moveStep * 4;
	}
	if (gfc_input_command_down("moveback")) {
		move -= moveStep * 4;
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
		move -= moveStep * 4;
	}
	if (gfc_input_command_down("moveleft")) {
		move += moveStep * 4;
	}
	if (move) {
		gfc_vector2d_scale(direction2d, direction2d, move);
		gfc_vector2d_add(self->velocity, self->velocity, direction2d);
	}
	if (gfc_input_command_down("jump")) {
		self->velocity.z += moveStep * 4;
	}
	if (gfc_input_command_down("crouch")) {
		self->velocity.z -= moveStep * 4;
	}

	if (gfc_input_command_down("nextHead") && partSwapCooldown == 0) {
		world_enemy_spawn(1, self->position, GFC_COLOR_WHITE);
		//player_ui_update(data);
		partSwapCooldown = 60;
		partChanged = 1;
	}
	if (gfc_input_command_down("nextArm") && partSwapCooldown == 0) {
		world_enemy_spawn(2, self->position, GFC_COLOR_WHITE);
		//player_ui_update(data);
		partSwapCooldown = 60;
		partChanged = 1;
	}
	if (gfc_input_command_down("nextBody") && partSwapCooldown == 0) {
		world_enemy_spawn(3, self->position, GFC_COLOR_WHITE);
		//player_ui_update(data);
		partSwapCooldown = 60;
		partChanged = 1;
	}
	if (gfc_input_command_down("nextLeg") && partSwapCooldown == 0) {
		world_enemy_spawn(4, self->position, GFC_COLOR_WHITE);
		//player_ui_update(data);
		partSwapCooldown = 60;
		partChanged = 1;
	}
	if (gfc_input_command_down("nextGun") && partSwapCooldown == 0) {
		world_enemy_spawn(5, self->position, GFC_COLOR_WHITE);
		//player_ui_update(data);
		partSwapCooldown = 60;
		partChanged = 1;
	}
	if (gfc_input_command_down("nextShoulder") && partSwapCooldown == 0) {
		selectedWorld += 1;
		if (selectedWorld > 2) selectedWorld = 0;
		if (selectedWorld == 0) {
			world_set_model("models/terrain/terrain1.obj", "models/terrain/terrain1.png");
			get_the_world()->lightPosition.y = 0;
		}
		else if (selectedWorld == 1) {
			world_set_model("models/terrain/terrain2.obj", "models/terrain/terrain2.png");
			get_the_world()->lightPosition.y = 0;
		}
		else if (selectedWorld == 2) {
			world_set_model("models/terrain/terrain3.obj", "models/terrain/terrain3.png");
			get_the_world()->lightPosition.y = 700;
		}
		//player_ui_update(data);
		partSwapCooldown = 60;
		partChanged = 1;
	}
	if (gfc_input_command_down("save1") && partSwapCooldown == 0) {
		/* NOT YET, NO WANT BREAKY
		if (selectedWorld == 0) {
			world_save(0, "models/terrain/terrain1.obj", "models/terrain/terrain1.png", 1);
		}
		else if (selectedWorld == 1) {
			world_save(0, "models/terrain/terrain2.obj", "models/terrain/terrain2.png", 2);
		}
		else if (selectedWorld == 2) {
			world_save(0, "models/terrain/terrain3.obj", "models/terrain/terrain3.png", 3);
		}
		*/
		partSwapCooldown = 60;
		partChanged = 1;
	}
	if (gfc_input_command_down("save2") && partSwapCooldown == 0) {
		if (selectedWorld == 0) {
			world_save(1, "models/terrain/terrain1.obj", "models/terrain/terrain1.png", 1);
		}
		else if (selectedWorld == 1) {
			world_save(1, "models/terrain/terrain2.obj", "models/terrain/terrain2.png", 2);
		}
		else if (selectedWorld == 2) {
			world_save(1, "models/terrain/terrain3.obj", "models/terrain/terrain3.png", 3);
		}
		partSwapCooldown = 60;
		partChanged = 1;
	}
	if (gfc_input_command_down("save3") && partSwapCooldown == 0) {
		if (selectedWorld == 0) {
			world_save(2, "models/terrain/terrain1.obj", "models/terrain/terrain1.png", 1);
		}
		else if (selectedWorld == 1) {
			world_save(2, "models/terrain/terrain2.obj", "models/terrain/terrain2.png", 2);
		}
		else if (selectedWorld == 2) {
			world_save(2, "models/terrain/terrain3.obj", "models/terrain/terrain3.png", 3);
		}
		partSwapCooldown = 60;
		partChanged = 1;
	}
}

void editor_update(Entity* self) {
	GFC_Box bounds;
	GFC_Vector3D positionPre, positionPost, contact;
	GFC_Vector2D direction2d;
	if (!self) return;
	entity_move(self);
}

void player_data_new(PlayerData* data) { //hardcode the stuff for now
	Head* headG; //generics for setting
	Arm* armG;
	Body* bodyG;
	Leg* legG;
	SJson* defArray, *part;

	data->movementState = MS_ON_GROUND;

	data->ui = gfc_allocate_array(sizeof(PlayerUI), 1);
	data->ui->enabled = 1;
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
	data->maxFuel = data->body->fuel; //first time

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

	data->reticle = gf2d_sprite_load_image("images/reticle.png");
	data->reticleLocked = gf2d_sprite_load_image("images/reticleLocked.png");

	player_ui_update(data);
}

void editor_data_new(PlayerData* data) {

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
	if (!data || !data->ui) return;
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
			strcpy(data->ui->partDescription1, "Arms");
			strcpy(data->ui->partDescription2, "Part: ");
			strcat(data->ui->partDescription2, data->arm->name);
			strcpy(data->ui->partDescription3, "AP: ");
			snprintf(numBuffer, sizeof(numBuffer), "%d", data->arm->health);
			strcat(data->ui->partDescription3, numBuffer);
			strcpy(data->ui->partDescription4, "");
			break;
		case 2:
			strcpy(data->ui->partDescription1, "Bodies");
			strcpy(data->ui->partDescription2, "Part: ");
			strcat(data->ui->partDescription2, data->body->name);
			strcpy(data->ui->partDescription3, "AP: ");
			snprintf(numBuffer, sizeof(numBuffer), "%d", data->body->health);
			strcat(data->ui->partDescription3, numBuffer);
			strcpy(data->ui->partDescription4, "Fuel: ");
			snprintf(numBuffer, sizeof(numBuffer), "%d", data->body->fuel);
			strcat(data->ui->partDescription4, numBuffer);
			break;
		case 3:
			strcpy(data->ui->partDescription1, "Leg");
			strcpy(data->ui->partDescription2, "Part: ");
			strcat(data->ui->partDescription2, data->leg->name);
			strcpy(data->ui->partDescription3, "AP: ");
			snprintf(numBuffer, sizeof(numBuffer), "%d", data->leg->health);
			strcat(data->ui->partDescription3, numBuffer);
			strcpy(data->ui->partDescription4, "Speed: ");
			snprintf(numBuffer, sizeof(numBuffer), "%f", data->leg->speed);
			strcat(data->ui->partDescription4, numBuffer);
			break;
		case 4:
			strcpy(data->ui->partDescription1, "Guns");
			strcpy(data->ui->partDescription2, "Gun: ");
			strcat(data->ui->partDescription2, data->gun->name);
			strcpy(data->ui->partDescription3, "Damage: ");
			snprintf(numBuffer, sizeof(numBuffer), "%d", data->gun->damage);
			strcat(data->ui->partDescription3, numBuffer);
			strcpy(data->ui->partDescription4, "Cooldown: ");
			snprintf(numBuffer, sizeof(numBuffer), "%d", data->gun->cooldown);
			strcat(data->ui->partDescription4, numBuffer);
			break;
		case 5:
			strcpy(data->ui->partDescription1, "Shoulders");
			strcpy(data->ui->partDescription2, "Shoulder: ");
			strcat(data->ui->partDescription2, data->shoulder->name);
			strcpy(data->ui->partDescription3, "Damage: ");
			snprintf(numBuffer, sizeof(numBuffer), "%d", data->shoulder->damage);
			strcat(data->ui->partDescription3, numBuffer);
			strcpy(data->ui->partDescription4, "Cooldown: ");
			snprintf(numBuffer, sizeof(numBuffer), "%d", data->shoulder->cooldown);
			strcat(data->ui->partDescription4, numBuffer);
			break;
	}
}

void player_ui_draw() { //use static player instance to be easily accesible in game.c
	PlayerData* data;
	char* buffer1[128];
	char* buffer2[128];
	char* buffer3[128];
	if (!thePlayer) return;

	if (strcmp(thePlayer->name, "Editor") == 0) {
		strcpy(buffer1, "1-5: Spawn Enemies");
		return;
	}

	data = thePlayer->data;
	if (!data || !data->ui) return;
	//draw health
	strcpy(buffer1, "AP: ");
	snprintf(buffer2, sizeof(buffer2), "%d", data->currentHealth);
	strcat(buffer1, buffer2);
	gf2d_font_draw_line_tag(buffer1, FT_H4, GFC_COLOR_LIGHTRED, gfc_vector2d(10,200));
	//draw fuel
	strcpy(buffer1, "Fuel: ");
	snprintf(buffer2, sizeof(buffer2), "%d", data->currentFuel);
	strcat(buffer1, buffer2);
	gf2d_font_draw_line_tag(buffer1, FT_H4, GFC_COLOR_LIGHTRED, gfc_vector2d(10, 230));
	//draw quickboost indicator
	if (boostCooldown > 0) {
		strcpy(buffer1, "Boost: Cooldown...");
		gf2d_font_draw_line_tag(buffer1, FT_H4, GFC_COLOR_LIGHTRED, gfc_vector2d(10, 260));
	}
	else {
		strcpy(buffer1, "Boost: READY");
		gf2d_font_draw_line_tag(buffer1, FT_H4, GFC_COLOR_DARKGREEN, gfc_vector2d(10, 260));
	}
	//draw lockon indicator
	if (lockedOn == 1) {
		strcpy(buffer1, "TARGET LOCK: ");
		if (targetedEntity != NULL) {
			snprintf(buffer2, sizeof(buffer2), "%s", targetedEntity->name);
			strcat(buffer1, buffer2);
		}
		gf2d_font_draw_line_tag(buffer1, FT_H4, GFC_COLOR_LIGHTRED, gfc_vector2d(10, 290));
		gf2d_sprite_draw_image(data->reticleLocked, gfc_vector2d(667, 368));
	}
	else {
		strcpy(buffer1, "Scanning...");
		gf2d_font_draw_line_tag(buffer1, FT_H4, GFC_COLOR_DARKGREEN, gfc_vector2d(10, 290));
		gf2d_sprite_draw_image(data->reticle, gfc_vector2d(667, 368));
	}

	if (data->ui->enabled) { //this is currently just for the part selection UI
		gf2d_font_draw_line_tag(data->ui->partDescription1, FT_H4, GFC_COLOR_LIGHTRED, gfc_vector2d(10, 500));
		gf2d_font_draw_line_tag(data->ui->partDescription2, FT_H4, GFC_COLOR_LIGHTRED, gfc_vector2d(10, 530));
		gf2d_font_draw_line_tag(data->ui->partDescription3, FT_H4, GFC_COLOR_LIGHTRED, gfc_vector2d(10, 560));
		gf2d_font_draw_line_tag(data->ui->partDescription4, FT_H4, GFC_COLOR_LIGHTRED, gfc_vector2d(10, 590));
	}
}

void player_free() {
	PlayerData* pData;

	if (!thePlayer) return;
	pData = thePlayer->data;
	if (!pData) return;
	slog("Freeing parts");
	player_free_heads(pData); //this will have also freed the player's currently equipped parts
	player_free_arms(pData);
	player_free_bodies(pData);
	player_free_legs(pData);
	player_free_weapons(pData);
	slog("Freeing lists");
	gfc_list_clear(pData->headInventory);
	gfc_list_clear(pData->armInventory);
	gfc_list_clear(pData->bodyInventory);
	gfc_list_clear(pData->legInventory);
	gfc_list_clear(pData->gunInventory);
	gfc_list_clear(pData->shoulderInventory);
	slog("Freeing sjsons");
	sj_free(pData->heads);
	sj_free(pData->arms);
	sj_free(pData->bodies);
	sj_free(pData->legs);
	sj_free(pData->guns);
	sj_free(pData->shoulders);
	slog("Freeing reticle sprites");
	gf2d_sprite_free(pData->reticle);
	gf2d_sprite_free(pData->reticleLocked);
	slog("Freeing ui");
	pData->ui->partDescription1[0] = '\0';
	pData->ui->partDescription2[0] = '\0';
	pData->ui->partDescription3[0] = '\0';
	pData->ui->partDescription4[0] = '\0';
	memset(pData->ui, 0 , sizeof(PlayerUI));
	slog("Freeing pData");
	memset(pData, 0, sizeof(PlayerData));
	thePlayer = NULL;
	slog("Freeing done");
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
	sj_object_get_value_as_int(selectedBody, "fuel", &currentBody->fuel);
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
	sj_object_get_value_as_float(selectedLeg, "speed", &currentLeg->speed);
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
	data->maxFuel = data->body->fuel;
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
	sj_object_get_value_as_int(bodyToAdd, "fuel", &body->fuel);
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
	sj_object_get_value_as_float(legToAdd, "speed", &leg->speed);
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

void player_free_heads(PlayerData* pData) {
	Head* part;
	for (int i = 0; i < gfc_list_get_count(pData->headInventory); i++) {
		part = gfc_list_get_nth(pData->headInventory, i);
		part->name[0] = '\0';
		gf3d_mesh_free(part->headMesh);
		gf3d_texture_free(part->headTexture);
		memset(part, 0, sizeof(Head));
	}
}

void player_free_arms(PlayerData* pData) {
	Arm* part;
	for (int i = 0; i < gfc_list_get_count(pData->armInventory); i++) {
		part = gfc_list_get_nth(pData->armInventory, i);
		part->name[0] = '\0';
		gf3d_mesh_free(part->armMesh);
		gf3d_texture_free(part->armTexture);
		memset(part, 0, sizeof(Arm));
	}
}

void player_free_bodies(PlayerData* pData) {
	Body* part;
	for (int i = 0; i < gfc_list_get_count(pData->bodyInventory); i++) {
		part = gfc_list_get_nth(pData->bodyInventory, i);
		part->name[0] = '\0';
		gf3d_mesh_free(part->bodyMesh);
		gf3d_texture_free(part->bodyTexture);
		memset(part, 0, sizeof(Body));
	}
}

void player_free_legs(PlayerData* pData) {
	Leg* part;
	for (int i = 0; i < gfc_list_get_count(pData->legInventory); i++) {
		part = gfc_list_get_nth(pData->legInventory, i);
		part->name[0] = '\0';
		gf3d_mesh_free(part->legMesh);
		gf3d_texture_free(part->legTexture);
		memset(part, 0, sizeof(Leg));
	}
}

void player_free_weapons(PlayerData* pData) {
	Weapon* part;
	for (int i = 0; i < gfc_list_get_count(pData->gunInventory); i++) {
		part = gfc_list_get_nth(pData->gunInventory, i);
		part->name[0] = '\0';
		gf3d_mesh_free(part->weaponMesh);
		gf3d_texture_free(part->weaponTexture);
		memset(part, 0, sizeof(Weapon));
	}
	for (int i = 0; i < gfc_list_get_count(pData->shoulderInventory); i++) {
		part = gfc_list_get_nth(pData->shoulderInventory, i);
		part->name[0] = '\0';
		gf3d_mesh_free(part->weaponMesh);
		gf3d_texture_free(part->weaponTexture);
		memset(part, 0, sizeof(Weapon));
	}
}