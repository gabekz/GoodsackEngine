#include "demo_scenes.h"

#include "util/filesystem.h"

#include "entity/ecs.h"
#include "entity/modules/modules_systems.h"

#include "core/graphics/texture/texture_set.h"

#include "asset/asset.h"
#include "asset/asset_cache.h"
#include "asset/import/loader_qmap.h"

/*----------------------
 |  Scene 9
 |  Description: Asset Test
 -----------------------*/
void
_scene9(gsk_ECS *ecs, gsk_Renderer *renderer, gsk_AssetCache *p_asset_cache)
{
    gsk_Skybox *def_skybox;
    def_skybox = gsk_skybox_hdr_create(
      texture_create_hdr(GSK_PATH("gsk://textures/hdr/sky_cloudy_ref.hdr")));

    // textures
    gsk_Texture *def_norm = (gsk_Texture *)gsk_asset_get_str(
      p_asset_cache, "gsk://textures/defaults/normal.png");

    gsk_Texture *texContDiff =
      texture_create_d(GSK_PATH("data://textures/container/diffuse.png"));
    gsk_Texture *texContSpec =
      texture_create_d(GSK_PATH("data://textures/container/specular.png"));

    // shader
    const char *standard_shader_path =
      GSK_PATH("gsk://shaders/lit-diffuse.shader");
    gsk_Material *matBox = gsk_material_create(
      NULL, standard_shader_path, 3, texContDiff, def_norm, texContSpec);

    // model

    gsk_Model *model_cube =
      gsk_model_load_from_file(GSK_PATH("gsk://models/cube.obj"), 1, FALSE);

    /*----------------------
     |  New texture tests
     -----------------------*/

    /*----------------------
     |  ECS Setup
     -----------------------*/

    ecs = gsk_renderer_active_scene(renderer, 9);
    __set_active_scene_skybox(renderer, def_skybox);

    /*----------------------
     |  Entities
     -----------------------*/

    /*
      (Root) initialization (entity references)
    */

    // allocate camera entity to pass into Root as reference
    gsk_Entity *pCamera = malloc(sizeof(gsk_Entity));
    *pCamera            = gsk_ecs_new(ecs);

    /*
      Camera Entity
    */

    gsk_Entity camera = *pCamera;
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
                            .position = {0.0f, 0.0f, 0.0f},
                          }));

#if 1
    gsk_Entity *pCubeEntity = malloc(sizeof(gsk_Entity));
    *pCubeEntity            = gsk_ecs_new(ecs);
    gsk_Entity cubeEntity   = *pCubeEntity;
    _gsk_ecs_add_internal(cubeEntity,
                          C_TRANSFORM,
                          (void *)(&(struct ComponentTransform) {
                            .position = {0.0f, 0.1f, 0.0f},
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
};