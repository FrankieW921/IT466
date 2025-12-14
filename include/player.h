#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "gf2d_sprite.h"

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
	Uint16 fuel;
	Mesh* bodyMesh;
	Texture* bodyTexture;
}Body;

typedef struct {
	const char* name[128];
	Uint16 health;
	float speed;
	Mesh* legMesh;
	Texture* legTexture;
}Leg;

typedef struct {
	const char* name[128];
	Uint16 damage;
	Uint16 cooldown;
	Mesh* weaponMesh;
	Texture* weaponTexture;
}Weapon;

typedef struct {
	Uint8 enabled;
	Uint8 selectedCategory; //0 for heads, 5 for shoulders
	const char* partDescription1[128];
	const char* partDescription2[128];
	const char* partDescription3[128];
	const char* partDescription4[128];
}PlayerUI;

typedef enum {
	MS_ON_GROUND,
	MS_FALLING,
	MS_FLYING
}MovementState;

typedef struct {
	Sint32			maxHealth;
	Sint32			currentHealth;
	Uint8			movementState;
	Uint16			maxFuel;
	Sint16			currentFuel;
	Uint16			dashCooldown;
	Uint16			fireCooldoown;
	PlayerUI*		ui;

	Sprite* reticle;
	Sprite* reticleLocked;

	Leg* leg;
	Body* body;
	Arm* arm;
	Head* head;
	Weapon* gun;
	Weapon* shoulder;

	Uint8			headIndex; //map to the lists
	Uint8			armIndex;
	Uint8			bodyIndex;
	Uint8			legIndex;
	Uint8			gunIndex;
	Uint8			shoulderIndex;
	Uint8			headIndexMax;
	Uint8			armIndexMax;
	Uint8			bodyIndexMax;
	Uint8			legIndexMax;
	Uint8			gunIndexMax;
	Uint8			shoulderIndexMax;

	SJson* heads; //store the def files because why not
	SJson* arms;
	SJson* bodies;
	SJson* legs;
	SJson* guns;
	SJson* shoulders;
	GFC_List* headInventory;
	GFC_List* armInventory;
	GFC_List* bodyInventory;
	GFC_List* legInventory;
	GFC_List* gunInventory;
	GFC_List* shoulderInventory;
}PlayerData;

//@brief returns the global static player
Entity* get_the_player();
//@brief creates the player at a given spawn position and change its color
Entity* player_spawn(GFC_Vector3D position, GFC_Color color);
Entity* editor_spawn(GFC_Vector3D position, GFC_Color color);
//@brief unique player think
void player_think(Entity* self);
//@brief unique player update
void player_update(Entity* self);
//@brief player dedicated move function, moves the player using their current state and world edge tests
void player_move(Entity* self);
//@brief populates a player's void data* with PlayerData
void player_data_new(PlayerData* data);

void editor_think(Entity* self);
void editor_update(Entity* self);

//@brief populates the editor's void data* with curated data from this function
void editor_data_new(PlayerData* data);
//@brief draws the player (body parts, weapons)
void player_draw(Entity* self, GFC_Vector3D lightPos, GFC_Color colorMod);

void player_ui_update(PlayerData* data);

void player_ui_draw();

void player_free();

void player_set_head(Head* currentHead, SJson* selectedHead);

void player_set_arm(Arm* currentArm, SJson* selectedArm);

void player_set_body(Body* currentBody, SJson* selectedBody);

void player_set_leg(Leg* currentLeg, SJson* selectedLeg);

void player_set_weapon(Weapon* currentWeapon, SJson* selectedWeapon);

void player_next_head(Entity* self);
void player_next_arm(Entity* self);
void player_next_body(Entity* self);
void player_next_leg(Entity* self);
void player_next_gun(Entity* self);
void player_next_shoulder(Entity* self);

void player_do_max_health(Entity* self);

void player_add_head(PlayerData* pData, SJson* headToAdd);
void player_add_arm(PlayerData* pData, SJson* armToAdd);
void player_add_body(PlayerData* pData, SJson* bodyToAdd);
void player_add_leg(PlayerData* pData, SJson* legToAdd);
void player_add_gun(PlayerData* pData, SJson* weaponToAdd);
void player_add_shoulder(PlayerData* pData, SJson* weaponToAdd);

void player_free_heads(PlayerData* pData);
void player_free_arms(PlayerData* pData);
void player_free_bodies(PlayerData* pData);
void player_free_legs(PlayerData* pData);
void player_free_weapons(PlayerData* pData);

#endif