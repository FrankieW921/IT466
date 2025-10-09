#include "simple_logger.h"

#include "monster.h"

Entity* monster_spawn(GFC_Vector3D position, GFC_Color color) {
	Entity* self;
	self = entity_new();
	if (!self)return;

	gfc_line_cpy(self->name, "Pikachu");
	self->mesh = gf3d_mesh_load("models/dino/dino.obj");
	self->texture = gf3d_texture_load("models/dino/dino.png");;
	self->color = color;
	self->position = position;
	self->rotation = gfc_vector3d(0, 0, 0);
	//void (*draw)(Entity_S);
	//void (*think)(Entity_S);
	//void (*update)(Entity_S);

	return self;
}