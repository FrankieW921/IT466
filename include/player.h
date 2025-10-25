#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "entity.h"

typedef struct {
	const char* name[128];
	Uint8 health;
	Mesh* headMesh;
	Texture* headTexture;
}Head;

typedef struct {
	const char* name[128];
	Uint8 health;
	Mesh* armMesh;
	Texture* armTexture;
}Arm;

typedef struct {
	const char* name[128];
	Uint8 health;
	Mesh* bodyMesh;
	Texture* bodyTexture;
}Body;

typedef struct {
	const char* name[128];
	Uint8 health;
	Mesh* legMesh;
	Texture* legTexture;
}Leg;

typedef struct {
	const char* name[128];
	Uint8 damage;
	Uint8 cooldown;
}Weapon;

typedef struct {
	Sint32			maxHealth;
	Sint32			currentHealth;

	Leg* leg;
	Body* body;
	Arm* arm;
	Head* head;
	Weapon* armWeapon;
	Weapon* shoulderWeapon;

	Uint8			headIndex;
	Uint8			armIndex;
	Uint8			bodyIndex;
	Uint8			legIndex;
	Uint8			headIndexMax;
	Uint8			armIndexMax;
	Uint8			bodyIndexMax;
	Uint8			legIndexMax;

	SJson* heads;
	SJson* arms;
	SJson* bodies;
	SJson* legs;
	GFC_List* headInventory;
	GFC_List* armInventory;
	GFC_List* bodyInventory;
	GFC_List* legInventory;
}PlayerData;

Entity* get_the_player();

Entity* player_spawn(GFC_Vector3D position, GFC_Color color);

void player_think(Entity* self);

void player_update(Entity* self);

void player_data_new(PlayerData* data);

#endif