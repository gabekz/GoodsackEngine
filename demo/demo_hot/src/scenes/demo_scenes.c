/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "demo_scenes.h"

#include "util/filesystem.h"

#include "entity/ecs.h"
#include "entity/modules/modules_systems.h"

#include "asset/asset.h"
#include "asset/asset_cache.h"
#include "physics/physics_types.h"

#define LOAD_SCENE(index)   GLUE(_scene, index)(ecs, renderer)
#define LOAD_SCENE_2(index) GLUE(_scene, index)(ecs, renderer, asset_cache)

gsk_Texture *texDefSpec, *texDefNorm, *texPbrAo, *texMissing;
gsk_Skybox *skyboxMain;

static gsk_Entity
__create_camera_entity(gsk_ECS *ecs, vec3 position, void *parent)
{
    gsk_Entity camera = gsk_ecs_new(ecs);
    _gsk_ecs_add_internal(
      camera,
      C_CAMERA,
      (void *)(&(struct ComponentCamera) {
        .axisUp      = {0.0f, 1.0f, 0.0f},
        .renderLayer = 0, // DEFAULT RENDER LAYER (camera-zero)
      }));
    _gsk_ecs_add_internal(camera,
                          C_CAMERALOOK,
                          (void *)(&(struct ComponentCameraLook) {
                            .sensitivity = 1.0f,
                          }));
    _gsk_ecs_add_internal(camera,
                          C_CAMERAMOVEMENT,
                          (void *)(&(struct ComponentCameraMovement) {
                            .speed = 5.0f,
                          }));
    _gsk_ecs_add_internal(camera,
                          C_TRANSFORM,
                          (void *)(&(struct ComponentTransform) {
                            .position = {position[0], position[1], position[2]},
                            .parent   = parent,
                          }));
    return camera;
}

static void
_scene0(gsk_ECS *ecs, gsk_Renderer *renderer)
{
    ecs = gsk_renderer_active_scene(renderer, 0);

    gsk_Texture *texEarthDiff =
      texture_create_d(GSK_PATH("data://textures/earth/diffuse.png"));
    gsk_Texture *texEarthNorm =
      texture_create_n(GSK_PATH("data://textures/earth/normal.png"));
    texture_create_n(GSK_PATH("data://textures/earth/normal.png"));

    gsk_Material *matSuzanne =
      gsk_material_create(NULL,
                          GSK_PATH("gsk://shaders/lit-diffuse.shader"),
                          3,
                          texEarthDiff,
                          texEarthNorm,
                          texDefSpec);

    gsk_Entity camera =
      __create_camera_entity(ecs, (vec3) {0.0f, 0.0f, 2.0f}, NULL);

    gsk_Entity suzanneObject = gsk_ecs_new(ecs);
    _gsk_ecs_add_internal(suzanneObject, C_TRANSFORM, NULL);
    _gsk_ecs_add_internal(suzanneObject,
                          C_MODEL,
                          (void *)(&(struct ComponentModel) {
                            .material   = matSuzanne,
                            .modelPath  = GSK_PATH("gsk://models/sphere.obj"),
                            .properties = {
                              .drawMode = DRAW_ARRAYS,
                              .cullMode = CULL_CW | CULL_FORWARD,
                            }}));
}

static void
_scene1(gsk_ECS *ecs, gsk_Renderer *renderer)
{
    ecs = gsk_renderer_active_scene(renderer, 1);

    gsk_Texture *texContDiff = texture_create_d(
      "../demo/demo_hot/Resources/textures/container/diffuse.png");
    gsk_Texture *texContSpec = texture_create_n(
      "../demo/demo_hot/Resources/textures/container/specular.png");

    gsk_Texture *texBrickDiff = texture_create_d(
      "../demo/demo_hot/Resources/textures/brickwall/diffuse.png");
    gsk_Texture *texBrickNorm = texture_create_n(
      "../demo/demo_hot/Resources/textures/brickwall/normal.png");

    gsk_Material *matFloor =
      gsk_material_create(NULL,
                          "../res/shaders/lit-diffuse.shader",
                          3,
                          texBrickDiff,
                          texBrickNorm,
                          texDefSpec);
    gsk_Material *matBox =
      gsk_material_create(NULL,
                          "../res/shaders/lit-diffuse.shader",
                          3,
                          texContDiff,
                          texDefNorm,
                          texContSpec);

    gsk_Entity camera =
      __create_camera_entity(ecs, (vec3) {0.0f, 0.0f, 2.0f}, NULL);

    gsk_Entity floorEntity = gsk_ecs_new(ecs);
    _gsk_ecs_add_internal(floorEntity,
                          C_TRANSFORM,
                          (void *)(&(struct ComponentTransform) {
                            .position = {0.0f, -0.3f, 0.0f},
                            .scale    = {10.0f, 10.0f, 10.0f},
                          }));
    _gsk_ecs_add_internal(
      floorEntity,
      C_MODEL,
      (void *)(&(struct ComponentModel) {.material  = matFloor,
                                         .modelPath = "../res/models/plane.obj",
                                         .properties = {
                                           .drawMode = DRAW_ARRAYS,
                                           .cullMode = CULL_CW | CULL_FORWARD,
                                         }}));

    gsk_Entity boxEntity = gsk_ecs_new(ecs);
    _gsk_ecs_add_internal(boxEntity,
                          C_TRANSFORM,
                          (void *)(&(struct ComponentTransform) {
                            .position = {0.0f, -0.085f, 0.0f},
                          }));
    _gsk_ecs_add_internal(
      boxEntity,
      C_MODEL,
      (void *)(&(struct ComponentModel) {.material   = matBox,
                                         .modelPath  = "../res/models/cube.obj",
                                         .properties = {
                                           .drawMode = DRAW_ARRAYS,
                                           .cullMode = CULL_CW | CULL_FORWARD,
                                         }}));
}

static void
_scene2(gsk_ECS *ecs, gsk_Renderer *renderer)
{
    ecs = gsk_renderer_active_scene(renderer, 2);
    __set_active_scene_skybox(renderer, skyboxMain);

    gsk_Texture *texGraniteAlbedo = texture_create_d(
      "../demo/demo_hot/Resources/textures/pbr/granite/albedo.png");
    gsk_Texture *texGraniteNormal = texture_create_n(
      "../demo/demo_hot/Resources/textures/pbr/granite/normal.png");
    gsk_Texture *texGraniteMetallic = texture_create_n(
      "../demo/demo_hot/Resources/textures/pbr/granite/metallic.png");
    gsk_Texture *texGraniteSpecular = texture_create_n(
      "../demo/demo_hot/Resources/textures/pbr/granite/roughness.png");
    gsk_Texture *texGraniteAo = texture_create_n(
      "../demo/demo_hot/Resources/textures/pbr/granite/ao.png");
    gsk_Material *matGranite = gsk_material_create(NULL,
                                                   "../res/shaders/pbr.shader",
                                                   5,
                                                   texGraniteAlbedo,
                                                   texGraniteNormal,
                                                   texGraniteMetallic,
                                                   texGraniteSpecular,
                                                   texGraniteAo);

    gsk_Texture *texPbrAlbedo = texture_create_d(
      "../demo/demo_hot/Resources/textures/pbr/rust/albedo.png");
    gsk_Texture *texPbrNormal = texture_create_n(
      "../demo/demo_hot/Resources/textures/pbr/rust/normal.png");
    gsk_Texture *texPbrMetallic = texture_create_n(
      "../demo/demo_hot/Resources/textures/pbr/rust/metallic.png");
    gsk_Texture *texPbrSpecular = texture_create_n(
      "../demo/demo_hot/Resources/textures/pbr/rust/roughness.png");
    gsk_Material *matRust = gsk_material_create(NULL,
                                                "../res/shaders/pbr.shader",
                                                5,
                                                texPbrAlbedo,
                                                texPbrNormal,
                                                texPbrMetallic,
                                                texPbrSpecular,
                                                texPbrAo);

    gsk_Texture *texBrassAlbedo = texture_create_d(
      "../demo/demo_hot/Resources/textures/pbr/fancybrass/albedo.png");
    gsk_Texture *texBrassNormal = texture_create_n(
      "../demo/demo_hot/Resources/textures/pbr/fancybrass/normal.png");
    gsk_Texture *texBrassMetallic = texture_create_n(
      "../demo/demo_hot/Resources/textures/pbr/fancybrass/metallic.png");
    gsk_Texture *texBrassSpecular = texture_create_n(
      "../demo/demo_hot/Resources/textures/pbr/fancybrass/roughness.png");
    gsk_Texture *texBrassAo = texture_create_n(
      "../demo/demo_hot/Resources/textures/pbr/fancybrass/ao.png");
    gsk_Material *matBrass = gsk_material_create(NULL,
                                                 "../res/shaders/pbr.shader",
                                                 5,
                                                 texBrassAlbedo,
                                                 texBrassNormal,
                                                 texBrassMetallic,
                                                 texBrassSpecular,
                                                 texBrassAo);

    gsk_Texture *texGoldAlbedo = texture_create_d(
      "../demo/demo_hot/Resources/textures/pbr/gold/albedo.png");
    gsk_Texture *texGoldNormal = texture_create_n(
      "../demo/demo_hot/Resources/textures/pbr/gold/normal.png");
    gsk_Texture *texGoldMetallic = texture_create_n(
      "../demo/demo_hot/Resources/textures/pbr/gold/metallic.png");
    gsk_Texture *texGoldSpecular = texture_create_n(
      "../demo/demo_hot/Resources/textures/pbr/gold/roughness.png");
    gsk_Material *matGold = gsk_material_create(NULL,
                                                "../res/shaders/pbr.shader",
                                                5,
                                                texGoldAlbedo,
                                                texGoldNormal,
                                                texGoldMetallic,
                                                texGoldSpecular,
                                                texPbrAo);

    gsk_Entity camera =
      __create_camera_entity(ecs, (vec3) {0.0f, 0.0f, 2.0f}, NULL);

    gsk_Entity sphereEntity = gsk_ecs_new(ecs);
    _gsk_ecs_add_internal(sphereEntity,
                          C_TRANSFORM,
                          (void *)(&(struct ComponentTransform) {
                            .position = {0.0f, 0.0f, 0.0f},
                          }));
    _gsk_ecs_add_internal(sphereEntity,
                          C_MODEL,
                          (void *)(&(struct ComponentModel) {
                            .material   = matGranite,
                            .modelPath  = "../res/models/sphere.obj",
                            .properties = {
                              .drawMode = DRAW_ARRAYS,
                              .cullMode = CULL_CW | CULL_FORWARD,
                            }}));
    gsk_Entity sphereEntity2 = gsk_ecs_new(ecs);
    _gsk_ecs_add_internal(sphereEntity2,
                          C_TRANSFORM,
                          (void *)(&(struct ComponentTransform) {
                            .position = {-0.5f, 0.0f, 0.0f},
                          }));
    _gsk_ecs_add_internal(sphereEntity2,
                          C_MODEL,
                          (void *)(&(struct ComponentModel) {
                            .material   = matRust,
                            .modelPath  = "../res/models/sphere.obj",
                            .properties = {
                              .drawMode = DRAW_ARRAYS,
                              .cullMode = CULL_CW | CULL_FORWARD,
                            }}));

    gsk_Entity sphereEntity3 = gsk_ecs_new(ecs);
    _gsk_ecs_add_internal(sphereEntity3,
                          C_TRANSFORM,
                          (void *)(&(struct ComponentTransform) {
                            .position = {0.5f, 0.0f, 0.0f},
                          }));
    _gsk_ecs_add_internal(sphereEntity3,
                          C_MODEL,
                          (void *)(&(struct ComponentModel) {
                            .material   = matBrass,
                            .modelPath  = "../res/models/sphere.obj",
                            .properties = {
                              .drawMode = DRAW_ARRAYS,
                              .cullMode = CULL_CW | CULL_FORWARD,
                            }}));
    gsk_Entity sphereEntity4 = gsk_ecs_new(ecs);
    _gsk_ecs_add_internal(sphereEntity4,
                          C_TRANSFORM,
                          (void *)(&(struct ComponentTransform) {
                            .position = {1.0f, 0.0f, 0.0f},
                          }));
    _gsk_ecs_add_internal(sphereEntity4,
                          C_MODEL,
                          (void *)(&(struct ComponentModel) {
                            .material   = matGold,
                            .modelPath  = "../res/models/sphere.obj",
                            .properties = {
                              .drawMode = DRAW_ARRAYS,
                              .cullMode = CULL_CW | CULL_FORWARD,
                            }}));

#if 0
    gsk_Entity floorEntity2 = gsk_ecs_new(ecs);
   _ecs_add_internal(floorEntity2,
            C_TRANSFORM,
            (void *)(&(struct ComponentTransform) {
              .position = {0.0f, -0.3f, 0.0f},
              .scale    = {10.0f, 10.0f, 10.0f},
            }));
   _ecs_add_internal(floorEntity2,
            C_MODEL,
            (void *)(&(struct ComponentModel) {.material   = matFloor,
                                     .modelPath  = "../demo/demo_hot/Resources/models/plane.obj",
                                     .properties = {
                                       .drawMode = DRAW_ARRAYS,
                                       .cullMode = CULL_CW | CULL_FORWARD,
                                     }}));
#endif
}

static void
_scene3(gsk_ECS *ecs, gsk_Renderer *renderer)
{

    ecs = gsk_renderer_active_scene(renderer, 3);
    __set_active_scene_skybox(renderer, skyboxMain);

    gsk_Texture *texCerbA =
      texture_create_d(GSK_PATH("data://textures/pbr/cerberus/Cerberus_A.tga"));
    gsk_Texture *texCerbN =
      texture_create_n(GSK_PATH("data://textures/pbr/cerberus/Cerberus_N.tga"));
    gsk_Texture *texCerbM =
      texture_create_n(GSK_PATH("data://textures/pbr/cerberus/Cerberus_M.tga"));
    gsk_Texture *texCerbS =
      texture_create_n(GSK_PATH("data://textures/pbr/cerberus/Cerberus_R.tga"));

    gsk_Material *matCerb =
      gsk_material_create(NULL,
                          GSK_PATH("gsk://shaders/pbr.shader"),
                          5,
                          texCerbA,
                          texCerbN,
                          texCerbM,
                          texCerbS,
                          texPbrAo);

    gsk_Entity camera =
      __create_camera_entity(ecs, (vec3) {0.0f, 1.0f, 0.0f}, NULL);

    gsk_Model *model_cerb = gsk_model_load_from_file(
      GSK_PATH("data://models/cerberus-triang.obj"), 1.0f, NULL);

#if 1
    gsk_Entity entCerb                          = gsk_ecs_new(ecs);
    struct ComponentTransform compCerbTransform = {
      .position = {0.0f, 0.0f, 0.0f},
      .scale    = {4.0f, 4.0f, 4.0f},
    };
    struct ComponentModel compCerbMesh = {.material   = matCerb,
                                          .pModel     = model_cerb,
                                          .properties = {
                                            .drawMode = DRAW_ARRAYS,
                                            .cullMode = CULL_CW | CULL_FORWARD,
                                          }};
    _gsk_ecs_add_internal(
      entCerb,
      C_TRANSFORM,
      (void *)((struct ComponentTransform *)&compCerbTransform));

    _gsk_ecs_add_internal(
      entCerb, C_MODEL, (void *)((struct ComponentModel *)&compCerbMesh));
#endif
}

static void
_scene4(gsk_ECS *ecs, gsk_Renderer *renderer)
{
    ecs = gsk_renderer_active_scene(renderer, 4);

    gsk_Material *matCharacter = gsk_material_create(
      NULL, GSK_PATH("gsk://shaders/skinning-test.shader"), 0);

    gsk_Model *modelCharacter = gsk_model_load_from_file(
      GSK_PATH("data://models/character-anim.gltf"), 1, FALSE);

    gsk_Entity camera =
      __create_camera_entity(ecs, (vec3) {0.0f, 1.0f, 0.0f}, NULL);

    gsk_Entity characterEntity = gsk_ecs_new(ecs);
    _gsk_ecs_add_internal(characterEntity,
                          C_TRANSFORM,
                          (void *)(&(struct ComponentTransform) {
                            .position = {0.0f, 0.0f, 0.0f},
                            //.scale = {0.001f, 0.001f, 0.001f},
                          }));
    _gsk_ecs_add_internal(
      characterEntity,
      C_MODEL,
      (void *)(&(struct ComponentModel) {.material   = matCharacter,
                                         .pModel     = modelCharacter,
                                         .properties = {
                                           .drawMode = DRAW_ELEMENTS,
                                           .cullMode = CULL_CW | CULL_FORWARD,
                                         }}));
    _gsk_ecs_add_internal(characterEntity, C_ANIMATOR, NULL);
}

static void
_scene5(gsk_ECS *ecs, gsk_Renderer *renderer)
{
    ecs = gsk_renderer_active_scene(renderer, 5);
    __set_active_scene_skybox(renderer, skyboxMain);

    gsk_Material *matWire =
      gsk_material_create(NULL, GSK_PATH("gsk://shaders/wireframe.shader"), 0);

    gsk_Entity *pCamera = malloc(sizeof(gsk_Entity));
    *pCamera = __create_camera_entity(ecs, (vec3) {-1.2f, 0.5f, 0.2f}, NULL);
    gsk_Entity e_camera = *pCamera;

    gsk_Model *modelSponza =
      // gsk_model_load_from_file("../demo/demo_hot/Resources/models/AK.glb",
      // 1);
      gsk_model_load_from_file(GSK_PATH("data://models/sponza.glb"), 1, TRUE);

    gsk_Entity e_sponza = gsk_ecs_new(ecs);
    _gsk_ecs_add_internal(e_sponza,
                          C_TRANSFORM,
                          (void *)(&(struct ComponentTransform) {
                            .position = {0.0f, -1.5f, 0.0f},
                            .scale    = {0.001f, 0.001f, 0.001f},
                          }));
    _gsk_ecs_add_internal(
      e_sponza,
      C_MODEL,
      (void *)(&(struct ComponentModel) {
        .material = matWire,
        .pModel   = modelSponza,
        //.modelPath = "../demo/demo_hot/Resources/models/sponza.glb",
        //.modelPath  = "../res/models/test3.gltf",
        .properties = {
          .drawMode = DRAW_ELEMENTS,
          .cullMode = CULL_CW | CULL_FORWARD,
        }}));
    //_gsk_ecs_add_internal(characterEntity, C_ANIMATOR, NULL);
}

// physics test
static void
_scene6(gsk_ECS *ecs, gsk_Renderer *renderer)
{
    ecs = gsk_renderer_active_scene(renderer, 6);
    __set_active_scene_skybox(renderer, skyboxMain);

    gsk_Texture *texContDiff =
      texture_create_d(GSK_PATH("data://textures/container/diffuse.png"));
    gsk_Texture *texContSpec =
      texture_create_n(GSK_PATH("data://textures/container/specular.png"));

    gsk_Texture *texBrickDiff =
      texture_create_d(GSK_PATH("data://textures/brickwall/diffuse.png"));
    gsk_Texture *texBrickNorm =
      texture_create_n(GSK_PATH("data://textures/brickwall/normal.png"));

    gsk_Model *model_plane =
      gsk_model_load_from_file(GSK_PATH("gsk://models/plane.obj"), 1, FALSE);

    gsk_Model *model_sphere =
      gsk_model_load_from_file(GSK_PATH("gsk://models/sphere.obj"), 1, FALSE);

    gsk_Model *model_cube =
      gsk_model_load_from_file(GSK_PATH("gsk://models/cube.obj"), 1, FALSE);

    gsk_Material *matFloor =
      gsk_material_create(NULL,
                          GSK_PATH("gsk://shaders/lit-diffuse.shader"),
                          3,
                          texBrickDiff,
                          texBrickNorm,
                          texDefSpec);
    gsk_Material *matBox =
      gsk_material_create(NULL,
                          GSK_PATH("gsk://shaders/lit-diffuse.shader"),
                          3,
                          texContDiff,
                          texDefNorm,
                          texContSpec);

    gsk_Entity *pCamera = malloc(sizeof(gsk_Entity));

    *pCamera = __create_camera_entity(ecs, (vec3) {-2.0f, 5.0f, 8.0f}, NULL);
    gsk_Entity camera = *pCamera;

    // Testing entity on heap memory
    gsk_Entity *pFloorEntity = malloc(sizeof(gsk_Entity));
    *pFloorEntity            = gsk_ecs_new(ecs);
    gsk_Entity floorEntity   = *pFloorEntity;

#if 1
    _gsk_ecs_add_internal(floorEntity,
                          C_TRANSFORM,
                          (void *)(&(struct ComponentTransform) {
                            .position    = {0.0f, -1.0f, 0.0f},
                            .scale       = {10.0f, 1.0f, 10.0f},
                            .orientation = {0.0f, 0.0f, 0.0f},
                          }));
    _gsk_ecs_add_internal(floorEntity,
                          C_COLLIDER,
                          (void *)(&(struct ComponentCollider) {
                            .type = COLLIDER_PLANE,
                          }));
    _gsk_ecs_add_internal(
      floorEntity,
      C_MODEL,
      (void *)(&(struct ComponentModel) {.material   = matFloor,
                                         .pModel     = model_plane,
                                         .properties = {
                                           .drawMode = DRAW_ARRAYS,
                                           .cullMode = CULL_CW | CULL_FORWARD,
                                         }}));
#endif
#if 1

    gsk_Entity *pSphereEntity = malloc(sizeof(gsk_Entity));
    *pSphereEntity            = gsk_ecs_new(ecs);
    gsk_Entity sphereEntity   = *pSphereEntity;
    _gsk_ecs_add_internal(sphereEntity,
                          C_TRANSFORM,
                          (void *)(&(struct ComponentTransform) {
                            .position = {0.0f, 4.5f, -2.2f},
                          }));

    _gsk_ecs_add_internal(sphereEntity,
                          C_RIGIDBODY,
                          (void *)(&(struct ComponentRigidbody) {
                            .gravity = GRAVITY_EARTH,
                            .mass    = 20.0f,
                          }));

    _gsk_ecs_add_internal(sphereEntity,
                          C_COLLIDER,
                          (void *)(&(struct ComponentCollider) {
                            .type = COLLIDER_SPHERE,
                          }));

    _gsk_ecs_add_internal(
      sphereEntity,
      C_MODEL,
      (void *)(&(struct ComponentModel) {.material   = matBox,
                                         .pModel     = model_sphere,
                                         .properties = {
                                           .drawMode = DRAW_ARRAYS,
                                           .cullMode = CULL_CW | CULL_FORWARD,
                                         }}));
#endif

// Second sphere
#if 1

    gsk_Entity *pSphereEntity2 = malloc(sizeof(gsk_Entity));
    *pSphereEntity2            = gsk_ecs_new(ecs);
    gsk_Entity sphereEntity2   = *pSphereEntity2;
    _gsk_ecs_add_internal(sphereEntity2,
                          C_TRANSFORM,
                          (void *)(&(struct ComponentTransform) {
                            .position = {0.0f, 3.0f, -2.2f},
                          }));

    _gsk_ecs_add_internal(sphereEntity2,
                          C_RIGIDBODY,
                          (void *)(&(struct ComponentRigidbody) {
                            .gravity = GRAVITY_EARTH,
                            .mass    = 20.0f,
                          }));

    _gsk_ecs_add_internal(sphereEntity2,
                          C_COLLIDER,
                          (void *)(&(struct ComponentCollider) {
                            .type = COLLIDER_SPHERE,
                          }));

    _gsk_ecs_add_internal(
      sphereEntity2,
      C_MODEL,
      (void *)(&(struct ComponentModel) {.material   = matBox,
                                         .pModel     = model_sphere,
                                         .properties = {
                                           .drawMode = DRAW_ARRAYS,
                                           .cullMode = CULL_CW | CULL_FORWARD,
                                         }}));
#endif

// Cube
#if 1
    gsk_Entity *pCubeEntity = malloc(sizeof(gsk_Entity));
    *pCubeEntity            = gsk_ecs_new(ecs);
    gsk_Entity cubeEntity   = *pCubeEntity;
    _gsk_ecs_add_internal(cubeEntity,
                          C_TRANSFORM,
                          (void *)(&(struct ComponentTransform) {
                            .position = {0.0f, 0.1f, 0.0f},
                          }));

    _gsk_ecs_add_internal(cubeEntity,
                          C_RIGIDBODY,
                          (void *)(&(struct ComponentRigidbody) {
                            .gravity = GRAVITY_EARTH,
                            .mass    = 10.0f,
                          }));

    _gsk_ecs_add_internal(cubeEntity,
                          C_COLLIDER,
                          (void *)(&(struct ComponentCollider) {
                            .type = COLLIDER_BOX,
                          }));

    _gsk_ecs_add_internal(
      cubeEntity,
      C_MODEL,
      (void *)(&(struct ComponentModel) {.material   = matBox,
                                         .pModel     = model_cube,
                                         .properties = {
                                           .drawMode = DRAW_ARRAYS,
                                           .cullMode = CULL_CW | CULL_FORWARD,
                                         }}));

#endif
}

void
demo_scenes_create(gsk_ECS *ecs,
                   gsk_Renderer *renderer,
                   gsk_AssetCache *asset_cache)
{
#if 0
    // Default textures with options
    texDefSpec =
      texture_create_n(GSK_PATH("gsk://textures/defaults/black.png"));
    texDefNorm =
      texture_create_n(GSK_PATH("gsk://textures/defaults/normal.png"));
    texPbrAo = texture_create_n(GSK_PATH("gsk://textures/defaults/white.png"));
    texMissing =
      texture_create_n(GSK_PATH("gsk://textures/defaults/missing.jpg"));
#endif

    skyboxMain = gsk_skybox_hdr_create(
      texture_create_hdr(GSK_PATH("gsk://textures/hdr/sky_cloudy_ref.hdr")));

// NOTE: asset cache now handled by runtime
#if 0
    gsk_asset_cache_add_by_ext(asset_cache,
                               "data://textures/container/diffuse.png");
    gsk_asset_cache_add_by_ext(asset_cache,
                               "data://textures/container/specular.png");

    // test load material
    gsk_asset_cache_add_by_ext(asset_cache, "data://materials/cube.material");
#endif

#if LOAD_ALL_SCENES
    LOAD_SCENE(0);
    LOAD_SCENE(1);
    LOAD_SCENE(2);
    LOAD_SCENE(3);
    LOAD_SCENE(4);
    LOAD_SCENE(5);
    LOAD_SCENE(6);
    LOAD_SCENE(7);
#elif INITIAL_SCENE == SCENE_ASSET_TEST
    LOAD_SCENE_2(9);
#else
    LOAD_SCENE(INITIAL_SCENE);
#endif // LOAD_ALL_SCENES
}
