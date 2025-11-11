#ifndef __CAMERA_ENTITY_H__
#define __CAMERA_ENTITY_H__

#include "gfc_vector.h"
#include "gf3d_camera.h"
#include "entity.h"

typedef struct {
	GFC_Vector3D position;
	GFC_Vector3D target;
	float zOffset;
	Entity* player;
}CameraEntity;

CameraEntity* camera_entity_new();

void camera_think();

GFC_Vector3D get_view_vector();

Entity* camera_target_lock();

#endif
