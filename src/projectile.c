#include "simple_logger.h"

//#include "gfc_audio.h"

#include "projectile.h"

static SJson* projectileDefFile = NULL;
static SJson* projectilesDefs = NULL;


Entity* projectile_spawn(Uint8 projectileType, GFC_Vector3D position, GFC_Vector3D velocity) {
	Entity* self;
	ProjectileData* data;
	SJson* projectileObject;
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

	self->position.x += self->velocity.x * data->speed;
	self->position.y += self->velocity.y * data->speed;
	self->position.z += self->velocity.z * data->speed;
	self->bounds.x = self->position.x - 1;
	self->bounds.y = self->position.y - 1;
	self->bounds.z = self->position.z - 1;

	if (data->timeToLive <= 0) projectile_free(self);
}

void projectile_free(Entity* self) {
	ProjectileData* data;
	if (!self) return;
	data = self->data;

	memset(data, 0, sizeof(ProjectileData)); //all ints, okay
	entity_free(self);
}