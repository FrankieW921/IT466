#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "entity.h"

typedef struct {
	void* data; //replace with ACTUAL data
}PlayerData;

Entity* get_the_player();

Entity* player_spawn(GFC_Vector3D position, GFC_Color color);

void player_think(Entity* self);

void player_update(Entity* self);

void player_data_new(PlayerData* data);

#endif