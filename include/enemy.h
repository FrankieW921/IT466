#ifndef __ENEMY_H__
#define __ENEMY_H__

#include "entity.h"

typedef struct {
	Uint16 fireCooldown;
	Uint16 fireCooldownSet;
	Uint16 seeingRange;
	Uint8 projectileIndex;
}EnemyData;

Entity* enemy_spawn1(GFC_Vector3D position, GFC_Color colorMod);
Entity* enemy_spawn2(GFC_Vector3D position, GFC_Color colorMod);
Entity* enemy_spawn3(GFC_Vector3D position, GFC_Color colorMod);
Entity* enemy_spawn4(GFC_Vector3D position, GFC_Color colorMod);
Entity* enemy_spawn5(GFC_Vector3D position, GFC_Color colorMod);
//@brief runs thinks that all enemies need like firing cooldown and rotating towards the player
void enemy_thinkg(Entity* self, EnemyData* eData, Entity* player, GFC_Vector3D playerVector);

void enemy_think1(Entity* self);
void enemy_think2(Entity* self);
void enemy_think3(Entity* self);
void enemy_think4(Entity* self);
void enemy_think5(Entity* self);

void enemy_updateg(Entity* self, EnemyData* eData);
void enemy_update1(Entity* self);
void enemy_update2(Entity* self);
void enemy_update3(Entity* self);
void enemy_update4(Entity* self);
void enemy_update5(Entity* self);

void enemy_config(Entity* self, int enemyIndex);
void enemy_free(Entity* self);

#endif