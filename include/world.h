#ifndef __WORLD_H__
#define __WORLD_H__

#include "gf3d_mesh.h"
#include "entity.h"

typedef enum {
	MISSION_EDIT,
	MISSION_DESTORY,
	MISSION_SURVIVE,
	MISSION_RACE
}MissionType;

typedef struct {
	Mesh* mesh;
	Texture* texture;
	GFC_List* entities;
	GFC_Color color;
	GFC_Vector3D lightPosition;
	MissionType mission;
}World;

//@brief returns the global static world
World* get_the_world();
//@brief allocates a new world, intended for use in world_load
World* world_new();
//@brief loads a world from json config
//@param filename the local path of the json
//@returns a loaded world
void world_load(const char* filename);

void world_save(int worldIndex, const char* meshName, const char* textureName, MissionType mType);
void world_think(World* w);
//brief frees the given world
//@param w the world to free
void world_free(World* w);

void world_set_model(const char* meshName, const char* textureName);
//brief draws the given world
//@param w the world to draw
void world_draw(World* w);
//@brief tests for collisions between an entity's start and end positions from move, and the world
Uint8 world_edge_test(World* world, GFC_Vector3D start, GFC_Vector3D end, GFC_Vector3D* contact);

void world_enemy_spawn(int enemyType, GFC_Vector3D position, GFC_Color colorMod);

void world_free(World *w);

#endif