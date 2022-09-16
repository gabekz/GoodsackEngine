#ifndef RENDERER_H
#define RENDERER_H

#include "gfx.h"
#include "camera.h"
#include <model/mesh.h>
#include <lighting/lighting.h>

#include <util/sysdefs.h>

typedef struct _renderer Renderer;

typedef struct _light Light;
typedef struct _scene Scene;

struct _renderer {
    GLFWwindow *window;
    int windowWidth, windowHeight;

    Camera *activeCamera;

    Scene *sceneList;
    ui16  sceneCount, sceneIndex;
};

Renderer* renderer_init();

// Logical
void renderer_fixedupdate();
void renderer_update();

// Rendering Loop
void renderer_tick();
//-------------------------------

struct _scene {
    ui32 id;
    Mesh **meshL;
    Light **lightL;

    /* scene steps::
     * 1) Initialize and set active camera
     * 2) Create Light information
     [Standard]
     * 3) Renderer tick [all logic updates + shader updates]
     * 4) Render [all meshes in meshList]
     [ECS]
     * 3) Process Systems
     */
};


#endif
