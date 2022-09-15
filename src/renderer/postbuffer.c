/* postbuffer.c ~ singleton framebuffer program */

#include "postbuffer.h"
#include "../glbuffer/glbuffer.h"
#include "../gfx.h"
#include "../shader.h"
#include <model/primitives.h>

#include <util/sysdefs.h>

#include <stdlib.h>
#include <stdio.h>

static ShaderProgram *shader;
static VAO *vaoRect;
static ui32 textureId, fboId;

void postbuffer_init(ui32 winWidth, ui32 winHeight) {
    // Shader
    shader = shader_create_program("../res/shaders/framebuffer.shader");
    shader_use(shader);
    glUniform1i(glGetUniformLocation(shader->id, "u_ScreenTexture"), 0);

    // Create Rectangle
    vaoRect = vao_create();
    vao_bind(vaoRect);
    float *rectPositions = prim_vert_rect();

#if 1
    VBO *vboRect = vbo_create(rectPositions, (2 * 3 * 4) * sizeof(float));
    vbo_bind(vboRect);
    vbo_push(vboRect, 2, GL_FLOAT, GL_FALSE);
    vbo_push(vboRect, 2, GL_FLOAT, GL_FALSE);
    vao_add_buffer(vaoRect, vboRect);
    free(rectPositions);
#else
    Material *matFrame = material_create(shader);
    Mesh *frameMesh = mesh_create_primitive(matFrame, PRIMITIVE_PLANE, 1.0f,
            0, 0, 0);
#endif


    // Create Texture
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, winWidth, winHeight,
        0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    // Create Framebuffer object
    glGenFramebuffers(1, &fboId);
    glBindFramebuffer(GL_FRAMEBUFFER, fboId);
    // Attach texture to FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D, textureId, 0);

    // Create Renderbuffer object [Depth]
    ui32 RBO;
    glGenRenderbuffers(1, &RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
        winWidth, winHeight);
    //glBindRenderbuffer(GL_RENDERBUFFER, 0); -- This doesn't have to be done?
    // Attach Renderbuffer
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
        GL_RENDERBUFFER, RBO);

    // Error checking
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE) {
        printf("\nFramebuffer ERROR: %u\n", status);
    }
}

void postbuffer_bind() {
    //glDebugMessageInsert(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_MARKER, 0,                       
    //    GL_DEBUG_SEVERITY_NOTIFICATION, -1, "Post Processing buffer init");

    glBindFramebuffer(GL_FRAMEBUFFER, fboId);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CW);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.00f, 1.0f);
}

void postbuffer_draw() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // Bind the backbuffer
        vao_bind(vaoRect);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glActiveTexture(GL_TEXTURE0);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        shader_use(shader);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
}

void postbuffer_cleanup() {
    glDeleteProgram(shader->id);
}
