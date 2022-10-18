#include "camera.h"

#include <util/gfx.h>
#include <ecs/ecs.h>

static void init(Entity e) {
    if(!(ecs_has(e, C_CAMERA))) return;
    struct ComponentCamera *camera = ecs_get(e, C_CAMERA);

    camera->screen.width  = e.ecs->renderer->windowWidth;
    camera->screen.height = e.ecs->renderer->windowHeight;

    // Defaults
    if(camera->fov <= 0) {
        camera->fov = 45.0f;
    }
    if(camera->clipping.nearZ <= 0) {
        camera->clipping.nearZ = 0.1f;
    }
    if(camera->clipping.farZ <= 0) {
        camera->clipping.farZ = 100.0f;
    }

// initialize default view and projection matrices
    mat4 m4i = GLM_MAT4_IDENTITY_INIT;
    glm_mat4_copy(m4i, camera->mvp.view);
    glm_mat4_copy(m4i, camera->mvp.proj);

// Create UBO for camera data
    ui32 uboId;
    ui32 uboSize = 4 + sizeof(vec3) + (2 * sizeof(mat4));
    glGenBuffers(1, &uboId);
    glBindBuffer(GL_UNIFORM_BUFFER, uboId);
    glBufferData(GL_UNIFORM_BUFFER, uboSize, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboId, 0, uboSize);
    camera->uboId = uboId;

    float *axisUp = camera->axisUp;
    float *center = GLM_VEC3_ZERO; // position + orientation _v
    glm_vec3_mul(camera->position, (vec3){0.0f, 0.0f, -1.0f}, center);

}

static void update(Entity e) {
    if(!(ecs_has(e, C_CAMERA))) return;
    struct ComponentCamera *camera = ecs_get(e, C_CAMERA);
    
    float *axisUp = camera->axisUp;
    float *center = GLM_VEC3_ZERO; // position + orientation _v
    glm_vec3_mul(camera->position, (vec3){0.0f, 0.0f, -1.0f}, center);
    glm_lookat(camera->position, center, axisUp, camera->mvp.view);

    float aspectRatio = (float)camera->screen.width / (float)camera->screen.height;
    glm_perspective(glm_rad(camera->fov), aspectRatio,
            camera->clipping.nearZ, camera->clipping.farZ, camera->mvp.proj);

// Update camera UBO
        glBindBuffer(GL_UNIFORM_BUFFER, camera->uboId);
        glBufferSubData(GL_UNIFORM_BUFFER,
            0, sizeof(vec3) + 4,
            camera->position);
        glBufferSubData(GL_UNIFORM_BUFFER,
            sizeof(vec3) + 4, sizeof(mat4),
            camera->mvp.proj);
        glBufferSubData(GL_UNIFORM_BUFFER,
            sizeof(mat4) + sizeof(vec3) + 4, sizeof(mat4),
            camera->mvp.view);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Process Camera Input
    camera_input(camera, e.ecs->renderer->window);
}

void s_camera_init(ECS *ecs) {
    ecs_component_register(ecs, C_CAMERA, sizeof(struct ComponentCamera));
    ecs_system_register(ecs, ((ECSSystem){
        .init       = (ECSSubscriber) init,
        .destroy    = NULL,
        .render     = NULL,
        .update     = (ECSSubscriber) update,
    }));
}
