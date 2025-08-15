/*
 * Copyright (c) 2024-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "loader_qmap.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asset/qmap/qmap_build.h"
#include "asset/qmap/qmap_model.h"
#include "asset/qmap/qmap_parse.h"
#include "asset/qmap/qmap_util.h"
#include "asset/qmap/qmapdefs.h"

#include "core/graphics/mesh/model.h"
#include "core/graphics/shader/shader.h"
#include "core/graphics/texture/texture_set.h"

#include "util/filesystem.h"
#include "util/logger.h"
#include "util/maths.h"
#include "util/sysdefs.h"

#define _DEBUG_LOG_POINTS FALSE

/**********************************************************************/
/*   Layer Util Functions                                             */
/**********************************************************************/
static void
__create_qmap_layers(gsk_QMapContainer *p_container)
{
    for (int i = 0; i < p_container->list_entities.list_next; i++)
    {
        gsk_QMapEntity *p_ent = LIST_GET(&p_container->list_entities, i);

        gsk_QMapEntityField *origin_field =
          gsk_qmap_util_get_field(p_ent, "classname");

        if (origin_field == NULL) { continue; }
        if (strcmp(origin_field->value, "func_group")) { continue; }

        origin_field = gsk_qmap_util_get_field(p_ent, "_tb_type");
        if (origin_field == NULL) { continue; }

        if (!origin_field->value) { continue; }
        if (strcmp(origin_field->value, "_tb_layer")) { continue; }

        origin_field = gsk_qmap_util_get_field(p_ent, "_tb_id");

        u8 layer_exists = FALSE;
        u32 layer_id    = (u32)origin_field->members[0];

        // go through each layer. If this ID does not exist, add it
        for (int j = 0; j < p_container->list_layers.list_next; j++)
        {
            gsk_QMapLayer *p_layer = LIST_GET(&p_container->list_layers, j);
            if (p_layer->layer_id == layer_id)
            {
                layer_exists = TRUE;
                break;
            }
        }

        if (layer_exists == FALSE)
        {

            gsk_QMapLayer layer = {
              .layer_id   = layer_id,
              .is_visible = TRUE,
            };

            LOG_TRACE("adding layer %d to map", layer_id);
            LIST_PUSH(&p_container->list_layers, &layer);
        }
    }
}

static void
__add_entity_layers(gsk_QMapContainer *p_container)
{
    // set entity layers
    for (int i = 0; i < p_container->list_entities.list_next; i++)
    {
        gsk_QMapEntity *p_ent = LIST_GET(&p_container->list_entities, i);

        gsk_QMapEntityField *origin_field =
          gsk_qmap_util_get_field(p_ent, "classname");
        if (origin_field == NULL) { continue; }

        origin_field = gsk_qmap_util_get_field(p_ent, "_tb_id");
        if (origin_field == NULL)
        {
            origin_field = gsk_qmap_util_get_field(p_ent, "_tb_layer");
            if (origin_field == NULL) { continue; }
        }

        u32 layer_id = (u32)origin_field->members[0];

        // go through each layer. If this ID does not exist, add it
        for (int j = 0; j < p_container->list_layers.list_next; j++)
        {
            gsk_QMapLayer *p_layer = LIST_GET(&p_container->list_layers, j);
            if (p_layer->layer_id == layer_id)
            {
                LOG_TRACE(
                  "Setting entity %d to layer %d", p_ent->ent_index, layer_id);
                p_ent->layer_id = layer_id;
            }
        }
    }
}

/**********************************************************************/
/*   Build Functions                                                  */
/**********************************************************************/

gsk_QMapContainer
gsk_qmap_load(const char *map_path,
              gsk_TextureSet *p_texture_set,
              gsk_ShaderProgram *p_shader_program)
{
    // Parse map container
    gsk_QMapContainer ret = gsk_qmap_parse_map_file(map_path, p_texture_set);

    __create_qmap_layers(&ret);
    __add_entity_layers(&ret);

    // build polygons for each brush
    for (int i = 0; i < ret.list_entities.list_next; i++)
    {
        gsk_QMapEntity *ent = LIST_GET(&ret.list_entities, i);
        for (int j = 0; j < ent->list_brushes.list_next; j++)
        {
            gsk_QMapBrush *brush = LIST_GET(&ent->list_brushes, j);

            gsk_qmap_build_polys_from_brush(&ret, brush);
        }
    }

    // build models for each entity
    gsk_qmap_load_models(&ret, p_shader_program);

    return ret;
}