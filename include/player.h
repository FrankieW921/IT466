#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "entity.h"

typedef struct {
	const char* name[128];
	Uint16 health;
	Mesh* headMesh;
	Texture* headTexture;
}Head;

typedef struct {
	const char* name[128];
	Uint16 health;
	Mesh* armMesh;
	Texture* armTexture;
}Arm;

typedef struct {
	const char* name[128];
	Uint16 health;
	Mesh* bodyMesh;
	Texture* bodyTexture;
}Body;

typedef struct {
	const char* name[128];
	Uint16 health;
	Mesh* legMesh;
	Texture* legTexture;
}Leg;

typedef struct {
	const char* name[128];
	Uint16 damage;
	Uint16 cooldown;
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

//@brief returns the global static player
Entity* get_the_player();
//@brief creates the player at a given spawn position and change its color
Entity* player_spawn(GFC_Vector3D position, GFC_Color color);
//@brief unique player think
void player_think(Entity* self);
//@brief unique player update
void player_update(Entity* self);
//@brief populates a player's void data* with PlayerData
void player_data_new(PlayerData* data);
//@brief draws the player (body parts, weapons)
void player_draw(Entity* self, GFC_Vector3D lightPos, GFC_Color colorMod);

#endif