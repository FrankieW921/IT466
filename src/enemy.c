#include "simple_logger.h"

#include "enemy.h"

Entity* enemy_spawn1(GFC_Vector3D position, GFC_Color color) {
	Entity* self;
	self = entity_new();
	if (!self)return;

	gfc_line_cpy(self->name, "Heli");
	self->mesh = gf3d_mesh_load("models/enemy1/enemy1.obj");
	self->texture = gf3d_texture_load("models/enemy1/enemy1.png");
	self->color = color;
	self->position = position;
	self->rotation = gfc_vector3d(0, 0, 0);
	//void (*draw)(Entity_S);
	//void (*think)(Entity_S);
	//void (*update)(Entity_S);

	return self;
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
	self->rotation = gfc_vector3d(0, 0, 0);
	//void (*draw)(Entity_S);
	//void (*think)(Entity_S);
	//void (*update)(Entity_S);

	return self;
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
	self->rotation = gfc_vector3d(0, 0, 0);
	//void (*draw)(Entity_S);
	//void (*think)(Entity_S);
	//void (*update)(Entity_S);

	return self;
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
	self->rotation = gfc_vector3d(0, 0, 0);
	//void (*draw)(Entity_S);
	//void (*think)(Entity_S);
	//void (*update)(Entity_S);

	return self;
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
	self->rotation = gfc_vector3d(0, 0, 0);
	//void (*draw)(Entity_S);
	//void (*think)(Entity_S);
	//void (*update)(Entity_S);

	return self;
}