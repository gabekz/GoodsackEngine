#ifndef RENDERER_H
#define RENDERER_H

#include "gfx.h"
#include "camera.h"

typedef struct _renderer Renderer;
typedef struct _model Model;

struct _renderer {
    Camera *activeCamera;

    GLFWwindow *window;
    int windowWidth, windowHeight;
    //Model *models;
};

Renderer* renderer_init();
void renderer_tick();

#endif
