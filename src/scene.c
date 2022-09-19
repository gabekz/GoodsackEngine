#include "scene.h"

static float  rotation     = 0.0f;
static float  rotationInc  = 0.5f;
static double timePrev     = -1.0f;

void scene_update(Scene *self) {
    if((glfwGetTime() - timePrev) >= (1.0f / 60.0f)) {
        rotation += rotationInc;
    }
    if(self->id == 0) {
        shader_use(self->meshL[0]->material->shaderProgram);
        mat4 suzanne = GLM_MAT4_IDENTITY_INIT;
        glm_rotate(suzanne, glm_rad(rotation), (vec3){0.0f, 1.0f, 0.0f});
        glUniformMatrix4fv(
        glGetUniformLocation(self->meshL[0]->material->shaderProgram->id, "u_Model"),
        1, GL_FALSE, (float *)suzanne);

    #if 0
        shader_use(self->meshL[1]->material->shaderProgram);
        mat4 lightObj = GLM_MAT4_IDENTITY_INIT;
        glm_translate(lightObj, (vec3){0.0f, 0.1f, 0.4f});
        glUniformMatrix4fv(
            glGetUniformLocation(self->meshL[1]->material->shaderProgram->id, "u_Model"),
            1, GL_FALSE, (float *)lightObj);
    #endif
    }
}

void scene_draw(Scene *self, bool isExplicit, Material *override) {
    for (int i = 0; i < self->meshC; i++){

        // TEST: Send transform to shadowmap
        if(isExplicit) {
            model_send_matrix(self->meshL[i]->model, override->shaderProgram);
        }

        (isExplicit)
            ? mesh_draw_explicit(self->meshL[i], override)
            : mesh_draw(self->meshL[i]);
    }
}
