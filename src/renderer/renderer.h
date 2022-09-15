#ifndef RENDERER_H
#define RENDERER_H

#include "gfx.h"
#include "camera.h"

typedef struct _renderer Renderer;
typedef struct _model Model;

struct _renderer {
    Camera *camera;
    GLFWwindow *win;
    Model *models;
};

// TODO: Textures should be on shaders. Model is only
// to make sure that the VAO is speaking with the shader,
// and that we can pack all the information we need to
// RENDER the model.
//
// Later.. the Entity will handle logical updates as per the model,
// i.e., things such as updating the position, hitboxes, etc.. 
//
// On a second thought, we may not want the texture to be a part of
// the shader. Hell, we are going to use the same shader for multiple
// types of objects.. But again, we are using the shader code, not exactly
// the shader STRUCT itself.. Interesting. Something to think about.
struct _model {
    const char *vertShader;
    const char *fragShader;
    Texture *diffuse;
    Texture *specular;
    VAO *vao;
};

void renderer_init(Renderer *self, int winWidth, int winHeight);
void renderer_tick();

#endif
