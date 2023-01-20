#ifndef H_RENDERER
#define H_RENDERER

#include <util/gfx.h>
#include <util/maths.h>
#include <util/sysdefs.h>

#include <core/lighting/lighting.h>
#include <core/lighting/skybox.h>
#include <core/scene/scene.h>

#include <model/material.h>

#include <core/api/vulkan/vulkan_device.h>

#define RENDER_RESOLUTION_OVERRIDE SYS_DISABLED
#define PSX_WIDTH                  320
#define PSX_HEIGHT                 240

typedef enum renderPass { REGULAR = 0, SHADOW } RenderPass;

typedef struct _renderer Renderer;

struct _renderer
{
    GLFWwindow *window;
    int windowWidth, windowHeight; // window resolution
    int renderWidth, renderHeight; // render resolution

    Scene **sceneL;
    ui16 sceneC, activeScene;

    RenderPass currentPass; // TODO: rename -> RenderStage
    Material *explicitMaterial;

    // Skybox test
    Skybox *skybox;

    // Hacky shit for temporary shadowmap values
    ShaderProgram *shaderDepthMap;
    Material *materialDepthMap;
    ui32 depthMapFBO;
    ui32 depthMapTexture;
    mat4 lightSpaceMatrix;

    ui32 drawCalls;
    ui32 faces;
    ui32 totalVertices;

    // TODO: still hacky shit
    VulkanDeviceContext *vulkanDevice;
    ui32 hdrTextureId;
};

/**
 * Initialize the Renderer.
 * @return allocated Renderer structure
 */
Renderer *
renderer_init();

void
renderer_add_light();

// Logical
void
renderer_fixedupdate();
void
renderer_update();

// Rendering Loop
void
renderer_start(Renderer *renderer);
void
renderer_tick(Renderer *renderer);

// Analytics
void
renderer_add_draw_data(ui32 drawCalls, ui32 vertices);

/* scene management */
struct _ecs *
renderer_active_scene(Renderer *self, ui16 sceneIndex);
//-------------------------------

//#endif // __cplusplus

#endif // H_RENDERER
