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
#include "gf3d_obj_load.h"
#include "gf3d_camera.h"

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
    Texture* defaultTexture;
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

//WORK WITH REFCOUNT NOT INSUE
Mesh* gf3d_mesh_load(const char* filename) {
    ObjData* objectData;
    MeshPrimitive* primitiveFromObject;
    Mesh* mesh;
    
    if (!filename) return NULL;

    //implement gf3d_mesh_get_by_file(const char* filename)

    objectData = gf3d_obj_load_from_file(filename);
    if (!objectData) {
        slog("failed to parse obj file %s", filename);
        return NULL;
    }
    mesh = gf3d_mesh_new();
    if (!mesh) {
        slog("failed to allocate mesh for file %s", filename);
        gf3d_obj_free(objectData);
        return NULL;
    }
    primitiveFromObject = gf3d_mesh_primitive_new();
    if (!primitiveFromObject) {
        slog("failed to allocate mesh primitive for file %s", filename);
        gf3d_obj_free(objectData);
        gf3d_mesh_free(mesh);
        return NULL;
    }

    gfc_list_append(mesh->primitives, primitiveFromObject);
    primitiveFromObject->objData = objectData;
    
    gf3d_mesh_primitive_create_vertex_buffer(primitiveFromObject);
    gf3d_mesh_primitive_create_face_buffer(primitiveFromObject); //name different from prof
    
    return mesh;
}

void gf3d_mesh_draw(Mesh* mesh, GFC_Matrix4 modelMat, GFC_Color mod, Texture* texture) {
    MeshUBO ubo;
    if (!mesh) return;

    ubo = gf3d_mesh_get_ubo(modelMat, mod);

    gf3d_mesh_queue_render(mesh, gf3d_mesh.pipe, &ubo, texture);
}

/*
void gf3d_mesh_draw_all() {
    int i;
    if (!&gf3d_mesh) return; 
    for (i = 0; i < gfc_list_count(gf3d_mesh.mesh_list); i++) {
        if (gf3d_mesh.mesh_list[i]._inuse) {
            gf3d_mesh_draw(gfc_list_nth(gf3d_mesh.mesh_list, i), , GFC_COLOR_WHITE, gf3d_mesh.defaultTexture);
        }
    }
}
figure out how to pass in model/identity matrix to draw*/

void gf3d_mesh_queue_render(Mesh* mesh, Pipeline* pipe, void* uboData, Texture* texture) {
    int i, c;
    MeshPrimitive *primitive;
    if (!mesh || !pipe || !uboData) {
        slog("Failed to queue mesh for render");
        return;
    }
    c = gfc_list_count(mesh->primitives);
    for (i = 0; i < c; i++) {
        primitive = gfc_list_nth(mesh->primitives, i);
        if (!primitive) continue;
        gf3d_mesh_primitive_queue_render(primitive, pipe, uboData, texture);
    }
}

void gf3d_mesh_primitive_queue_render(MeshPrimitive* primitive, Pipeline* pipe, void* uboData, Texture* texture) {
    if (!primitive || !pipe || !uboData) return NULL;
    if (!texture) texture = gf3d_mesh.defaultTexture;
    gf3d_pipeline_queue_render(pipe, primitive->vertexBuffer, primitive->vertexCount, primitive->faceBuffer, uboData, texture);
}

MeshPrimitive* gf3d_mesh_primitive_new() {
    return gfc_allocate_array(sizeof(MeshPrimitive), 1);
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
    //memset?
    free(mesh);
}

void gf3d_mesh_primitive_create_vertex_buffer(MeshPrimitive* primitive) {
    void* data = NULL;
    VkDevice device = gf3d_vgraphics_get_default_logical_device();
    Vertex* vertices;
    Uint32 vcount;
    size_t bufferSize;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    if (!primitive) {
        slog("no mesh primitive given to create vertex buffers for");
        return;
    }
 
    vertices = primitive->objData->faceVertices;
    vcount = primitive->objData->face_vert_count;
    bufferSize = sizeof(Vertex) * vcount;
    //create staging data
    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingBuffer, &stagingBufferMemory);
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices, (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);
    //copy staged data to the primitive
    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT|VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &primitive->vertexBuffer, &primitive->vertexBufferMemory);
    gf3d_buffer_copy(stagingBuffer, primitive->vertexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, NULL);
    vkFreeMemory(device, stagingBufferMemory, NULL);

    primitive->vertexCount = vcount;
}

void gf3d_mesh_primitive_create_face_buffer(MeshPrimitive* primitive) {
    void* data = NULL;
    VkDevice device = gf3d_vgraphics_get_default_logical_device();
    Face* faces;
    Uint32 fcount;
    size_t bufferSize;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    if (!primitive) {
        slog("no mesh primitive given to create vertex buffers for");
        return;
    }

    faces = primitive->objData->outFace; //i think its outface based on this being seperated like the vertices in the struct
    fcount = primitive->objData->face_count;
    bufferSize = sizeof(Face) * fcount;
    //create staging data
    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingBuffer, &stagingBufferMemory);
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, faces, (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);
    //copy staged data to the primitive
    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &primitive->faceBuffer, &primitive->faceBufferMemory);
    gf3d_buffer_copy(stagingBuffer, primitive->faceBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, NULL);
    vkFreeMemory(device, stagingBufferMemory, NULL);

    primitive->faceCount = fcount;
}

Pipeline* gf3d_mesh_get_pipeline() {
    return gf3d_mesh.pipe;
}

MeshUBO gf3d_mesh_get_ubo(GFC_Matrix4 modelMat, GFC_Color colorMod) {
    ModelViewProjection mvp;
    MeshUBO ubo = { 0 };

    GFC_Vector4D color = gfc_color_to_vector4(colorMod);
    mvp = gf3d_vgraphics_get_mvp();

    gfc_matrix4_copy(ubo.model, modelMat);
    gfc_matrix4_copy(ubo.view, mvp.view);
    gfc_matrix4_copy(ubo.proj, mvp.proj);
    gfc_vector4d_copy(ubo.color, color);
    ubo.camera = gfc_vector3dw(gf3d_camera_get_position(), 1.0);

    return ubo;
}