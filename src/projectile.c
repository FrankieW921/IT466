#include "simple_logger.h"

//#include "gfc_audio.h"

#include "projectile.h"
#include "world.h"

static SJson* projectileDefFile = NULL;
static SJson* projectilesDefs = NULL;


Entity* projectile_spawn(Uint8 projectileType, GFC_Vector3D position, GFC_Vector3D velocity) {
	Entity* self;
	ProjectileData* data;
	SJson* projectileObject;
	SJson* colorArray;
	Uint8 colorR = 0, colorG = 0, colorB = 0;
	//GFC_Sound* projectileSound;

	if (!projectileDefFile) {
		projectileDefFile = sj_load("defs/projectiles.def");
		if (!projectilesDefs) {
			projectilesDefs = sj_object_get_value(projectileDefFile, "projectiles");
		}
	}

	self = entity_new();
	if (!self) return;

	self->think = projectile_think;
	self->update = projectile_update;
	gfc_vector3d_copy(self->position, position);
	gfc_vector3d_copy(self->velocity, velocity);
	self->bounds = gfc_box(self->position.x - 1, self->position.y - 1, self->position.z - 1, 2, 2, 2);
	
	projectileObject = sj_array_get_nth(projectilesDefs, projectileType);
	data = gfc_allocate_array(sizeof(ProjectileData), 1);
	if (data) {
		data->projectileType = projectileType;
		sj_object_get_int(projectileObject, "damage", &data->damage);
		sj_object_get_int(projectileObject, "speed", &data->speed);
		sj_object_get_int(projectileObject, "timeToLive", &data->timeToLive);
	}
	self->data = data;
	self->mesh = gf3d_mesh_load("models/projectile/projectile.obj");
	self->texture = gf3d_texture_load("models/projectile/projectile.png");
	colorArray = sj_object_get_value(projectileObject, "color");
	sj_get_uint8_value(sj_array_get_nth(colorArray, 0), &colorR);
	sj_get_uint8_value(sj_array_get_nth(colorArray, 1), &colorG);
	sj_get_uint8_value(sj_array_get_nth(colorArray, 2), &colorB);
	self->color = gfc_color8(colorR, colorG, colorB, 255);
}

void projectile_think(Entity* self) {
	ProjectileData* data;
	data = self->data;
	data->timeToLive -= 1;
}

void projectile_update(Entity* self) {
	ProjectileData* data;
	data = self->data;
	if (!data) {
		return;
	}

	projectile_move(self);
	self->bounds.x = self->position.x - 1;
	self->bounds.y = self->position.y - 1;
	self->bounds.z = self->position.z - 1;

	if (data->timeToLive == 0) projectile_free(self);
}

void projectile_move(Entity* self) {
	GFC_Box bounds;
	GFC_Vector3D positionPre, positionPost, contact, scaledVelocity;
	ProjectileData* data;
	data = self->data;
	if (!data) {
		return;
	}

	gfc_vector3d_scale(scaledVelocity, self->velocity, data->speed);
	gfc_vector3d_copy(positionPre, self->position);
	gfc_vector3d_add(positionPost, self->position, scaledVelocity);
	if (world_edge_test(get_the_world(), positionPre, positionPost, &contact)) {
		data->timeToLive = 0;
	}
	else {
		gfc_vector3d_copy(self->position, positionPost);
	}
}

void projectile_free(Entity* self) {
	ProjectileData* data;
	if (!self) return;
	data = self->data;

	memset(data, 0, sizeof(ProjectileData)); //all ints, okay
	entity_free(self);
}