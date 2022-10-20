#include "mesh_draw.h"
#include "mesh.h"

#include <components/transform/transform.h>

#include <core/shader/shader.h>
#include <ecs/ecs.h>
#include <loaders/loader_obj.h>
#include <model/model.h>

static void init(Entity e) {
    if(!(ecs_has(e, C_TRANSFORM))) return;
    if(!(ecs_has(e, C_MESH))) return;

    struct ComponentTransform *transform = ecs_get(e, C_TRANSFORM);
    struct ComponentMesh *mesh = ecs_get(e, C_MESH);

    // TODO: stupid hack grabbing only scale.x...
    Model *model = load_obj(mesh->modelPath, transform->scale[0]);

    mesh->model = model;

    // send lightspace matrix from renderer to entity shader
    ShaderProgram *shader = mesh->material->shaderProgram;
    shader_use(shader);
    glUniformMatrix4fv(
        glGetUniformLocation(shader->id, "u_LightSpaceMatrix"),
        1, GL_FALSE, (float *)e.ecs->renderer->lightSpaceMatrix);

    // TODO: send model matrix to shader
}

static void DrawMesh(
        struct ComponentMesh *mesh,
        struct ComponentTransform *transform,
        Material *material) {

    // Handle culling
    /*
    if((mesh->properties.cullMode | CULL_DISABLED)) {
        printf("Disable culling");
    }
    */

    material_use(material);
    glUniformMatrix4fv(
    glGetUniformLocation(material->shaderProgram->id, "u_Model"),
    1, GL_FALSE, (float *)transform->mvp.model);

    vao_bind(mesh->model->vao);

    ui32 vertices = mesh->model->vertexCount;
    ui32 indices  = mesh->model->indicesCount;

    ui16 drawMode = mesh->properties.drawMode;

    switch(drawMode) {
        case DRAW_ELEMENTS:
            glDrawElements(GL_TRIANGLES, indices, GL_UNSIGNED_INT, NULL);
        break;
        case DRAW_ARRAYS:
        default:
            glDrawArrays(GL_TRIANGLES, 0, vertices);
        break;
    }
}

static void render(Entity e) {
    if(!(ecs_has(e, C_TRANSFORM))) return;
    if(!(ecs_has(e, C_MESH))) return;
    struct ComponentTransform *transform = ecs_get(e, C_TRANSFORM);
    struct ComponentMesh *mesh = ecs_get(e, C_MESH);

    RenderPass pass = e.ecs->renderer->currentPass;

    // TODO: get lightspace matrix

    if(pass == REGULAR) {
        DrawMesh(mesh, transform, mesh->material);
        return;
    }
    Material *override = e.ecs->renderer->explicitMaterial;
    DrawMesh(mesh, transform, override);
}

void s_mesh_draw_init(ECS *ecs) {
    ecs_component_register(ecs, C_MESH, sizeof(struct ComponentMesh));
    ecs_system_register(ecs, ((ECSSystem){
        .init       = (ECSSubscriber) init,
        .destroy    = NULL,
        .render     = (ECSSubscriber) render,
        .update     = NULL,
    }));
}