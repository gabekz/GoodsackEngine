#include "pass_compute.h"

#include <core/api/opengl/opengl.h>
#include <core/texture/texture.h>
#include <core/shader/shader.h>

#include <model/primitives.h>

#include <core/api/device.h>

static ui32 csTexture;
static ShaderProgram *csShader;
static ShaderProgram *shader2;
static VAO *vaoRect;

void computebuffer_init() {

    // shader Program
    const char *csPath = "../res/shaders/hello.compute";
    csShader = shader_create_compute_program(csPath);
    shader2 = shader_create_program("../res/shaders/framebuffer-simple.shader");

     // texture size
    const ui32 TEXTURE_WIDTH = 1000, TEXTURE_HEIGHT = 1000;
    
    glGenTextures(1, &csTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, csTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, 
                 GL_FLOAT, NULL);

    // Create Rectangle
    vaoRect = vao_create();
    vao_bind(vaoRect);
    float *rectPositions = prim_vert_rect();
    VBO *vboRect = vbo_create(rectPositions, (2 * 3 * 4) * sizeof(float));
    vbo_bind(vboRect);
    vbo_push(vboRect, 2, GL_FLOAT, GL_FALSE);
    vbo_push(vboRect, 2, GL_FLOAT, GL_FALSE);
    vao_add_buffer(vaoRect, vboRect);
    free(rectPositions);
}

void computebuffer_draw() {

    glBindImageTexture(0, csTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    shader_use(csShader);
    glUniform1f(glGetUniformLocation(csShader->id, "t"),
            device_getAnalytics().delta);
    glDispatchCompute((ui32)1000/10, (ui32)1000/10, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader_use(shader2);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, csTexture);
    vao_bind(vaoRect);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
}
