#ifndef H_RENDERER
#define H_RENDERER

#include <util/sysdefs.h>
#include <util/gfx.h>

#include <core/lighting.h>
#include <core/scene.h>
#include <model/mesh.h>
#include <model/material.h>


typedef struct _renderer Renderer;
typedef struct _light Light;

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
