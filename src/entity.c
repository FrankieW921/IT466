#include "simple_logger.h"

#include "entity.h"

typedef struct {
	Entity* entity_list;
	Uint32 entity_max;
}EntitySystem;

static EntitySystem entity_system = { 0 };

Entity* entity_new() {
	int i;
	if (entity_system.entity_list) {
		for (i = 0; i < entity_system.entity_max; i++) {
			if (!entity_system.entity_list[i]._inuse) {
				entity_system.entity_list[i]._inuse = 1;
				//set defaults like color and texture
				entity_system.entity_list[i].color = GFC_COLOR_WHITE;
				entity_system.entity_list[i].scale = gfc_vector3d(1, 1, 1);
				return &entity_system.entity_list[i];
			}
		}
	}
	return NULL;
}

void entity_free(Entity* e) {
	if (!e) return;
	gf3d_mesh_free(e->mesh);
	gf3d_texture_free(e->texture);
	memset(e, 0, sizeof(Entity));
}

void entity_system_init(Uint8 maxEnts) {
	if (!maxEnts) {
		slog("Invalid maxEnts for entity system init");
		return;
	}
	entity_system.entity_list = gfc_allocate_array(sizeof(Entity), maxEnts);
	if (!entity_system.entity_list) {
		slog("Failed to allocate entities in entity system");
		return;
	}
	entity_system.entity_max = maxEnts;
	slog("Entity system initialized with %i entities", entity_system.entity_max);
	atexit(entity_system_close);
}

void entity_draw(Entity* ent, GFC_Vector3D lightPos, GFC_Color colorMod) {
	GFC_Matrix4 modelMat;
	if (!ent) return;
	gfc_matrix4_from_vectors(modelMat, ent->position, ent->rotation, ent->scale);
	gf3d_mesh_draw(ent->mesh,
		modelMat,
		ent->color,
		ent->texture,
		lightPos,
		colorMod);
}

void entity_system_close() {
	int i;
	if (entity_system.entity_list) {
		for (i = 0; i < entity_system.entity_max; i++) {
			if (entity_system.entity_list[i]._inuse) {
				entity_free(&entity_system.entity_list[i]);
			}
		}
	}
}

void entity_draw_all(GFC_Vector3D lightPos, GFC_Color colorMod) {
	int i;
	for (i = 0; i < entity_system.entity_max; i++) {
		if (!entity_system.entity_list[i]._inuse) continue;
		entity_draw(&entity_system.entity_list[i], lightPos, colorMod);
	}
}

void entity_think_all();

void entity_update_all();