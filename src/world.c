#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_config.h"
#include "gfc_text.h"
#include "gf3d_obj_load.h"
#include "world.h"
#include "enemy.h"

static World* theWorld;

World* get_the_world() {
	if (!theWorld) {
		slog("No static world to return");
		return NULL;
	}
	return theWorld;
}

World* world_new() {
	World* world;
	world = gfc_allocate_array(sizeof(World), 1);
	if (!world)return NULL;

	return world;
}

void world_load(const char* filename) {
	const char* str;
	SJson* json, *config, *enemies, *enemy;
	int enemyIndex, enemyX, enemyY, enemyZ;

	json = sj_load(filename);
	if (!json) {
		slog("Failed to load world file %s", filename);
		return NULL;
	}
	if (!theWorld) { //check if we already have space allocated for 
		theWorld = world_new();
	}
	
	if (!theWorld) {
		slog("Failed to allocate world for file %s", filename);
		return NULL;
	}

	config = sj_object_get_value(json, "world");
	str = sj_object_get_value_as_string(config, "filename");
	theWorld->mesh = gf3d_mesh_load(str);
	str = sj_object_get_value_as_string(config, "texture");
	theWorld->texture = gf3d_texture_load(str);
	sj_object_get_color_value(config, "color", &theWorld->color);
	sj_object_get_vector3d(config, "lightPosition", &theWorld->lightPosition);
	sj_object_get_int(config, "mission", &theWorld->mission);

	theWorld->entities = gfc_list_new();
	enemies = sj_object_get_value(json, "enemies");
	for (int i = 0; i < sj_array_get_count(enemies); i++) {
		enemy = sj_array_get_nth(enemies, i);
		sj_object_get_int(enemy, "enemyIndex", &enemyIndex);
		sj_object_get_int(enemy, "x", &enemyX);
		sj_object_get_int(enemy, "y", &enemyY);
		sj_object_get_int(enemy, "z", &enemyZ);
		world_enemy_spawn(enemyIndex, gfc_vector3d(enemyX, enemyY, enemyZ), GFC_COLOR_WHITE);
	}
	sj_free(json);



	//theWorld = world; a remnant of a kinder time
	//return world; Im gonna miss ur classes Professor Kehoe
}

void world_free(World* w) { //because the world is static im not going to deallocate the space sorry not sorry
	if (!w) return;
	gf3d_mesh_free(w->mesh);
	gf3d_texture_free(w->texture);
	//for loop clearing the entities in the list
	for (int i = 0; i < gfc_list_count(w->entities); i++) {
		entity_free(gfc_list_get_nth(w->entities, i));
	}
	gfc_list_clear(w->entities);
}

void world_draw(World* w) {
	if (!w) return;
	GFC_Matrix4 id;
	gfc_matrix4_identity(id);
	gf3d_mesh_draw(w->mesh, id, GFC_COLOR_WHITE, w->texture, w->lightPosition, w->color);
}

Uint8 world_edge_test(World* world, GFC_Vector3D start, GFC_Vector3D end, GFC_Vector3D* contact) {
	int i, j, pCount, fCount;
	GFC_Edge3D edge;
	GFC_Triangle3D tri;
	MeshPrimitive* primitive;

	if (!world) return NULL;

	edge = gfc_edge3d_from_vectors(start, end);
	pCount = gfc_list_count(world->mesh->primitives);

	for (i = 0; i < pCount; i++) {
		primitive = gfc_list_nth(world->mesh->primitives, i);
		if ((!primitive) || (!primitive->objData)) continue;
		fCount = primitive->objData->face_count;
		for (j = 0; j < fCount; j++) {
			tri.a = primitive->objData->faceVertices[primitive->objData->outFace[j].verts[0]].vertex;
			tri.b = primitive->objData->faceVertices[primitive->objData->outFace[j].verts[1]].vertex;
			tri.c = primitive->objData->faceVertices[primitive->objData->outFace[j].verts[2]].vertex;
			if (gfc_trigfc_angle_edge_test(edge, tri, contact)) {
				return 1; 
			}
		}
	}

	return 0;
}

void world_enemy_spawn(int enemyType, GFC_Vector3D position, GFC_Color colorMod) {
	Entity* enemy;
	if (!theWorld) return;

	if (enemyType == 1) {
		enemy = enemy_spawn1(position, colorMod);
	}
	else if (enemyType == 2) {
		enemy = enemy_spawn2(position, colorMod);
	}
	else if (enemyType == 3) {
		enemy = enemy_spawn3(position, colorMod);
	}
	else if (enemyType == 4) {
		enemy = enemy_spawn4(position, colorMod);
	}
	else if (enemyType == 5) {
		enemy = enemy_spawn5(position, colorMod);
	}
	else {
		slog("Invalid enemy type");
		return;
	}
	
	if (!enemy) return NULL;
	gfc_list_append(theWorld->entities, enemy);

}