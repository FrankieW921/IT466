#include "simple_logger.h"
#include "gfc_input.h"

#include "camera_entity.h"
#include "player.h"

static CameraEntity* ce;

CameraEntity* camera_entity_new() {
	ce = gfc_allocate_array(sizeof(CameraEntity), 1);
	ce->player = get_the_player();
	if (!ce->player) {
		slog("Failed to initialize camera ent, player null");
	}
	ce->position = gfc_vector3d(0, 0, 0);
	ce->target = gfc_vector3d(0, 0, 0);
	ce->zOffset = 8;
	return ce;
}

void camera_think(CameraEntity* ce) {
	GFC_Vector3D playerRotation = ce->player->rotation;
	gfc_vector3d_negate(playerRotation, playerRotation);
	gfc_vector3d_normalize(&playerRotation);

	GFC_Vector3D positionOffset = gfc_vector3d(-20, -20, 12); //be this far away from the player
	ce->target = ce->player->position;

	if (gfc_input_command_down("panup")) {
		ce->zOffset += .3;
	}
	if (gfc_input_command_down("pandown")) {
		ce->zOffset -= .3;
	}

	if (ce->zOffset > 14) {
		ce->zOffset = 14;
	}
	if (ce->zOffset < -2) {
		ce->zOffset = -2;
	}

	ce->position.x = ce->player->position.x + (playerRotation.z * positionOffset.x);
	ce->position.y = ce->player->position.y + (playerRotation.z * positionOffset.y);
	ce->position.z = ce->player->position.z + positionOffset.z;

	ce->target.z += ce->zOffset;

	gf3d_camera_look_at(ce->target, &ce->position);
}

