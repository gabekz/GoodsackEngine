#include "demo_scenes.h"

#include <entity/v1/ecs.h>

// #include <core/graphics/renderer/v1/renderer.h>

#include <entity/v1/builtin/component_test.h>
#include <entity/v1/builtin/components.h>

#define texture_create_d(x) texture_create(x, NULL, s_texOpsPbr)
#define texture_create_n(x) texture_create(x, NULL, s_texOpsNrm)

Texture *texDefSpec, *texDefNorm, *texPbrAo;
static TextureOptions s_texOpsPbr, s_texOpsNrm;

#define MY_THINGY(x) (void *)(&(__typeof(x)))

static Entity
__create_camera_entity(ECS *ecs, vec3 position)
{
    Entity camera = ecs_new(ecs);
    _ecs_add_internal(camera,
                      C_CAMERA,
                      (void *)(&(struct ComponentCamera) {
                        .axisUp = {0.0f, 1.0f, 0.0f},
                        .speed  = 2.5f,
                      }));
    _ecs_add_internal(camera,
                      C_TRANSFORM,
                      (void *)(&(struct ComponentTransform) {
                        .position = *(float *)position,
                      }));
    return camera;
}

static void
_scene0(ECS *ecs, Renderer *renderer)
{
    ecs = renderer_active_scene(renderer, 0);

    Texture *texEarthDiff =
      texture_create_d("../demo/demo_hot/Resources/textures/earth/diffuse.png");
    Texture *texEarthNorm =
      texture_create_n("../demo/demo_hot/Resources/textures/earth/normal.png");

    Material *matSuzanne = material_create(NULL,
                                           "../res/shaders/lit-diffuse.shader",
                                           3,
                                           texEarthDiff,
                                           texEarthNorm,
                                           texDefSpec);

    Entity camera = __create_camera_entity(ecs, (vec3) {0.0f, 0.0f, 2.0f});

    Entity suzanneObject = ecs_new(ecs);
    _ecs_add_internal(suzanneObject, C_TRANSFORM, NULL);
    _ecs_add_internal(suzanneObject,
                      C_MODEL,
                      (void *)(&(struct ComponentModel) {
                        .material   = matSuzanne,
                        .modelPath  = "../res/models/sphere.obj",
                        .properties = {
                          .drawMode = DRAW_ARRAYS,
                          .cullMode = CULL_CW | CULL_FORWARD,
                        }}));
}

static void
_scene1(ECS *ecs, Renderer *renderer)
{
    ecs = renderer_active_scene(renderer, 1);

    Texture *texContDiff = texture_create_d(
      "../demo/demo_hot/Resources/textures/container/diffuse.png");
    Texture *texContSpec = texture_create_n(
      "../demo/demo_hot/Resources/textures/container/specular.png");

    Texture *texBrickDiff = texture_create_d(
      "../demo/demo_hot/Resources/textures/brickwall/diffuse.png");
    Texture *texBrickNorm = texture_create_n(
      "../demo/demo_hot/Resources/textures/brickwall/normal.png");

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

    Entity camera = __create_camera_entity(ecs, (vec3) {0.0f, 0.0f, 2.0f});

    Entity floorEntity = ecs_new(ecs);
    _ecs_add_internal(floorEntity,
                      C_TRANSFORM,
                      (void *)(&(struct ComponentTransform) {
                        .position = {0.0f, -0.3f, 0.0f},
                        .scale    = {10.0f, 10.0f, 10.0f},
                      }));
    _ecs_add_internal(
      floorEntity,
      C_MODEL,
      (void *)(&(struct ComponentModel) {.material  = matFloor,
                                         .modelPath = "../res/models/plane.obj",
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
    _ecs_add_internal(
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
_scene2(ECS *ecs, Renderer *renderer)
{
    ecs = renderer_active_scene(renderer, 2);

    Texture *texGraniteAlbedo = texture_create_d(
      "../demo/demo_hot/Resources/textures/pbr/granite/albedo.png");
    Texture *texGraniteNormal = texture_create_n(
      "../demo/demo_hot/Resources/textures/pbr/granite/normal.png");
    Texture *texGraniteMetallic = texture_create_n(
      "../demo/demo_hot/Resources/textures/pbr/granite/metallic.png");
    Texture *texGraniteSpecular = texture_create_n(
      "../demo/demo_hot/Resources/textures/pbr/granite/roughness.png");
    Texture *texGraniteAo = texture_create_n(
      "../demo/demo_hot/Resources/textures/pbr/granite/ao.png");
    Material *matGranite = material_create(NULL,
                                           "../res/shaders/pbr.shader",
                                           5,
                                           texGraniteAlbedo,
                                           texGraniteNormal,
                                           texGraniteMetallic,
                                           texGraniteSpecular,
                                           texGraniteAo);

    Texture *texPbrAlbedo = texture_create_d(
      "../demo/demo_hot/Resources/textures/pbr/rust/albedo.png");
    Texture *texPbrNormal = texture_create_n(
      "../demo/demo_hot/Resources/textures/pbr/rust/normal.png");
    Texture *texPbrMetallic = texture_create_n(
      "../demo/demo_hot/Resources/textures/pbr/rust/metallic.png");
    Texture *texPbrSpecular = texture_create_n(
      "../demo/demo_hot/Resources/textures/pbr/rust/roughness.png");
    Material *matRust = material_create(NULL,
                                        "../res/shaders/pbr.shader",
                                        5,
                                        texPbrAlbedo,
                                        texPbrNormal,
                                        texPbrMetallic,
                                        texPbrSpecular,
                                        texPbrAo);

    Texture *texBrassAlbedo = texture_create_d(
      "../demo/demo_hot/Resources/textures/pbr/fancybrass/albedo.png");
    Texture *texBrassNormal = texture_create_n(
      "../demo/demo_hot/Resources/textures/pbr/fancybrass/normal.png");
    Texture *texBrassMetallic = texture_create_n(
      "../demo/demo_hot/Resources/textures/pbr/fancybrass/metallic.png");
    Texture *texBrassSpecular = texture_create_n(
      "../demo/demo_hot/Resources/textures/pbr/fancybrass/roughness.png");
    Texture *texBrassAo = texture_create_n(
      "../demo/demo_hot/Resources/textures/pbr/fancybrass/ao.png");
    Material *matBrass = material_create(NULL,
                                         "../res/shaders/pbr.shader",
                                         5,
                                         texBrassAlbedo,
                                         texBrassNormal,
                                         texBrassMetallic,
                                         texBrassSpecular,
                                         texBrassAo);

    Texture *texGoldAlbedo = texture_create_d(
      "../demo/demo_hot/Resources/textures/pbr/gold/albedo.png");
    Texture *texGoldNormal = texture_create_n(
      "../demo/demo_hot/Resources/textures/pbr/gold/normal.png");
    Texture *texGoldMetallic = texture_create_n(
      "../demo/demo_hot/Resources/textures/pbr/gold/metallic.png");
    Texture *texGoldSpecular = texture_create_n(
      "../demo/demo_hot/Resources/textures/pbr/gold/roughness.png");
    Material *matGold = material_create(NULL,
                                        "../res/shaders/pbr.shader",
                                        5,
                                        texGoldAlbedo,
                                        texGoldNormal,
                                        texGoldMetallic,
                                        texGoldSpecular,
                                        texPbrAo);

    Entity camera = __create_camera_entity(ecs, (vec3) {0.0f, 0.0f, 2.0f});

    Entity sphereEntity = ecs_new(ecs);
    _ecs_add_internal(sphereEntity,
                      C_TRANSFORM,
                      (void *)(&(struct ComponentTransform) {
                        .position = {0.0f, 0.0f, 0.0f},
                      }));
    _ecs_add_internal(sphereEntity,
                      C_MODEL,
                      (void *)(&(struct ComponentModel) {
                        .material   = matGranite,
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
                      C_MODEL,
                      (void *)(&(struct ComponentModel) {
                        .material   = matRust,
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
                      C_MODEL,
                      (void *)(&(struct ComponentModel) {
                        .material   = matBrass,
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
                      C_MODEL,
                      (void *)(&(struct ComponentModel) {
                        .material   = matGold,
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
_scene3(ECS *ecs, Renderer *renderer)
{

    ecs = renderer_active_scene(renderer, 3);

    Texture *texCerbA = texture_create_d(
      "../demo/demo_hot/Resources/textures/pbr/cerberus/Cerberus_A.tga");
    Texture *texCerbN = texture_create_n(
      "../demo/demo_hot/Resources/textures/pbr/cerberus/Cerberus_N.tga");
    Texture *texCerbM = texture_create_n(
      "../demo/demo_hot/Resources/textures/pbr/cerberus/Cerberus_M.tga");
    Texture *texCerbS = texture_create_n(
      "../demo/demo_hot/Resources/textures/pbr/cerberus/Cerberus_R.tga");

    Material *matCerb = material_create(NULL,
                                        "../res/shaders/pbr.shader",
                                        5,
                                        texCerbA,
                                        texCerbN,
                                        texCerbM,
                                        texCerbS,
                                        texPbrAo);

    Entity camera = __create_camera_entity(ecs, (vec3) {0.0f, 1.0f, 0.0f});
    _ecs_add_internal(camera,
                      C_TEST,
                      (void *)(&(struct ComponentTest) {
                        .rotation_speed = 11, .movement_increment = 5,
                        //.scale = {0.001f, 0.001f, 0.001f},
                      }));

#if 1
    Entity entCerb                              = ecs_new(ecs);
    struct ComponentTransform compCerbTransform = {
      .position = {0.0f, 0.0f, 0.0f},
      .scale    = {4.0f, 4.0f, 4.0f},
    };
    struct ComponentModel compCerbMesh = {
      .material   = matCerb,
      .modelPath  = "../demo/demo_hot/Resources/models/cerberus-triang.obj",
      .properties = {
        .drawMode = DRAW_ARRAYS,
        .cullMode = CULL_CW | CULL_FORWARD,
      }};
    _ecs_add_internal(
      entCerb,
      C_TRANSFORM,
      (void *)((struct ComponentTransform *)&compCerbTransform));

    _ecs_add_internal(
      entCerb, C_MODEL, (void *)((struct ComponentModel *)&compCerbMesh));
#endif
#if 0
    _ecs_add_internal(entCerb,
                      C_TEST,
                      (void *)(&(struct ComponentTest) {
                        .rotation_speed     = 50.0f,
                        .movement_increment = 10,
                      }));
#endif
}

static void
_scene4(ECS *ecs, Renderer *renderer)
{
    ecs = renderer_active_scene(renderer, 4);

    Material *matCharacter =
      material_create(NULL, "../res/shaders/skinning-test.shader", 0);

    Entity camera = __create_camera_entity(ecs, (vec3) {0.0f, 1.0f, 0.0f});

    Entity characterEntity = ecs_new(ecs);
    _ecs_add_internal(characterEntity,
                      C_TRANSFORM,
                      (void *)(&(struct ComponentTransform) {
                        .position = {0.0f, 0.0f, 0.0f},
                        //.scale = {0.001f, 0.001f, 0.001f},
                      }));
    _ecs_add_internal(
      characterEntity,
      C_MODEL,
      (void *)(&(struct ComponentModel) {
        .material  = matCharacter,
        .modelPath = "../demo/demo_hot/Resources/models/character-anim.gltf",
        //.modelPath  = "../demo/demo_hot/Resources/models/sponza.glb",
        //.modelPath  = "../res/models/test3.gltf",
        .properties = {
          .drawMode = DRAW_ELEMENTS,
          .cullMode = CULL_CW | CULL_FORWARD,
        }}));
    _ecs_add_internal(characterEntity, C_ANIMATOR, NULL);
}

static void
_scene5(ECS *ecs, Renderer *renderer)
{
    ecs = renderer_active_scene(renderer, 5);

    Material *matCharacter =
      material_create(NULL, "../res/shaders/skinning-test.shader", 0);

    Material *matWhite =
      material_create(NULL, "../res/shaders/wireframe.shader", 0);

    Entity e_camera = __create_camera_entity(ecs, (vec3) {-1.2f, 0.5f, 0.2f});

    Entity e_sponza = ecs_new(ecs);
    _ecs_add_internal(e_sponza,
                      C_TRANSFORM,
                      (void *)(&(struct ComponentTransform) {
                        .position = {0.0f, -1.5f, 0.0f},
                        .scale    = {0.001f, 0.001f, 0.001f},
                      }));
    _ecs_add_internal(
      e_sponza,
      C_MODEL,
      (void *)(&(struct ComponentModel) {
        .material = matWhite,
        //.modelPath  = "../demo/demo_hot/Resources/models/character-anim.gltf",
        .modelPath = "../demo/demo_hot/Resources/models/sponza.glb",
        //.modelPath  = "../res/models/test3.gltf",
        .properties = {
          .drawMode = DRAW_ELEMENTS,
          .cullMode = CULL_CW | CULL_FORWARD,
        }}));
    //_ecs_add_internal(characterEntity, C_ANIMATOR, NULL);
}

// physics test
static void
_scene6(ECS *ecs, Renderer *renderer)
{
    ecs = renderer_active_scene(renderer, 6);

    Texture *texContDiff = texture_create_d(
      "../demo/demo_hot/Resources/textures/container/diffuse.png");
    Texture *texContSpec = texture_create_n(
      "../demo/demo_hot/Resources/textures/container/specular.png");

    Texture *texBrickDiff = texture_create_d(
      "../demo/demo_hot/Resources/textures/brickwall/diffuse.png");
    Texture *texBrickNorm = texture_create_n(
      "../demo/demo_hot/Resources/textures/brickwall/normal.png");

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

    Entity *pCamera = malloc(sizeof(Entity));

    *pCamera      = __create_camera_entity(ecs, (vec3) {0.0f, 0.0f, 2.0f});
    Entity camera = *pCamera;

    // Testing entity on heap memory
    Entity *pFloorEntity = malloc(sizeof(Entity));
    *pFloorEntity        = ecs_new(ecs);
    Entity floorEntity   = *pFloorEntity;

    _ecs_add_internal(floorEntity,
                      C_TRANSFORM,
                      (void *)(&(struct ComponentTransform) {
                        .position = {0.0f, -0.3f, 0.0f},
                        .scale    = {10.0f, 10.0f, 10.0f},
                      }));
    _ecs_add_internal(floorEntity,
                      C_COLLIDER,
                      (void *)(&(struct ComponentCollider) {
                        .type = 2,
                      }));
    _ecs_add_internal(
      floorEntity,
      C_MODEL,
      (void *)(&(struct ComponentModel) {.material  = matFloor,
                                         .modelPath = "../res/models/plane.obj",
                                         .properties = {
                                           .drawMode = DRAW_ARRAYS,
                                           .cullMode = CULL_CW | CULL_FORWARD,
                                         }}));

    Entity *pSphereEntity = malloc(sizeof(Entity));
    *pSphereEntity        = ecs_new(ecs);
    Entity sphereEntity   = *pSphereEntity;
    _ecs_add_internal(sphereEntity,
                      C_TRANSFORM,
                      (void *)(&(struct ComponentTransform) {
                        //.position = {0.0f, -0.085f, -1.0f},
                        .position = {0.0f, 5.0f, -1.0f},
                      }));

    _ecs_add_internal(sphereEntity,
                      C_RIGIDBODY,
                      (void *)(&(struct ComponentRigidbody) {
                        .gravity = {0.0f, -0.981f, 0.0f},
                        .mass    = 1.0f,
                      }));

    _ecs_add_internal(sphereEntity,
                      C_COLLIDER,
                      (void *)(&(struct ComponentCollider) {
                        .type = 1,
                      }));

    _ecs_add_internal(sphereEntity,
                      C_MODEL,
                      (void *)(&(struct ComponentModel) {
                        .material   = matBox,
                        .modelPath  = "../res/models/sphere.obj",
                        .properties = {
                          .drawMode = DRAW_ARRAYS,
                          .cullMode = CULL_CW | CULL_FORWARD,
                        }}));
}

// Transform parenting test
static void
_scene7(ECS *ecs, Renderer *renderer)
{
    ecs = renderer_active_scene(renderer, 7);

    Texture *texContDiff = texture_create_d(
      "../demo/demo_hot/Resources/textures/container/diffuse.png");
    Texture *texContSpec = texture_create_n(
      "../demo/demo_hot/Resources/textures/container/specular.png");

    Texture *texBrickDiff = texture_create_d(
      "../demo/demo_hot/Resources/textures/brickwall/diffuse.png");
    Texture *texBrickNorm = texture_create_n(
      "../demo/demo_hot/Resources/textures/brickwall/normal.png");

    Material *matFloor = material_create(NULL,
                                         "../res/shaders/lit-diffuse.shader",
                                         3,
                                         texBrickDiff,
                                         texBrickNorm,
                                         texDefSpec);

    Entity *pCamera = malloc(sizeof(Entity));

    *pCamera      = __create_camera_entity(ecs, (vec3) {0.0f, 0.0f, 0.0f});
    Entity camera = *pCamera;

    // Testing entity on heap memory
    Entity *pFloorEntity = malloc(sizeof(Entity));
    *pFloorEntity        = ecs_new(ecs);
    Entity floorEntity   = *pFloorEntity;

    _ecs_add_internal(floorEntity,
                      C_TRANSFORM,
                      (void *)(&(struct ComponentTransform) {
                        .position = {0.0f, -0.3f, 0.0f},
                        .scale    = {10.0f, 10.0f, 10.0f},
                      }));
    _ecs_add_internal(floorEntity,
                      C_COLLIDER,
                      (void *)(&(struct ComponentCollider) {
                        .type = 2,
                      }));
    _ecs_add_internal(
      floorEntity,
      C_MODEL,
      (void *)(&(struct ComponentModel) {.material  = matFloor,
                                         .modelPath = "../res/models/plane.obj",
                                         .properties = {
                                           .drawMode = DRAW_ARRAYS,
                                           .cullMode = CULL_CW | CULL_FORWARD,
                                         }}));

    Texture *texCerbA = texture_create_d(
      "../demo/demo_hot/Resources/textures/pbr/cerberus/Cerberus_A.tga");
    Texture *texCerbN = texture_create_n(
      "../demo/demo_hot/Resources/textures/pbr/cerberus/Cerberus_N.tga");
    Texture *texCerbM = texture_create_n(
      "../demo/demo_hot/Resources/textures/pbr/cerberus/Cerberus_M.tga");
    Texture *texCerbS = texture_create_n(
      "../demo/demo_hot/Resources/textures/pbr/cerberus/Cerberus_R.tga");

    /*
    Material *matWeapon =
      material_create(NULL, "../res/shaders/pbr.shader", 1, texContDiff);
      */
    Material *matWeapon = material_create(NULL,
                                          "../res/shaders/pbr.shader",
                                          5,
                                          texCerbA,
                                          texCerbN,
                                          texCerbM,
                                          texCerbS,
                                          texPbrAo);

    Model *modelWeapon =
      //model_load_from_file("../demo/demo_hot/Resources/models/AK.glb", 1);
      model_load_from_file("../demo/demo_hot/Resources/models/AK.glb", 1);

    Entity attachedEntity = ecs_new(ecs);
    _ecs_add_internal(attachedEntity,
                      C_TRANSFORM,
                      (void *)(&(struct ComponentTransform) {
                        .position    = {0.1f, -0.22f, -0.4340f},
                        .orientation = {0.0f, 0.0f, -180.0f},
                        .scale       = {0.02f, 0.02f, 0.02f},
                        .parent      = pCamera,
                      }));

    _ecs_add_internal(
      attachedEntity,
      C_MODEL,
      (void *)(&(struct ComponentModel) {.material   = matWeapon,
                                         .pModel     = modelWeapon,
                                         .modelPath  = NULL,
                                         .properties = {
                                           .drawMode = DRAW_ELEMENTS,
                                           .cullMode = CULL_CW | CULL_FORWARD,
                                         }}));
    _ecs_add_internal(attachedEntity,
                      C_WEAPON,
                      (void *)(&(struct ComponentWeapon) {
                        .damage       = 25,
                        .pos_starting = {0, 0, 0},
                        .rot_starting = {0, 0, 0},
                      }));
};

#define GLUE_HELPER(x, y)   x##y
#define GLUE(x, y)          GLUE_HELPER(x, y)
#define LOAD_SCENE(__index) GLUE(_scene, __index)(ecs, renderer)

void
demo_scenes_create(ECS *ecs, Renderer *renderer)
{
    // Default textures with options
    s_texOpsNrm = (TextureOptions) {1, GL_RGB, false, true};
    s_texOpsPbr = (TextureOptions) {0, GL_SRGB_ALPHA, false, true};
    texDefSpec  = texture_create_n("../res/textures/defaults/black.png");
    texDefNorm  = texture_create_n("../res/textures/defaults/normal.png");
    texPbrAo    = texture_create_n("../res/textures/defaults/white.png");

#if LOAD_ALL_SCENES
    LOAD_SCENE(0);
    LOAD_SCENE(1);
    LOAD_SCENE(2);
    LOAD_SCENE(3);
    LOAD_SCENE(4);
    LOAD_SCENE(5);
    LOAD_SCENE(6);
    LOAD_SCENE(7);
#else
    LOAD_SCENE(INITIAL_SCENE);
#endif // LOAD_ALL_SCENES
}
