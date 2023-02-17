#include "demo_scenes.h"

#include <ecs/ecs.h>

//#include <core/renderer/v1/renderer.h>

#include <components/components.h>

#define texture_create_d(x) texture_create(x, GL_SRGB_ALPHA, true, 16, renderer->vulkanDevice)
#define texture_create_n(x) texture_create(x, GL_RGB, false, 1, renderer->vulkanDevice)

Texture *texDefSpec, *texDefNorm, *texPbrAo;

static void _scene0 (ECS *ecs, Renderer *renderer)
{
    ecs = renderer_active_scene(renderer, 0);

    Texture *texEarthDiff =
      texture_create_d("../res/textures/earth/diffuse.png");
    Texture *texEarthNorm =
      texture_create_n("../res/textures/earth/normal.png");

    Material *matSuzanne = material_create(NULL,
                                           "../res/shaders/lit-diffuse.shader",
                                           3,
                                           texEarthDiff,
                                           texEarthNorm,
                                           texDefSpec);

    // Create the Camera, containing starting-position and up-axis coords.
    Entity camera = ecs_new(ecs);
   _ecs_add_internal(camera,
            C_CAMERA,
            (void *)(&(struct ComponentCamera) {
              .position = {0.0f, 0.0f, 2.0f},
              .axisUp   = {0.0f, 1.0f, 0.0f},
              .speed    = 0.05f,
            }));

    Entity suzanneObject = ecs_new(ecs);
   _ecs_add_internal(suzanneObject, C_TRANSFORM, NULL);
   _ecs_add_internal(suzanneObject,
            C_MESH,
            (void *)(&(struct ComponentMesh) {.material   = matSuzanne,
                                     .modelPath  = "../res/models/sphere.obj",
                                     .properties = {
                                       .drawMode = DRAW_ARRAYS,
                                       .cullMode = CULL_CW | CULL_FORWARD,
                                     }}));
}

static void _scene1 (ECS *ecs, Renderer *renderer)
{
    ecs = renderer_active_scene(renderer, 1);

    Texture *texContDiff =
      texture_create_d("../res/textures/container/diffuse.png");
    Texture *texContSpec =
      texture_create_n("../res/textures/container/specular.png");

    Texture *texBrickDiff =
      texture_create_d("../res/textures/brickwall/diffuse.png");
    Texture *texBrickNorm =
      texture_create_n("../res/textures/brickwall/normal.png");

    Material *matFloor = material_create(NULL,
                                         "../res/shaders/lit-diffuse.shader",
                                         3,
                                         texBrickDiff,
                                         texBrickNorm,
                                         texDefSpec);
    Material *matBox   = material_create(NULL,
                                       "../res/shaders/lit-diffuse.shader",
                                       3,
                                       texContDiff,
                                       texDefNorm,
                                       texContSpec);

    Entity camera2 = ecs_new(ecs);
   _ecs_add_internal(camera2,
            C_CAMERA,
            (void *)(&(struct ComponentCamera) {
              .position = {0.0f, 0.0f, 2.0f},
              .axisUp   = {0.0f, 1.0f, 0.0f},
              .speed    = 0.05f,
            }));

    Entity floorEntity = ecs_new(ecs);
   _ecs_add_internal(floorEntity,
            C_TRANSFORM,
            (void *)(&(struct ComponentTransform) {
              .position = {0.0f, -0.3f, 0.0f},
              .scale    = {10.0f, 10.0f, 10.0f},
            }));
   _ecs_add_internal(floorEntity,
            C_MESH,
            (void *)(&(struct ComponentMesh) {.material   = matFloor,
                                     .modelPath  = "../res/models/plane.obj",
                                     .properties = {
                                       .drawMode = DRAW_ARRAYS,
                                       .cullMode = CULL_CW | CULL_FORWARD,
                                     }}));

    Entity boxEntity = ecs_new(ecs);
   _ecs_add_internal(boxEntity,
            C_TRANSFORM,
            (void *)(&(struct ComponentTransform) {
              .position = {0.0f, -0.085f, 0.0f},
            }));
   _ecs_add_internal(boxEntity,
            C_MESH,
            (void *)(&(struct ComponentMesh) {.material  = matBox,
                                     .modelPath = "../res/models/cube-test.obj",
                                     .properties = {
                                       .drawMode = DRAW_ARRAYS,
                                       .cullMode = CULL_CW | CULL_FORWARD,
                                     }}));
}

static void _scene2 (ECS *ecs, Renderer *renderer)
{
    ecs = renderer_active_scene(renderer, 2);

    Entity camera3 = ecs_new(ecs);
   _ecs_add_internal(camera3,
            C_CAMERA,
            (void *)(&(struct ComponentCamera) {
              .position = {0.0f, 0.0f, 2.0f},
              .axisUp   = {0.0f, 1.0f, 0.0f},
              .speed    = 0.05f,
            }));

    Texture *texGraniteAlbedo =
      texture_create_d("../res/textures/pbr/granite/albedo.png");
    Texture *texGraniteNormal =
      texture_create_n("../res/textures/pbr/granite/normal.png");
    Texture *texGraniteMetallic =
      texture_create_n("../res/textures/pbr/granite/metallic.png");
    Texture *texGraniteSpecular =
      texture_create_n("../res/textures/pbr/granite/roughness.png");
    Texture *texGraniteAo =
      texture_create_n("../res/textures/pbr/granite/ao.png");
    Material *matGranite = material_create(NULL,
                                           "../res/shaders/pbr.shader",
                                           5,
                                           texGraniteAlbedo,
                                           texGraniteNormal,
                                           texGraniteMetallic,
                                           texGraniteSpecular,
                                           texGraniteAo);

    Texture *texPbrAlbedo =
      texture_create_d("../res/textures/pbr/rust/albedo.png");
    Texture *texPbrNormal =
      texture_create_n("../res/textures/pbr/rust/normal.png");
    Texture *texPbrMetallic =
      texture_create_n("../res/textures/pbr/rust/metallic.png");
    Texture *texPbrSpecular =
      texture_create_n("../res/textures/pbr/rust/roughness.png");
    Material *matRust = material_create(NULL,
                                        "../res/shaders/pbr.shader",
                                        5,
                                        texPbrAlbedo,
                                        texPbrNormal,
                                        texPbrMetallic,
                                        texPbrSpecular,
                                        texPbrAo);

    Texture *texBrassAlbedo =
      texture_create_d("../res/textures/pbr/fancybrass/albedo.png");
    Texture *texBrassNormal =
      texture_create_n("../res/textures/pbr/fancybrass/normal.png");
    Texture *texBrassMetallic =
      texture_create_n("../res/textures/pbr/fancybrass/metallic.png");
    Texture *texBrassSpecular =
      texture_create_n("../res/textures/pbr/fancybrass/roughness.png");
    Texture *texBrassAo =
      texture_create_n("../res/textures/pbr/fancybrass/ao.png");
    Material *matBrass = material_create(NULL,
                                        "../res/shaders/pbr.shader",
                                        5,
                                        texBrassAlbedo,
                                        texBrassNormal,
                                        texBrassMetallic,
                                        texBrassSpecular,
                                        texBrassAo);

    Texture *texGoldAlbedo =
      texture_create_d("../res/textures/pbr/gold/albedo.png");
    Texture *texGoldNormal =
      texture_create_n("../res/textures/pbr/gold/normal.png");
    Texture *texGoldMetallic =
      texture_create_n("../res/textures/pbr/gold/metallic.png");
    Texture *texGoldSpecular =
      texture_create_n("../res/textures/pbr/gold/roughness.png");
    Material *matGold = material_create(NULL,
                                        "../res/shaders/pbr.shader",
                                        5,
                                        texGoldAlbedo,
                                        texGoldNormal,
                                        texGoldMetallic,
                                        texGoldSpecular,
                                        texPbrAo);
    Entity sphereEntity = ecs_new(ecs);
    _ecs_add_internal(sphereEntity,
            C_TRANSFORM,
            (void *)(&(struct ComponentTransform) {
              .position = {0.0f, 0.0f, 0.0f},
            }));
    _ecs_add_internal(sphereEntity,
            C_MESH,
            (void *)(&(struct ComponentMesh) {.material   = matGranite,
                                     .modelPath  = "../res/models/sphere.obj",
                                     .properties = {
                                       .drawMode = DRAW_ARRAYS,
                                       .cullMode = CULL_CW | CULL_FORWARD,
                                     }}));
    Entity sphereEntity2 = ecs_new(ecs);
   _ecs_add_internal(sphereEntity2,
            C_TRANSFORM,
            (void *)(&(struct ComponentTransform) {
              .position = {-0.5f, 0.0f, 0.0f},
            }));
   _ecs_add_internal(sphereEntity2,
            C_MESH,
            (void *)(&(struct ComponentMesh) {.material   = matRust,
                                     .modelPath  = "../res/models/sphere.obj",
                                     .properties = {
                                       .drawMode = DRAW_ARRAYS,
                                       .cullMode = CULL_CW | CULL_FORWARD,
                                     }}));

    Entity sphereEntity3 = ecs_new(ecs);
   _ecs_add_internal(sphereEntity3,
            C_TRANSFORM,
            (void *)(&(struct ComponentTransform) {
              .position = {0.5f, 0.0f, 0.0f},
            }));
   _ecs_add_internal(sphereEntity3,
            C_MESH,
            (void *)(&(struct ComponentMesh) {.material   = matBrass,
                                     .modelPath  = "../res/models/sphere.obj",
                                     .properties = {
                                       .drawMode = DRAW_ARRAYS,
                                       .cullMode = CULL_CW | CULL_FORWARD,
                                     }}));
    Entity sphereEntity4 = ecs_new(ecs);
   _ecs_add_internal(sphereEntity4,
            C_TRANSFORM,
            (void *)(&(struct ComponentTransform) {
              .position = {1.0f, 0.0f, 0.0f},
            }));
   _ecs_add_internal(sphereEntity4,
            C_MESH,
            (void *)(&(struct ComponentMesh) {.material   = matGold,
                                     .modelPath  = "../res/models/sphere.obj",
                                     .properties = {
                                       .drawMode = DRAW_ARRAYS,
                                       .cullMode = CULL_CW | CULL_FORWARD,
                                     }}));

   #if 0
    Entity floorEntity2 = ecs_new(ecs);
   _ecs_add_internal(floorEntity2,
            C_TRANSFORM,
            (void *)(&(struct ComponentTransform) {
              .position = {0.0f, -0.3f, 0.0f},
              .scale    = {10.0f, 10.0f, 10.0f},
            }));
   _ecs_add_internal(floorEntity2,
            C_MESH,
            (void *)(&(struct ComponentMesh) {.material   = matFloor,
                                     .modelPath  = "../res/models/plane.obj",
                                     .properties = {
                                       .drawMode = DRAW_ARRAYS,
                                       .cullMode = CULL_CW | CULL_FORWARD,
                                     }}));
   #endif
}

static void _scene3(ECS *ecs, Renderer *renderer) {

    ecs = renderer_active_scene(renderer, 3);

    Texture *texCerbA =
      texture_create_d("../res/textures/pbr/cerberus/Cerberus_A.tga");
    Texture *texCerbN =
      texture_create_n("../res/textures/pbr/cerberus/Cerberus_N.tga");
    Texture *texCerbM =
      texture_create_n("../res/textures/pbr/cerberus/Cerberus_M.tga");
    Texture *texCerbS =
      texture_create_n("../res/textures/pbr/cerberus/Cerberus_R.tga");

    Material *matCerb = material_create(NULL,
                                        "../res/shaders/pbr.shader",
                                        5,
                                        texCerbA,
                                        texCerbN,
                                        texCerbM,
                                        texCerbS,
                                        texPbrAo);

    struct ComponentCamera compCamera = {
      .position = {-1.5f, 0.0f, 0.0f},
      .axisUp   = {0.0f, 1.0f, 0.0f},
      .speed    = 0.05f,
    };
    /*
    struct ComponentCamera *compCamera =
    (&(struct ComponentCamera({
        .position = {-1.5f, 0.0f, 0.0f},
        .axisUp   = {0.0f, 1.0f, 0.0f},
        .speed    = 0.05f,
    })));
    */

    Entity camera4 = ecs_new(ecs);
    _ecs_add_internal(camera4, C_CAMERA, (void *)((struct ComponentCamera *)&compCamera));
   //_ecs_add_internal(camera4, C_AUDIO_LISTENER, NULL);

    Entity entCerb = ecs_new(ecs);

    struct ComponentTransform compCerbTransform = {
        .position = {0.0f, 0.0f, 0.0f},
        .scale    = {4.0f, 4.0f, 4.0f},
   };
    struct ComponentMesh compCerbMesh = {
        .material = matCerb,
        .modelPath = "../res/models/cerberus-triang.obj",
                                         .properties = {
                                           .drawMode = DRAW_ARRAYS,
                                           .cullMode = CULL_CW | CULL_FORWARD,
                                         }
    };

    _ecs_add_internal(entCerb,
            C_TRANSFORM,
            (void *)((struct ComponentTransform *)&compCerbTransform));

    _ecs_add_internal(entCerb,
            C_MESH,
            (void *)((struct ComponentMesh *)&compCerbMesh));


}

void demo_scenes_create(ECS *ecs, Renderer *renderer)
{
    /*
    Entity camera = ecs_new(ecs);
    _ecs_add_internal(camera,
            C_CAMERA,
            (void *)(&(struct ComponentCamera){
              .position = {0.0f, 0.0f, 2.0f},
              .axisUp   = {0.0f, 1.0f, 0.0f},
              .speed    = 0.05f,
            }));
    */
    
    // Default textures
    texDefSpec =
      texture_create_n("../res/textures/defaults/specular.png");
    texDefNorm =
      texture_create_n("../res/textures/defaults/normal.png");
    texPbrAo = texture_create_n("../res/textures/defaults/white.png");


   //_scene0(ecs, renderer);
   //_scene1(ecs, renderer);
   //_scene2(ecs, renderer);
   _scene3(ecs, renderer);
}