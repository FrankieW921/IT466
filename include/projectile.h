#ifndef __PROJECTILE_H__
#define __PROJECTILE_H__

#include "entity.h"

typedef struct { //get by name??
	Uint8			projectileType; //0 for Scudder, 1 for DAC, 2 for Kurasawa
	Uint8			damage;
	Uint8			speed; //velocity (normalized) multiplier
	Uint16			timeToLive;
}ProjectileData;

Entity* projectile_spawn(Uint8 projectileType, GFC_Vector3D position, GFC_Vector3D velocity);

void projectile_think(Entity* self);

void projectile_update(Entity* self);

void projectile_move(Entity* self);

void projectile_free(Entity* self);

#endif 
