#include "demo_scenes.h"

#include "util/filesystem.h"

#include "entity/ecs.h"
#include "entity/modules/modules_systems.h"

#include "core/graphics/texture/texture_set.h"

#include "asset/import/loader_qmap.h"

/*----------------------
 |  Scene 8
 |  Description: Map Test
 -----------------------*/
void
_scene8(gsk_ECS *ecs, gsk_Renderer *renderer)
{
    gsk_Texture *def_spec, *def_norm, *def_ao, *def_missing;
    gsk_Skybox *def_skybox;

    // TODO: Move default textures
    // Default textures with options
    def_spec = texture_create_n(GSK_PATH("gsk://textures/defaults/black.png"));
    def_norm = texture_create_n(GSK_PATH("gsk://textures/defaults/normal.png"));
    def_ao   = texture_create_n(GSK_PATH("gsk://textures/defaults/white.png"));
    def_missing =
      texture_create_d(GSK_PATH("gsk://textures/defaults/missing.jpg"));

#if 0
    def_skybox = gsk_skybox_hdr_create(texture_create_hdr(
      GSK_PATH("gsk://textures/hdr/belfast_sunset_puresky_4k.hdr")));
#else
    def_skybox = gsk_skybox_hdr_create(
      texture_create_hdr(GSK_PATH("gsk://textures/hdr/sky_cloudy_ref.hdr")));
#endif

    const char *standard_shader_path =
      GSK_PATH("gsk://shaders/lit-diffuse.shader");
    const char *test_shader_path =
      GSK_PATH("gsk://shaders/unlit-textured.shader");
    const char *pbr_shader_path = GSK_PATH("gsk://shaders/pbr.shader");

    /*----------------------
     |  Create some textures
     -----------------------*/
    gsk_Texture *tex_uv_check =
      texture_create_d(GSK_PATH("gsk://textures/prototype/uv_checker.png"));

    gsk_Texture *tex_prototype =
      texture_create_d(GSK_PATH("gsk://textures/prototype/128_64.png"));

    gsk_Texture *tex_uv_test_top =
      texture_create_d(GSK_PATH("gsk://textures/prototype/uv_test_top.png"));

    /*----------------------
     |  Create a texture set (from the textures above)
     -----------------------*/
    gsk_TextureSet texture_set = gsk_texture_set_init();

    gsk_texture_set_add(&texture_set, def_missing, "MISSING");

    gsk_texture_set_add(&texture_set, def_spec, "SPEC");
    gsk_texture_set_add(&texture_set, def_spec, "textures/defaults/black");

    gsk_texture_set_add(&texture_set, def_norm, "NORM");
    gsk_texture_set_add(&texture_set, def_norm, "textures/defaults/normal");

    gsk_texture_set_add(&texture_set, tex_prototype, "__TB_empty");

    gsk_texture_set_add(
      &texture_set, tex_uv_check, "textures/prototype/uv_checker");

    gsk_texture_set_add(
      &texture_set, tex_uv_test_top, "textures/prototype/uv_test_top");

    gsk_texture_set_add(
      &texture_set, tex_prototype, "textures/prototype/128_64");

    gsk_texture_set_add(&texture_set, tex_uv_check, "terrain/terrain");

    gsk_Texture *lookup_texture =
      gsk_texture_set_get_by_name(&texture_set, "NORM");

    /*----------------------
     |  Default MISSING Material
     -----------------------*/
    gsk_Material *mat_missing = gsk_material_create(
      NULL, test_shader_path, 3, def_missing, def_norm, def_spec);

    /*----------------------
     |  Import QMap
     -----------------------*/
    gsk_ShaderProgram *p_shader_qmap =
      gsk_shader_program_create(GSK_PATH("gsk://shaders/lit-diffuse.shader"));

    gsk_QMapContainer qmap = gsk_qmap_load(
      GSK_PATH("gsk://map/cube_valve.map"), &texture_set, p_shader_qmap);

    gsk_Model *qmap_model = qmap.p_model;
    /*----------------------
     |  ECS Setup
     -----------------------*/

    ecs = gsk_renderer_active_scene(renderer, 8);
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

    /*
      Map Entity
    */

    gsk_Entity ent_map = gsk_ecs_new(ecs);
    _gsk_ecs_add_internal(ent_map,
                          C_TRANSFORM,
                          (void *)(&(struct ComponentTransform) {
                            .position = {0, 0, 0},
                          }));
    _gsk_ecs_add_internal(ent_map,
                          C_MODEL,
                          (void *)(&(struct ComponentModel) {
                            .material  = mat_missing,
                            .pModel    = qmap_model,
                            .modelPath = NULL,
                          }));
};
