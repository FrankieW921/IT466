#include "simple_logger.h"

#include "enemy.h"

static SJson* enemyDefFile = NULL;
static SJson* enemyDefs = NULL;

Entity* enemy_spawn1(GFC_Vector3D position, GFC_Color color) {
	Entity* self;
	self = entity_new();
	if (!self)return;

	gfc_line_cpy(self->name, "Heli");
	self->mesh = gf3d_mesh_load("models/enemy1/enemy1.obj");
	self->texture = gf3d_texture_load("models/enemy1/enemy1.png");
	self->color = color;
	self->position = position;
	self->bounds = gfc_box(position.x - 1.5, position.y - 1.5, position.z - 1.5, 3, 3, 3);
	self->rotation = gfc_vector3d(0, 0, 0);
	//void (*draw)(Entity_S);
	self->think = enemy_think1;
	//void (*update)(Entity_S);

	return self;
}
void enemy_think1(Entity* self) {

}

Entity* enemy_spawn2(GFC_Vector3D position, GFC_Color color) {
	Entity* self;
	self = entity_new();
	if (!self)return;

	gfc_line_cpy(self->name, "Tank");
	self->mesh = gf3d_mesh_load("models/enemy2/enemy2.obj");
	self->texture = gf3d_texture_load("models/enemy2/enemy2.png");
	self->color = color;
	self->position = position;
	self->bounds = gfc_box(position.x - 1.5, position.y - 1.5, position.z, 3, 3, 2);
	self->rotation = gfc_vector3d(0, 0, 0);
	//void (*draw)(Entity_S);
	self->think = enemy_think2;
	//void (*update)(Entity_S);

	return self;
}
void enemy_think2(Entity* self) {

}

Entity* enemy_spawn3(GFC_Vector3D position, GFC_Color color) {
	Entity* self;
	self = entity_new();
	if (!self)return;

	gfc_line_cpy(self->name, "Drone");
	self->mesh = gf3d_mesh_load("models/enemy3/enemy3.obj");
	self->texture = gf3d_texture_load("models/enemy3/enemy3.png");
	self->color = color;
	self->position = position;
	self->bounds = gfc_box(position.x - 1, position.y - 1, position.z - 1, 2, 2, 2);
	self->rotation = gfc_vector3d(0, 0, 0);
	//void (*draw)(Entity_S);
	self->think = enemy_think3;
	//void (*update)(Entity_S);

	return self;
}
void enemy_think3(Entity* self) {

}

Entity* enemy_spawn4(GFC_Vector3D position, GFC_Color color) {
	Entity* self;
	self = entity_new();
	if (!self)return;

	gfc_line_cpy(self->name, "Muscle Tracer");
	self->mesh = gf3d_mesh_load("models/enemy4/enemy4.obj");
	self->texture = gf3d_texture_load("models/enemy4/enemy4.png");
	self->color = color;
	self->position = position;
	self->bounds = gfc_box(position.x - 2, position.y - 2, position.z, 4, 4, 8.5);
	self->rotation = gfc_vector3d(0, 0, 0);
	//void (*draw)(Entity_S);
	self->think = enemy_think4;
	//void (*update)(Entity_S);

	return self;
}
void enemy_think4(Entity* self) {

}

Entity* enemy_spawn5(GFC_Vector3D position, GFC_Color color) {
	Entity* self;
	self = entity_new();
	if (!self)return;

	gfc_line_cpy(self->name, "Turret");
	self->mesh = gf3d_mesh_load("models/enemy5/enemy5.obj");
	self->texture = gf3d_texture_load("models/enemy5/enemy5.png");
	self->color = color;
	self->position = position;
	self->bounds = gfc_box(position.x - 1, position.y - 1, position.z, 2, 2, 1.5);
	self->rotation = gfc_vector3d(0, 0, 0);
	//void (*draw)(Entity_S);
	self->think = enemy_think5;
	//void (*update)(Entity_S);

	return self;
}
void enemy_think5(Entity* self) {

}

void enemy_config(Entity* self, int enemyIndex) {
	SJson* enemyDef;

	if (!self) {
		return;
	}
	if (!enemyDefFile) {
		enemyDefFile = sj_load("defs/enemies.def");
		if (!enemyDefs) {
			enemyDefs = sj_object_get_value(enemyDefFile, "enemies");
		}
	}

	enemyDef = sj_array_get_nth(enemyDefs, enemyIndex);
	//continue implementing, adjust spawn functions
}