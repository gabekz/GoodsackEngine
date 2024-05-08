#include "demo_scenes.h"

#include "util/filesystem.h"

#include "entity/ecs.h"
#include "entity/modules/modules_systems.h"

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
      texture_create_n(GSK_PATH("gsk://textures/defaults/missing.jpg"));

#if 0
    def_skybox = gsk_skybox_hdr_create(texture_create_hdr(
      GSK_PATH("gsk://textures/hdr/belfast_sunset_puresky_4k.hdr")));
#else
    def_skybox = gsk_skybox_hdr_create(
      texture_create_hdr(GSK_PATH("gsk://textures/hdr/sky_cloudy_ref.hdr")));
#endif

    /*----------------------
     |  Import QMap
     -----------------------*/
    gsk_QMapContainer qmap = gsk_load_qmap(GSK_PATH("gsk://map/octagon.map"));

    gsk_Model *qmap_model   = malloc(sizeof(gsk_Model));
    qmap_model->meshes      = malloc(sizeof(gsk_Mesh *) * 40000);
    qmap_model->meshesCount = 0;
    qmap_model->modelPath   = "none";

    int cnt_poly = 0;
    for (int i = 0; i < qmap.list_entities.list_next; i++)
    {
        gsk_QMapEntity *ent = array_list_get_at_index(&qmap.list_entities, i);

        for (int j = 0; j < ent->list_brushes.list_next; j++)
        {
            gsk_QMapBrush *brush =
              array_list_get_at_index(&ent->list_brushes, j);

            for (int k = 0; k < brush->list_planes.list_next; k++)
            {
                gsk_QMapPolygon *poly =
                  array_list_get_at_index(&brush->list_polygons, k);

                qmap_model->meshesCount++;

                qmap_model->meshes[cnt_poly] =
                  gsk_mesh_assemble((gsk_MeshData *)poly->p_mesh_data);
                qmap_model->meshes[cnt_poly]->usingImportedMaterial = FALSE;

                mat4 localMatrix = GLM_MAT4_IDENTITY_INIT;
                glm_mat4_copy(localMatrix,
                              qmap_model->meshes[cnt_poly]->localMatrix);

                cnt_poly++;
            }
        }
    }

#if 0
    qmap_model->meshes[0] = gsk_mesh_assemble(qmap.mesh_data);
    qmap_model->meshes[0]->usingImportedMaterial = FALSE;
    qmap_model->meshesCount                      = 1;
    qmap_model->modelPath                        = "none";

    mat4 localMatrix = GLM_MAT4_IDENTITY_INIT;
    glm_mat4_copy(localMatrix, qmap_model->meshes[0]->localMatrix);
#endif

    /*----------------------
     |  ECS Setup
     -----------------------*/

    ecs = gsk_renderer_active_scene(renderer, 8);
    //__set_active_scene_skybox(renderer, def_skybox);

    /*----------------------
     |  Resources
     -----------------------*/
    const char *standard_shader_path =
      GSK_PATH("gsk://shaders/lit-diffuse.shader");
    const char *test_shader_path =
      GSK_PATH("gsk://shaders/unlit-textured.shader");
    const char *pbr_shader_path = GSK_PATH("gsk://shaders/pbr.shader");

    gsk_Texture *tex_prototype =
      texture_create_d(GSK_PATH("gsk://textures/prototype/128_64.png"));

    gsk_Material *mat_missing = gsk_material_create(
      NULL, test_shader_path, 3, def_missing, def_norm, def_spec);

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
                            .position = {0, 0, 0},
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
