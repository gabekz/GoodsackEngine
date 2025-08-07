/*
 * Copyright (c) 2024-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "qmap_model.h"

#include "core/graphics/material/material.h"
#include "core/graphics/mesh/model.h"
#include "core/graphics/shader/shader.h"
#include "core/graphics/texture/texture_set.h"

#include "qmap_util.h"

#include "util/array_list.h"
#include "util/maths.h"
#include "util/sysdefs.h"

#include <string.h>

#define MESH_PTR_LIST_INC 100
#define _MESH_BATCHING    TRUE

struct _BatchInfo
{
    gsk_Material *p_material;
    ArrayList list_meshdata_ptrs;
    u32 total_verts;
    u32 total_size;
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

    /* error batch */
    ArrayList list_batches      = LIST_INIT(sizeof(struct _BatchInfo), 1);
    struct _BatchInfo batch_err = {
      .p_material         = p_material_err,
      .total_verts        = 0,
      .total_size         = 0,
      .list_meshdata_ptrs = LIST_INIT(sizeof(gsk_MeshData *), 1),
    };
    LIST_PUSH(&list_batches, &batch_err);

    /* data for qmap */

    // list of mesh pointers
    ArrayList list_mesh_ptrs = LIST_INIT(sizeof(gsk_Mesh *), MESH_PTR_LIST_INC);

    /* loop */

    // int cnt_poly = 0;

    for (int i = 0; i < p_container->list_entities.list_next; i++)
    {
        gsk_QMapEntity *ent = LIST_GET(&p_container->list_entities, i);
        u8 no_render        = FALSE;

        // omit brush from export based on field
        gsk_QMapEntityField *field_class =
          gsk_qmap_util_get_field(ent, QMAP_NOEXPORT_FIELD_STR);
        if (field_class != NULL) { continue; }

        field_class = gsk_qmap_util_get_field(ent, "classname");

        if (field_class != NULL)
        {
            if (strstr(field_class->value, QMAP_TRIGGER_CONT_STR))
            {
                LOG_DEBUG("ent is trigger - skipping render");
                no_render = TRUE;
            }
        }

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

#if !(_MESH_BATCHING)
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
#endif // !(_MESH_BATCHING)

                //----------------------------------------------------------
                // skip rendering

                if (no_render == TRUE) { continue; }

                //----------------------------------------------------------
                // create material for poly

                poly->p_texture = gsk_texture_set_get_by_name(
                  p_container->p_texture_set, plane->tex_name);

                if (poly->p_texture == NULL)
                {
                    LOG_ERROR("Failed to find texture %s", plane->tex_name);

                    struct _BatchInfo *p_batch = LIST_GET(&list_batches, 0);

                    p_batch->total_verts += p_meshdata->vertexCount;
                    p_batch->total_size +=
                      p_meshdata->mesh_buffers[0].buffer_size,

                      LIST_PUSH(&p_batch->list_meshdata_ptrs, &p_meshdata);

                    continue;
                    // set to first batch (error batch)
                }

                // LOG_TRACE("Successful loaded texture %s", plane->tex_name);
                u8 is_new_material = TRUE;

#if _MESH_BATCHING
                for (int m = 0; m < list_batches.list_next; m++)
                {
                    // break out immediately to start making a new material
                    if (list_batches.is_list_empty) { break; }

                    struct _BatchInfo *p_batch = LIST_GET(&list_batches, m);

                    if (poly->p_texture == p_batch->p_material->textures[0])
                    {
                        is_new_material = FALSE;

                        // update batch

                        p_batch->total_verts += p_meshdata->vertexCount;
                        p_batch->total_size +=
                          p_meshdata->mesh_buffers[0].buffer_size,

                          LIST_PUSH(&p_batch->list_meshdata_ptrs, &p_meshdata);

                        break;
                    }
                }
#endif //_MESH_BATCHING

#if (!_MESH_BATCHING)
                // updating mesh property
                // TODO: CHANGE
                p_mesh->materialImported = p_material_err;
#endif // (!_MESH_BATCHING)
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

#if _MESH_BATCHING
                    // push new batch info
                    struct _BatchInfo batch = {
                      .p_material  = material,
                      .total_verts = p_meshdata->vertexCount,
                      .total_size  = p_meshdata->mesh_buffers[0].buffer_size,
                      .list_meshdata_ptrs =
                        LIST_INIT(sizeof(gsk_MeshData *), 1),
                    };

                    LIST_PUSH(&batch.list_meshdata_ptrs, &p_meshdata);
                    LIST_PUSH(&list_batches, &batch);

#else
                    p_mesh->materialImported = material;
#endif // _MESH_BATCHING
                }
            }
        }
    }

#if _MESH_BATCHING

    list_mesh_ptrs = LIST_INIT(sizeof(gsk_Mesh **), 1);

    for (int i = 0; i < list_batches.list_next; i++)
    {
        struct _BatchInfo *p_batch = LIST_GET(&list_batches, i);

        LOG_TRACE("Batch %d has %d meshes with %d verts (size %d)",
                  i,
                  p_batch->list_meshdata_ptrs.list_next,
                  p_batch->total_verts,
                  p_batch->total_size);

        // break out of this batch early if it is empty. This is only really
        // possible with the default material_err batch.
        if (p_batch->total_verts <= 0 || p_batch->total_size <= 0)
        {
            LOG_TRACE("Batch %d has no mesh data. Skipping.", i);
            continue;
        }

        u32 num_indices   = (p_batch->total_verts - 2) * 3;
        f32 *buff_verts   = malloc(p_batch->total_size);
        u32 *buff_indices = malloc(num_indices * sizeof(u32));

        u32 offset = 0;

        u32 n_origin = 0;
        u32 k        = 0;

        for (int j = 0; j < p_batch->list_meshdata_ptrs.list_next; j++)
        {
            gsk_MeshData **pp_meshdata =
              LIST_GET(&p_batch->list_meshdata_ptrs, j);
            gsk_MeshBuffer *p_meshbuff = &((*pp_meshdata)->mesh_buffers[0]);

            // copy over vertices
            memcpy((char *)buff_verts + offset,
                   p_meshbuff->p_buffer,
                   p_meshbuff->buffer_size);
            offset += p_meshbuff->buffer_size;

            for (int n = 1; n < (*pp_meshdata)->vertexCount - 1; n++)
            {
                buff_indices[k++] = n_origin;
                buff_indices[k++] = n_origin + n;
                buff_indices[k++] = n_origin + n + 1;
            }
            n_origin += (*pp_meshdata)->vertexCount;
        }

        // CREATE NEW MESHDATA
        gsk_MeshData *meshdata = malloc(sizeof(gsk_MeshData));
        meshdata->usage_draw   = GskOglUsageType_Static;

        meshdata->mesh_buffers_count = 0;

        meshdata->mesh_buffers_count++;
        meshdata->mesh_buffers[0] = (gsk_MeshBuffer) {
          .buffer_flags =
            (GskMeshBufferFlag_Positions | GskMeshBufferFlag_Textures |
             GskMeshBufferFlag_Normals | GskMeshBufferFlag_Tangents),
          .p_buffer    = buff_verts,
          .buffer_size = p_batch->total_size,
        };

        meshdata->mesh_buffers_count++;
        meshdata->mesh_buffers[1] = (gsk_MeshBuffer) {
          .buffer_flags = (GskMeshBufferFlag_Indices),
          .p_buffer     = buff_indices,
          .buffer_size  = num_indices * sizeof(u32),
        };

        meshdata->vertexCount    = p_batch->total_verts;
        meshdata->indicesCount   = num_indices;
        meshdata->primitive_type = GskMeshPrimitiveType_Triangle;

        // copy bounds
        glm_vec3_one(meshdata->boundingBox[0]);
        glm_vec3_one(meshdata->boundingBox[1]);

        // ASSEMBLE & ALLOCATE HERE

        gsk_Mesh *p_mesh = gsk_mesh_allocate(meshdata);

        LIST_PUSH(&list_mesh_ptrs, &p_mesh);

        // Assemble on the GPU
        gsk_mesh_assemble(p_mesh);

        // Setup p_mesh properties

        mat4 localMatrix = GLM_MAT4_IDENTITY_INIT;
        glm_mat4_copy(localMatrix, p_mesh->localMatrix);

        p_mesh->usingImportedMaterial = TRUE;
        p_mesh->materialImported      = p_batch->p_material;
    }
#endif // _MESH_BATCHING

#if !(_MESH_BATCHING)
    gsk_Model *qmap_model   = malloc(sizeof(gsk_Model));
    qmap_model->meshes      = list_mesh_ptrs.data.buffer;
    qmap_model->meshesCount = LIST_COUNT(&list_mesh_ptrs) + 1;
    qmap_model->modelPath   = "NONE";
    qmap_model->fileType    = QMAP;

    p_container->is_model_loaded = TRUE;
    p_container->p_model         = qmap_model;
#else
    gsk_Model *qmap_model = malloc(sizeof(gsk_Model));
    qmap_model->meshes = list_mesh_ptrs.data.buffer;
    qmap_model->meshesCount = LIST_COUNT(&list_mesh_ptrs) + 1;
    qmap_model->modelPath = "NONE";
    qmap_model->fileType = QMAP;

    p_container->is_model_loaded = TRUE;
    p_container->p_model = qmap_model;

#endif

    return qmap_model;
}