#include "simple_logger.h"

#include "enemy.h"
#include "player.h"

static SJson* enemyDefFile = NULL;
static SJson* enemyDefs = NULL;

Entity* enemy_spawn1(GFC_Vector3D position, GFC_Color color) {
	Entity* self;
	self = entity_new();
	if (!self)return;

	enemy_config(self, 1);
	self->type = ET_Enemy;
	self->color = color;
	self->position = position;
	self->bounds = gfc_box(position.x - 1.5, position.y - 1.5, position.z - 1.5, 3, 3, 3);
	self->rotation = gfc_vector3d(0, 0, 0);
	//void (*draw)(Entity_S);
	self->think = enemy_think1;
	self->update = enemy_update1;

	return self;
}

void enemy_thinkg(Entity* self, EnemyData* eData, Entity* player, GFC_Vector3D playerVector) {
	if (!self) return;
	if (!eData) return;

	if (eData->fireCooldown > 0) {
		eData->fireCooldown -= 1;
	}
	if (gfc_vector3d_distance_between_less_than(self->position, player->position, eData->seeingRange)) {
			self->rotation.z = gfc_vector2d_angle(gfc_vector3dxy(playerVector));
	}
}

void enemy_think1(Entity* self) {
	Entity* player;
	EnemyData *eData;
	GFC_Vector3D playerVector;

	player = get_the_player();
	if (!player) return;
	eData = self->data;
	if (!self || !eData) return;

	playerVector = gfc_vector3d(self->position.x - player->position.x, self->position.y - player->position.y, self->position.z - player->position.z);
	gfc_vector3d_normalize(&playerVector);
	
	enemy_thinkg(self, eData, player, playerVector);
}

Entity* enemy_spawn2(GFC_Vector3D position, GFC_Color color) {
	Entity* self;
	self = entity_new();
	if (!self)return;

	enemy_config(self, 2);
	self->color = color;
	self->position = position;
	self->bounds = gfc_box(position.x - 1.5, position.y - 1.5, position.z, 3, 3, 2);
	self->rotation = gfc_vector3d(0, 0, 0);
	//void (*draw)(Entity_S);
	self->think = enemy_think2;
	self->update = enemy_update2;

	return self;
}
void enemy_think2(Entity* self) {

}

Entity* enemy_spawn3(GFC_Vector3D position, GFC_Color color) {
	Entity* self;
	self = entity_new();
	if (!self)return;

	enemy_config(self, 3);
	self->color = color;
	self->position = position;
	self->bounds = gfc_box(position.x - 1, position.y - 1, position.z - 1, 2, 2, 2);
	self->rotation = gfc_vector3d(0, 0, 0);
	//void (*draw)(Entity_S);
	self->think = enemy_think3;
	self->update = enemy_update3;

	return self;
}
void enemy_think3(Entity* self) {

}

Entity* enemy_spawn4(GFC_Vector3D position, GFC_Color color) {
	Entity* self;
	self = entity_new();
	if (!self)return;

	enemy_config(self, 4);
	self->color = color;
	self->position = position;
	self->bounds = gfc_box(position.x - 2, position.y - 2, position.z, 4, 4, 8.5);
	self->rotation = gfc_vector3d(0, 0, 0);
	//void (*draw)(Entity_S);
	self->think = enemy_think4;
	self->update = enemy_update4;

	return self;
}
void enemy_think4(Entity* self) {

}

Entity* enemy_spawn5(GFC_Vector3D position, GFC_Color color) {
	Entity* self;
	self = entity_new();
	if (!self)return;

	enemy_config(self, 5);
	self->color = color;
	self->position = position;
	self->bounds = gfc_box(position.x - 1, position.y - 1, position.z, 2, 2, 1.5);
	self->rotation = gfc_vector3d(0, 0, 0);
	//void (*draw)(Entity_S);
	self->think = enemy_think5;
	self->update = enemy_update5;

	return self;
}
void enemy_think5(Entity* self) {

}

void enemy_config(Entity* self, int enemyIndex) {
	SJson* enemyDef;
	EnemyData* eData;

	if (enemyIndex == 0) {
		slog("INVALID ENEMY TYPE 0, THATS THE FILLER ENTRY");
	}
	if (!self) {
		return;
	}
	if (!enemyDefFile) {
		enemyDefFile = sj_load("defs/enemies.def");
		if (!enemyDefs) {
			enemyDefs = sj_object_get_value(enemyDefFile, "enemies");
		}
	}
	eData = gfc_allocate_array(sizeof(EnemyData), 1);

	enemyDef = sj_array_get_nth(enemyDefs, enemyIndex);
	//continue implementing, adjust spawn functions
	strcpy(self->name, sj_object_get_value_as_string(enemyDef, "name"));
	self->mesh = gf3d_mesh_load(sj_object_get_value_as_string(enemyDef, "mesh"));
	self->texture = gf3d_texture_load(sj_object_get_value_as_string(enemyDef, "texture"));
	sj_object_get_value_as_int(enemyDef, "health", &self->health);
	if (!self->mesh || !self->texture) {
		slog("Failed to load %s enemy mesh or texture", self->name);
	}
	self->type = ET_Enemy; //all enemies need, not unique like other traits
	sj_object_get_value_as_int(enemyDef, "fireCooldown", &eData->fireCooldownSet);
	sj_object_get_value_as_int(enemyDef, "seeingRange", &eData->seeingRange);
	sj_object_get_value_as_int(enemyDef, "projectileIndex", &eData->projectileIndex);
	self->data = eData;
}