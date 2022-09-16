#ifndef H_RENDERER
#define H_RENDERER

#include "gfx.h"
#include "camera.h"
#include <model/mesh.h>
#include <lighting/lighting.h>

#include <util/sysdefs.h>

#include "../scene.h"

typedef struct _renderer Renderer;
typedef struct _light Light;

struct _renderer {
    GLFWwindow *window;
    int windowWidth, windowHeight;

    Camera *activeCamera;

    Scene **sceneL;
    ui16  sceneC, activeScene;
};

Renderer* renderer_init();

void renderer_add_mesh(Renderer* self, Mesh* mesh);
void renderer_add_light();

// Logical
void renderer_fixedupdate();
void renderer_update();

// Rendering Loop
void renderer_tick();

/* scene management */
void renderer_active_scene(Renderer* self, ui16 sceneIndex);
//-------------------------------

#endif // H_RENDERER
