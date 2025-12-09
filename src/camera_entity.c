#include "simple_logger.h"
#include "gfc_input.h"

#include "camera_entity.h"
#include "world.h"
#include "player.h"

static CameraEntity* ce;

CameraEntity* camera_entity_new() {
	if (!ce) {
		ce = gfc_allocate_array(sizeof(CameraEntity), 1);
	}
	ce->player = get_the_player();
	if (!ce->player) {
		slog("Failed to initialize camera ent, player null");
	}
	ce->position = gfc_vector3d(0, 0, 0);
	ce->target = gfc_vector3d(0, 0, 0);
	ce->zOffset = 10;
	return ce;
}

void camera_think() {
	if (!ce) return;
	if (!ce->player) return;
	GFC_Vector3D playerRotation = ce->player->rotation;
	GFC_Vector2D direction2d;
	direction2d = gfc_vector2d_from_angle(playerRotation.z);
	gfc_vector2d_normalize(&direction2d);

	GFC_Vector3D positionOffset = gfc_vector3d(-30, -30, 14); //be this far away from the player
	ce->target = ce->player->position;
	

	if (gfc_input_command_down("panup")) {
		ce->zOffset += .3;
	}
	if (gfc_input_command_down("pandown")) {
		ce->zOffset -= .3;
	}

	if (ce->zOffset > 17) {
		ce->zOffset = 17;
	}
	if (ce->zOffset < 3) {
		ce->zOffset = 3;
	}

	ce->position.x = ce->player->position.x + (direction2d.x * positionOffset.x);
	ce->position.y = ce->player->position.y + (direction2d.y * positionOffset.y);
	ce->position.z = ce->player->position.z + positionOffset.z;

	ce->target.z += ce->zOffset;

	gf3d_camera_look_at(ce->target, &ce->position);
}

GFC_Vector3D get_view_vector() {
	if (!ce) return;
	GFC_Vector3D vec = gfc_vector3d(ce->position.x - ce->target.x, ce->position.y - ce->target.y, ce->position.z - ce->target.z);
	gfc_vector3d_normalize(&vec);
	return vec;
}

Entity* camera_target_lock() {
	GFC_Vector3D entityVector;
	Entity* targetEnt;
	Entity* returnEnt = NULL;
	float dp = 0;
	GFC_Vector3D viewVector = get_view_vector();
	World* w = get_the_world();
	if (!w || !ce) return NULL;
	
	for (int i = 0; i < gfc_list_count(w->entities); i++) {
		targetEnt = gfc_list_get_nth(w->entities, i);
		entityVector = gfc_vector3d(ce->position.x - targetEnt->position.x, ce->position.y - targetEnt->position.y, ce->position.z - targetEnt->position.z);
		gfc_vector3d_normalize(&entityVector);
		dp = gfc_vector3d_dot_product(viewVector, entityVector);
		if (dp >= .995) {
			returnEnt = targetEnt;
		}
	}

	if (returnEnt == NULL) {
		return NULL;
	}
	else {
		return returnEnt;
	}
}

GFC_Vector3D camera_get_target() {
	if (!ce) return;
	return ce->target;
}

void camera_entity_free() {
	ce->player = NULL;
	memset(ce, 0, sizeof(CameraEntity));

}
