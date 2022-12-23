#ifndef H_RENDERER
#define H_RENDERER

#include <util/sysdefs.h>
#include <util/gfx.h>
#include <util/maths.h>

#include <core/renderer/scene.h>
#include <core/lighting/lighting.h>

#include <model/material.h>

#include <core/api/vulkan/vulkan_device.h>

typedef enum renderPass {REGULAR = 0, SHADOW} RenderPass;

typedef struct _renderer Renderer;

struct _renderer {
    GLFWwindow *window;
    int windowWidth, windowHeight;

    Scene **sceneL;
    ui16  sceneC, activeScene;

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

    // TODO: still hacky shit
    VulkanDeviceContext *vulkanDevice;
};

/**
 * Initialize the Renderer.
 * @return allocated Renderer structure
 */
Renderer* renderer_init();

void renderer_add_light();

// Logical
void renderer_fixedupdate();
void renderer_update();

// Rendering Loop
void renderer_start(Renderer *renderer);
void renderer_tick(Renderer *renderer);

/* scene management */
struct _ecs *renderer_active_scene(Renderer* self, ui16 sceneIndex);
//-------------------------------

//#endif // __cplusplus

#endif // H_RENDERER
