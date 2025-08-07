#include "demo_scenes.h"

#include "util/filesystem.h"

#include "entity/ecs.h"
#include "entity/modules/modules_systems.h"

#include "asset/asset.h"

#include "physics/physics_types.h"

/*----------------------
 |  Scene 7
 |  Description: Main transform/weapon test
 -----------------------*/
void
_scene7(gsk_ECS *ecs, gsk_Renderer *renderer)
{
    gsk_Texture *def_spec, *def_norm, *def_ao, *def_missing;
    gsk_Skybox *def_skybox;

    // TODO: Move default textures
    // Default textures with options
    def_spec = texture_create_n(GSK_PATH("gsk://textures/defaults/black.png"));
    def_norm = texture_create_n(GSK_PATH("gsk://textures/defaults/normal.png"));
    def_ao   = texture_create_n(GSK_PATH("gsk://textures/defaults/white.png"));
    def_missing =
      texture_create_n(GSK_PATH("gsk://textures/defaults/missing_1.png"));

#if 0
    def_skybox = gsk_skybox_hdr_create(texture_create_hdr(
      GSK_PATH("gsk://textures/hdr/belfast_sunset_puresky_4k.hdr")));
#else
    def_skybox = gsk_skybox_hdr_create(
      texture_create_hdr(GSK_PATH("gsk://textures/hdr/sky_cloudy_ref.hdr")));
#endif

    /*----------------------
     |  ECS Setup
     -----------------------*/

    ecs = gsk_renderer_active_scene(renderer, 7);
    __set_active_scene_skybox(renderer, def_skybox);

    /*----------------------
     |  Resources
     -----------------------*/
    const char *standard_shader_path =
      GSK_PATH("gsk://shaders/lit-diffuse.shader");
    const char *pbr_shader_path = GSK_PATH("gsk://shaders/pbr.shader");

    gsk_Texture *tex_prototype =
      GSK_ASSET("gsk://textures/prototype/128_64.png");

    gsk_Material *matFloor = gsk_material_create(
      NULL, standard_shader_path, 3, tex_prototype, def_norm, def_spec);

    gsk_Model *modelPlane = GSK_ASSET("gsk://models/plane.obj");

    gsk_Texture *texCerbA =
      GSK_ASSET("data://textures/pbr/cerberus/Cerberus_A.tga");
    gsk_Texture *texCerbN =
      GSK_ASSET("data://textures/pbr/cerberus/Cerberus_N.tga");
    gsk_Texture *texCerbM =
      GSK_ASSET("data://textures/pbr/cerberus/Cerberus_M.tga");
    gsk_Texture *texCerbS =
      GSK_ASSET("data://textures/pbr/cerberus/Cerberus_R.tga");

    gsk_Material *matWeapon = gsk_material_create(
      NULL, pbr_shader_path, 5, texCerbA, texCerbN, texCerbM, texCerbS, def_ao);

    gsk_Model *modelWeapon = GSK_ASSET("data://models/AK2.glb");

    /*----------------------
     |  Entities
     -----------------------*/

    gsk_Entity *pFloorEntity = malloc(sizeof(gsk_Entity));
    *pFloorEntity            = gsk_ecs_new(ecs);
    gsk_Entity floorEntity   = *pFloorEntity;

    _gsk_ecs_add_internal(floorEntity,
                          C_TRANSFORM,
                          (void *)(&(struct ComponentTransform) {
                            .position = {0.0f, -0.3f, 0.0f},
                            .scale    = {1.0f, 1.0f, 1.0f},
                          }));
#if DEMO_USING_AUDIO
    _gsk_ecs_add_internal(floorEntity,
                          C_AUDIO_SOURCE,
                          (void *)(&(struct ComponentAudioSource) {
                            .filePath = "../res/audio/test.wav",
                            .looping  = 0,
                          }));
#endif // DEMO_USING_AUDIO
    _gsk_ecs_add_internal(floorEntity,
                          C_COLLIDER,
                          (void *)(&(struct ComponentCollider) {
                            .type = COLLIDER_PLANE,
                          }));
    _gsk_ecs_add_internal(floorEntity,
                          C_MODEL,
                          (void *)(&(struct ComponentModel) {
                            .material = matFloor,
                            .pModel   = modelPlane,
                          }));

    /*
      (Root) initialization (entity references)
    */

    // allocate camera entity to pass into Root as reference
    gsk_Entity *pCamera = malloc(sizeof(gsk_Entity));
    *pCamera            = gsk_ecs_new(ecs);

    // allocate p_ent_player
    gsk_Entity *p_ent_player = malloc(sizeof(gsk_Entity));
    *p_ent_player            = gsk_ecs_new(ecs);

#if DEMO_USING_AUDIO
    _gsk_ecs_add_internal(camera, C_AUDIO_LISTENER, NULL);
#endif // DEMO_USING_AUDIO

    /*
      Camera 2
    */
#if DEMO_USING_MULTIPLE_CAMERAS
    gsk_Entity camera2 = gsk_ecs_new(ecs);
    _gsk_ecs_add_internal(camera2,
                          C_CAMERA,
                          (void *)(&(struct ComponentCamera) {
                            .axisUp      = {0.0f, 1.0f, 0.0f},
                            .fov         = 45,
                            .renderLayer = 1,
                          }));
    _gsk_ecs_add_internal(camera2,
                          C_TRANSFORM,
                          (void *)(&(struct ComponentTransform) {
                            .position         = {0.0f, 0.2f, 0.0f},
                            .orientation      = {0.0f, 0.0f, 0.0f},
                            .scale            = {1.0f, 1.0f, 1.0f},
                            .parent_entity_id = pCamera->id,
                          }));
#endif

    /*
      (Root) Player Entity
    */

    gsk_Entity ent_player = *p_ent_player;
    _gsk_ecs_add_internal(ent_player,
                          C_TRANSFORM,
                          (void *)(&(struct ComponentTransform) {
                            .position = {0.0f, 0.0f, 0.0f},
                          }));
    _gsk_ecs_add_internal(ent_player,
                          C_COLLIDER,
                          (void *)(&(struct ComponentCollider) {
                            .type = COLLIDER_CAPSULE,
                          }));
    _gsk_ecs_add_internal(ent_player,
                          C_RIGIDBODY,
                          (void *)(&(struct ComponentRigidbody) {
                            .gravity = {0, -30.0f, 0},
                            .mass    = 20.0f,
                          }));
    _gsk_ecs_add_internal(ent_player,
                          C_PLAYER_CONTROLLER,
                          (void *)(&(struct ComponentPlayerController) {
                            .speed         = 10.0f,
                            .entity_camera = pCamera->id,
                          }));

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
                          C_CAMERA_LOOK,
                          (void *)(&(struct ComponentCameraLook) {
                            .sensitivity = 1.0f,
                          }));
    _gsk_ecs_add_internal(camera,
                          C_TRANSFORM,
                          (void *)(&(struct ComponentTransform) {
                            .position         = {0, 0, 0},
                            .parent_entity_id = p_ent_player->id,
                          }));

    /*
      Weapon Parent (weapon-sway)
    */
    gsk_Entity *pWeaponParent = malloc(sizeof(gsk_Entity));
    *pWeaponParent            = gsk_ecs_new(ecs);
    gsk_Entity weaponParent   = *pWeaponParent;

    _gsk_ecs_add_internal(weaponParent,
                          C_TRANSFORM,
                          (void *)(&(struct ComponentTransform) {
                            .position         = {0.0f, 0.0f, 0.0f},
                            .orientation      = {0.0f, 0.0f, 0.0f},
                            .scale            = {1.0f, 1.0f, 1.0f},
                            .parent_entity_id = pCamera->id,
                          }));
    _gsk_ecs_add_internal(weaponParent,
                          C_WEAPON_SWAY,
                          (void *)(&(struct ComponentWeaponSway) {
                            .sway_amount = 5,
                          }));

    /*
      Weapon Model
    */

    gsk_Entity attachedEntity = gsk_ecs_new(ecs);
    _gsk_ecs_add_internal(attachedEntity,
                          C_TRANSFORM,
                          (void *)(&(struct ComponentTransform) {
                            .position         = {-0.1f, -0.22f, -0.4340f},
                            .orientation      = {0.0f, 0.0f, -180.0f},
                            .scale            = {-0.02f, 0.02f, 0.02f},
                            .parent_entity_id = pWeaponParent->id,
                          }));

    _gsk_ecs_add_internal(attachedEntity,
                          C_MODEL,
                          (void *)(&(struct ComponentModel) {
                            .material  = matWeapon,
                            .pModel    = modelWeapon,
                            .modelPath = NULL,
                          }));

#if DEMO_USING_MULTIPLE_CAMERAS
    // Render layer (only render on camera with specified layer)
    _gsk_ecs_add_internal(attachedEntity,
                          C_RENDER_LAYER,
                          (void *)(&(struct ComponentRenderLayer) {
                            .renderLayer = 1,
                          }));
#endif

    _gsk_ecs_add_internal(attachedEntity,
                          C_WEAPON,
                          (void *)(&(struct ComponentWeapon) {
                            .damage        = 25,
                            .pos_starting  = {0, 0, 0},
                            .rot_starting  = {0, 0, 0},
                            .entity_camera = pCamera->id,
                          }));
};
