#ifndef H_RENDERER
#define H_RENDERER

#include <util/gfx.h>
#include <util/maths.h>
#include <util/sysdefs.h>

#include <core/graphics/lighting/lighting.h>
#include <core/graphics/lighting/skybox.h>
#include <core/graphics/material/material.h>
#include <core/graphics/renderer/renderer_props.inl>
#include <core/graphics/scene/scene.h>
#include <core/graphics/ui/billboard.h>

#include <core/graphics/renderer/pipeline/pass_shadowmap.h>
#include <core/graphics/renderer/pipeline/pass_ssao.h>

#include <core/drivers/vulkan/vulkan_device.h>

#include <tools/debug/debug_context.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RENDER_RESOLUTION_OVERRIDE SYS_DISABLED
#define PSX_WIDTH                  320
#define PSX_HEIGHT                 240

typedef enum renderPass { REGULAR = 0, DEPTH_PREPASS, SHADOW } RenderPass;

typedef struct _renderer Renderer;

struct _renderer
{
    GLFWwindow *window;
    RendererProps properties;      // Frame properties/configuration
    int windowWidth, windowHeight; // window resolution
    int renderWidth, renderHeight; // render resolution

    Scene **sceneL;
    ui16 sceneC, activeScene;

    RenderPass currentPass; // TODO: rename -> RenderStage
    Material *explicitMaterial;

    Billboard2D *billboard; // Billboard testing

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

    // TODO: Fix this shit as well.
    Light *light;
    ShadowmapOptions shadowmapOptions;
    SsaoOptions ssaoOptions;

    // TODO: still hacky shit
    VulkanDeviceContext *vulkanDevice;
    ui32 hdrTextureId;

    DebugContext *debugContext;
};

/**
 * Initialize the Renderer.
 * @return allocated Renderer structure
 */
Renderer *
renderer_init();

// Rendering Loop
void
renderer_start(Renderer *renderer);
void
renderer_tick(Renderer *renderer);

/* scene management */
struct _ecs *
renderer_active_scene(Renderer *self, ui16 sceneIndex);
//-------------------------------

// #endif // __cplusplus

#ifdef __cplusplus
}
#endif

#endif // H_RENDERER
