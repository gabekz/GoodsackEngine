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

    /* data for qmap */

    gsk_Model *qmap_model   = malloc(sizeof(gsk_Model));
    qmap_model->meshes      = malloc(sizeof(gsk_Mesh *) * 40000);
    qmap_model->meshesCount = 0;
    qmap_model->modelPath   = "NONE";

    /* loop */

    int cnt_poly = 0;
    for (int i = 0; i < p_container->list_entities.list_next; i++)
    {
        gsk_QMapEntity *ent =
          array_list_get_at_index(&p_container->list_entities, i);

        for (int j = 0; j < ent->list_brushes.list_next; j++)
        {
            gsk_QMapBrush *brush =
              array_list_get_at_index(&ent->list_brushes, j);

            for (int k = 0; k < brush->list_planes.list_next; k++)
            {
                qmap_model->meshesCount++; /* increment mesh count */

                gsk_QMapPolygon *poly =
                  array_list_get_at_index(&brush->list_polygons, k);
                gsk_QMapPlane *plane =
                  array_list_get_at_index(&brush->list_planes, k);

                // Allocate Mesh and immediately push to GPU
                qmap_model->meshes[cnt_poly] =
                  gsk_mesh_allocate((gsk_MeshData *)poly->p_mesh_data);
                gsk_mesh_assemble(qmap_model->meshes[cnt_poly]);

                mat4 localMatrix = GLM_MAT4_IDENTITY_INIT;
                glm_mat4_copy(localMatrix,
                              qmap_model->meshes[cnt_poly]->localMatrix);

                qmap_model->meshes[cnt_poly]->usingImportedMaterial = TRUE;
                qmap_model->meshes[cnt_poly]->materialImported = p_material_err;

                poly->p_texture = gsk_texture_set_get_by_name(
                  p_container->p_texture_set, plane->tex_name);

                //----------------------------------------------------------
                // create material for poly
                // TODO: Change this (we don't want duplicated materials)
                if (poly->p_texture != NULL)
                {
                    LOG_DEBUG("Successful loaded texture %s", plane->tex_name);

                    gsk_Material *material = gsk_material_create(
                      p_shader,
                      NULL,
                      3,
                      poly->p_texture,
                      gsk_texture_set_get_by_name(p_container->p_texture_set,
                                                  "NORM"),
                      gsk_texture_set_get_by_name(p_container->p_texture_set,
                                                  "SPEC"));

                    qmap_model->meshes[cnt_poly]->materialImported = material;
                } else
                {
                    LOG_WARN("Failed to find texture %s", plane->tex_name);
                }

                //----------------------------------------------------------

                cnt_poly++;
            }
        }
    }

    p_container->is_model_loaded = TRUE;
    p_container->p_model         = qmap_model;
    return qmap_model;
}