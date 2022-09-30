#ifndef H_RENDERER
#define H_RENDERER

#include <util/sysdefs.h>
#include <util/gfx.h>
#include <util/maths.h>

#include <core/scene.h>

#include <model/material.h>


typedef struct _renderer Renderer;

typedef enum renderPass {REGULAR = 0, SHADOW} RenderPass;

struct _renderer {
    GLFWwindow *window;
    int windowWidth, windowHeight;

    Scene **sceneL;
    ui16  sceneC, activeScene;

    RenderPass currentPass;
    Material *explicitMaterial;
    mat4 lightSpaceMatrix;
};

Renderer* renderer_init();

void renderer_add_light();

// Logical
void renderer_fixedupdate();
void renderer_update();

// Rendering Loop
void renderer_tick(Renderer *renderer);

/* scene management */
struct _ecs *renderer_active_scene(Renderer* self, ui16 sceneIndex);
//-------------------------------

#endif // H_RENDERER
