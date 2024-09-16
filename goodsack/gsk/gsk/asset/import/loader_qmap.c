/*
 * Copyright (c) 2024, Gabriel Kutuzov
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
/*   Build Functions                                                  */
/**********************************************************************/

gsk_QMapContainer
gsk_qmap_load(const char *map_path,
              gsk_TextureSet *p_texture_set,
              gsk_ShaderProgram *p_shader_program)
{
    // Parse map container
    gsk_QMapContainer ret = gsk_qmap_parse_map_file(map_path, p_texture_set);

    // build polygons for each brush
    for (int i = 0; i < ret.list_entities.list_next; i++)
    {
        gsk_QMapEntity *ent = array_list_get_at_index(&ret.list_entities, i);
        for (int j = 0; j < ent->list_brushes.list_next; j++)
        {
            gsk_QMapBrush *brush =
              array_list_get_at_index(&ent->list_brushes, j);

            gsk_qmap_build_polys_from_brush(&ret, brush);
        }
    }

    // build model
    gsk_qmap_load_model(&ret, p_shader_program);

    return ret;
}