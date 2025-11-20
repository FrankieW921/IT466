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
	gfc_vector2d_copy(self->position, position);
	gfc_vector2d_copy(self->velocity, velocity);

	projectileObject = sj_array_get_nth(projectilesDefs, projectileType);
	data = gfc_allocate_array(sizeof(ProjectileData), 1);
	if (data) {
		data->projectileType = projectileType;
		sj_object_get_int(projectileObject, "damage", &data->damage);
		sj_object_get_int(projectileObject, "speed", &data->speed);
		sj_object_get_int(projectileObject, "timeToLive", &data->timeToLive);
	}
	self->data = data;
	//self->mesh = gf3d_mesh_load("models/"); do this along with scaling?

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
	//keep looking at 2d project

}