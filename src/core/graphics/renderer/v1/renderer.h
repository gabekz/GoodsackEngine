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
#include <core/graphics/ui/gui_element.h>
#include <core/graphics/ui/gui_text.h>

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

#define MAX_CAMERAS 4

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
    GuiElement *uiImage;    // GuiElement test
    GuiText *uiText;        // GuiText test

    Skybox *activeSkybox;  // Active skybox that is being rendered
    Skybox *defaultSkybox; // Default skybox set for each scene on creation

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

    // Camera information
    struct
    {
        ui32 uboId, uboSize;
        // CameraData **cameras; // List of cameras

        ui32 totalCameras; // TODO: find an alternative
        ui32 activeCamera;
    } camera_data;
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

/**
 * Sets the active scene for the renderer. Will create a new scene
 * if the specified index does not yet exist.
 *
 * @param[in] self Pointer to the renderer
 * @param[in] sceneIndex Index of the scene to load/create
 * @return Pointer to the ECS struct owned by the scene
 */
struct _ecs *
renderer_active_scene(Renderer *self, ui16 sceneIndex);
//-------------------------------

// #endif // __cplusplus

#ifdef __cplusplus
}
#endif

#endif // H_RENDERER
