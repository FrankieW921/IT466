#include <stdalign.h>

#include "simple_logger.h"

#include "gfc_types.h"
#include "gfc_shape.h"

#include "gf3d_buffers.h"
#include "gf3d_swapchain.h"
#include "gf3d_vgraphics.h"
#include "gf3d_pipeline.h"
#include "gf3d_commands.h"
#include "gf3d_mesh.h"

#define MESH_ATTRIBUTE_COUNT 3

extern int __DEBUG;

typedef struct {
    Mesh*       mesh_list;
    Uint32      max_meshes;
    Uint32      chain_length;
    VkDevice    device;
    Pipeline*   pipe;
    VkBuffer    faceBuffer;
    VkVertexInputAttributeDescription   attributeDescriptions[MESH_ATTRIBUTE_COUNT];
    VkVertexInputBindingDescription     bindingDescription;
    float       drawOrder;
}MeshManager;

static MeshManager gf3d_mesh = { 0 };

void gf3d_mesh_init(Uint32 meshMax) { //really thankful i have 2 codebases to work with, sprite's here and my 2D game
    if (!meshMax) {
        slog("Cannot initialize mesh system with 0 meshes");
        return;
    }
    gf3d_mesh.mesh_list = gfc_allocate_array(sizeof(Mesh), meshMax);
    if (!gf3d_mesh.mesh_list)
    {
        slog("failed to allocate %i meshes", meshMax);
        return;
    }
    gf3d_mesh.max_meshes = meshMax;
    atexit(gf3d_mesh_close);
    slog("entity system initialized");
}

void gf3d_mesh_close(){ //needs some other methods first


}

Mesh* gf3d_mesh_new() {
    int i;
    for (i = 0; i < gf3d_mesh.max_meshes; i++)
    {
        if (gf3d_mesh.mesh_list[i]._inuse)continue;
        gf3d_mesh.mesh_list[i]._inuse = 1;
        return &gf3d_mesh.mesh_list[i];
    }
    slog("gf3d_mesh_new: no free slots for new meshes");
    return NULL;
}

/**
 * @brief load mesh data from an obj filename.
 * @note: currently only supporting obj files
 * @note this free's the intermediate data loaded from the obj file, no longer needed for most applications
 * @param filename the name of the file to load
 * @return NULL on error or Mesh data
 */
Mesh* gf3d_mesh_load(const char* filename) {

}

MeshPrimitive* gf3d_mesh_primitive_new() {
    MeshPrimitive* r;
    r = gfc_allocate_array(sizeof(MeshPrimitive), 1);
    return r;
}


/**
 * @brief get the input attribute descriptions for mesh based rendering
 * @param count (optional, output) the number of attributes
 * @return a pointer to a vertex input attribute description array
 */
VkVertexInputAttributeDescription* gf3d_mesh_get_attribute_descriptions(Uint32* count);

/**
 * @brief get the binding description for mesh based rendering
 * @return vertex input binding descriptions compatible with mesh data
 */
VkVertexInputBindingDescription* gf3d_mesh_get_bind_description();

void gf3d_mesh_free(Mesh* mesh) {
    /*
    GFC_TextLine        filename;
    Uint32              _refCount;
    Uint8               _inuse;
    GFC_List* primitives;
    GFC_Box             bounds; */
    gfc_list_delete(mesh->primitives);
    memset(mesh, 0, sizeof(Mesh));
}

/**
 * @brief create a mesh's internal buffers based on vertices
 * @param primitive the mesh primitive to populate
 * @note the primitive must have the objData set and it must have be organizes in buffer order
 */
void gf3d_mesh_create_vertex_buffer_from_vertices(MeshPrimitive* primitive);

//gf3d_mesh_setup_face_buffers

/**
 * @brief get the pipeline that is used to render basic 3d meshes
 * @return NULL on error or the pipeline in question
 */
Pipeline* gf3d_mesh_get_pipeline();

/**
 * @brief given a model matrix and basic color, build the meshUBO needed to render a model
 * @param modelMat the model Matrix
 * @param colorMod the color for the UBO
 */
MeshUBO gf3d_mesh_get_ubo(
    GFC_Matrix4 modelMat,
    GFC_Color colorMod);