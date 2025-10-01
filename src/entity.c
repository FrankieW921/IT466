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
				//set defaults
				return &entity_system.entity_list[i];
			}
		}
	}
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

void entity_draw_all();

void entity_think_all();

void entity_update_all();