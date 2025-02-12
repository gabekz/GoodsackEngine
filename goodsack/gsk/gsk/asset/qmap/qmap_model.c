/*
 * Copyright (c) 2024-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "qmap_model.h"

#include "core/graphics/material/material.h"
#include "core/graphics/mesh/model.h"
#include "core/graphics/shader/shader.h"
#include "core/graphics/texture/texture_set.h"

#include "util/array_list.h"
#include "util/maths.h"
#include "util/sysdefs.h"

#define MESH_PTR_LIST_INC 100

struct _BatchInfo
{
    gsk_Material *p_material;
    ArrayList list_meshdata_ptrs;
    u32 total_verts;
};

gsk_Model *
gsk_qmap_load_model(gsk_QMapContainer *p_container, gsk_ShaderProgram *p_shader)
{
    if (p_container->is_map_compiled == FALSE)
    {
        LOG_CRITICAL("Cannot load Model-data for an uncompiled map.");
        return NULL;
    }

    if (p_container->is_model_loaded == TRUE)
    {
        LOG_ERROR("Attempting to load Model-data for a map while it is already "
                  "loaded.");
        return NULL;
    }

    /* default material */
    gsk_Material *p_material_err = gsk_material_create(
      p_shader,
      NULL,
      3,
      gsk_texture_set_get_by_name(p_container->p_texture_set, "MISSING"),
      gsk_texture_set_get_by_name(p_container->p_texture_set, "NORM"),
      gsk_texture_set_get_by_name(p_container->p_texture_set, "SPEC"));

    /* batch group info */
    ArrayList list_batches = LIST_INIT(sizeof(struct _BatchInfo), 1);

    /* data for qmap */

    // list of mesh pointers
    ArrayList list_mesh_ptrs = LIST_INIT(sizeof(gsk_Mesh *), MESH_PTR_LIST_INC);

    /* loop */

    // int cnt_poly = 0;

    for (int i = 0; i < p_container->list_entities.list_next; i++)
    {
        gsk_QMapEntity *ent = LIST_GET(&p_container->list_entities, i);

        // omit brush from export based on field
        gsk_QMapEntityField *field_class =
          gsk_qmap_util_get_field(ent, QMAP_NOEXPORT_FIELD_STR);
        if (field_class != NULL) { continue; }

        for (int j = 0; j < ent->list_brushes.list_next; j++)
        {
            gsk_QMapBrush *brush = LIST_GET(&ent->list_brushes, j);

            // setup brush-specific bounds
            vec3 minBounds = {10000, 10000, 10000};
            vec3 maxBounds = {-10000, -10000, -10000};

            for (int k = 0; k < brush->list_planes.list_next; k++)
            {
                // Get poly and plane
                gsk_QMapPolygon *poly = LIST_GET(&brush->list_polygons, k);
                gsk_QMapPlane *plane  = LIST_GET(&brush->list_planes, k);

                gsk_MeshData *p_meshdata = (gsk_MeshData *)poly->p_mesh_data;

                //----------------------------------------------------------
                // update brush bounds
                {

                    if (p_meshdata->boundingBox[0][0] < minBounds[0])
                        minBounds[0] = p_meshdata->boundingBox[0][0];
                    if (p_meshdata->boundingBox[1][0] > maxBounds[0])
                        maxBounds[0] = p_meshdata->boundingBox[1][0];

                    if (p_meshdata->boundingBox[0][1] < minBounds[1])
                        minBounds[1] = p_meshdata->boundingBox[0][1];
                    if (p_meshdata->boundingBox[1][1] > maxBounds[1])
                        maxBounds[1] = p_meshdata->boundingBox[1][1];

                    if (p_meshdata->boundingBox[0][2] < minBounds[2])
                        minBounds[2] = p_meshdata->boundingBox[0][2];
                    if (p_meshdata->boundingBox[1][2] > maxBounds[2])
                        maxBounds[2] = p_meshdata->boundingBox[1][2];

#if 0
            // calculate local-space bounds with aabb-center
            glm_vec3_copy(minBounds, brush->brush_bounds[0]);
            glm_vec3_copy(maxBounds, brush->brush_bounds[1]);

            glm_aabb_center(brush->brush_bounds, brush->world_pos);

            glm_vec3_sub(minBounds, brush->world_pos, brush->brush_bounds[0]);
            glm_vec3_sub(maxBounds, brush->world_pos, brush->brush_bounds[1]);
#else
                    // copy world-space bounds
                    glm_vec3_copy(minBounds, brush->brush_bounds[0]);
                    glm_vec3_copy(maxBounds, brush->brush_bounds[1]);
#endif
                }

                //----------------------------------------------------------
                // create mesh for each plane

                gsk_Mesh *p_mesh =
                  gsk_mesh_allocate((gsk_MeshData *)poly->p_mesh_data);

                LIST_PUSH(&list_mesh_ptrs, &p_mesh);

                // Assemble on the GPU
                gsk_mesh_assemble(p_mesh);

                // Setup p_mesh properties

                mat4 localMatrix = GLM_MAT4_IDENTITY_INIT;
                glm_mat4_copy(localMatrix, p_mesh->localMatrix);

                p_mesh->usingImportedMaterial = TRUE;
                p_mesh->materialImported      = p_material_err;

                //----------------------------------------------------------
                // create material for poly
                // TODO: Change this (we don't want duplicated materials)

                poly->p_texture = gsk_texture_set_get_by_name(
                  p_container->p_texture_set, plane->tex_name);

                if (poly->p_texture == NULL)
                {
                    LOG_ERROR("Failed to find texture %s", plane->tex_name);
                    // set to first batch (error batch)
                }

                // LOG_TRACE("Successful loaded texture %s", plane->tex_name);

                u8 is_new_material = TRUE;
                for (int m = 0; m < list_batches.list_next; m++)
                {
                    // break out immediately to start making a new material
                    if (list_batches.is_list_empty) { break; }

                    struct _BatchInfo *p_batch = LIST_GET(&list_batches, m);

                    if (poly->p_texture == p_batch->p_material->textures[0])
                    {
                        is_new_material = FALSE;

                        // update batch here
                        p_batch->total_verts += p_meshdata->vertexCount;
                        LIST_PUSH(&p_batch->list_meshdata_ptrs, &p_meshdata);

                        break;
                    }
                }

                // updating mesh property
                // TODO: CHANGE
                p_mesh->materialImported = p_material_err;

                if (is_new_material == TRUE)
                {
                    gsk_Material *material = gsk_material_create(
                      p_shader,
                      NULL,
                      3,
                      poly->p_texture,
                      gsk_texture_set_get_by_name(p_container->p_texture_set,
                                                  "NORM"),
                      gsk_texture_set_get_by_name(p_container->p_texture_set,
                                                  "SPEC"));

                    // create meshdata ptr list for upcoming batch

                    // push new batch info
                    struct _BatchInfo batch = {
                      .p_material  = material,
                      .total_verts = p_meshdata->vertexCount,
                      .list_meshdata_ptrs =
                        LIST_INIT(sizeof(gsk_MeshData *), 1),
                    };

                    LIST_PUSH(&list_batches, &batch);
                    LIST_PUSH(&batch.list_meshdata_ptrs, &p_meshdata);
                    p_mesh->materialImported = material;
                }

                //----------------------------------------------------------

                // cnt_poly++;
            }
        }
    }

    for (int i = 0; i < list_batches.list_next; i++)
    {
        struct _BatchInfo *p_batch = LIST_GET(&list_batches, i);
        LOG_INFO(
          "BATCH %d has %d meshes", i, p_batch->list_meshdata_ptrs.list_next);
    }

    gsk_Model *qmap_model   = malloc(sizeof(gsk_Model));
    qmap_model->meshes      = list_mesh_ptrs.data.buffer;
    qmap_model->meshesCount = LIST_COUNT(&list_mesh_ptrs) + 1;
    qmap_model->modelPath   = "NONE";
    qmap_model->fileType    = QMAP;

    p_container->is_model_loaded = TRUE;
    p_container->p_model         = qmap_model;
    return qmap_model;
}