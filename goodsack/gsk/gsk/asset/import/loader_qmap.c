/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "loader_qmap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/logger.h"
#include "util/maths.h"
#include "util/sysdefs.h"

// Mode (ensure file is not currupted when reading)
#define QM_MODE_NONE     0
#define QM_MODE_FILL_ENT 1
#define QM_MODE_FILL_BSH 2

// Operation (allocation ops)
#define QM_OP_NONE 0
#define QM_OP_NEW  1
#define QM_OP_END  2

//-----------------------------------------------------------------------------
// Helper functions
//-----------------------------------------------------------------------------

static int
__next_mode(int mode, u8 add)
{
    if ((mode >= QM_MODE_FILL_BSH && add == TRUE) ||
        (mode <= QM_MODE_NONE && add == FALSE)) {
        LOG_CRITICAL("Corrupt file. %d ", mode);
    }

    return (add) ? mode + 1 : mode - 1;
}

#define MODE_UP(x)   x = __next_mode(x, TRUE);
#define MODE_DOWN(x) x = __next_mode(x, FALSE);

//-----------------------------------------------------------------------------
// Parsing functions
//-----------------------------------------------------------------------------

static void
__read_plane(char *line)
{
    LOG_DEBUG("Reading plane..");

    int cnt_char  = 0;                  // cnt reading character of the line
    int cnt_coord = 0, cnt_num = 0;     // cnt coords and numbers
    int start_index = 0, end_index = 0; // index for parenthesis-split

    // points
    vec3 points[3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vec3 normal    = {0, 0, 0};

    char *texture_name;
    f32 texture_properties[5] = {0, 0, 0, 0, 0};

    //
    // --- READ VERT COORDINATES
    //

    // Loop through line
    while (cnt_char != strlen(line)) {

        // start of coordinate
        if (line[cnt_char] == '(') {
            start_index = cnt_char;
            cnt_num     = 0;
        }

        // end of coordinate
        if (line[cnt_char] == ')') {
            end_index = cnt_char + 1;

            // get coordiante substring
            char substring[256];
            strncpy(substring, line + (start_index), (end_index - start_index));
            substring[end_index - start_index] = '\0';

            start_index = cnt_char;
            end_index   = cnt_char;

            // LOG_INFO("%s", substring);

            // separate coordinate by spaces
            char delim[] = " ";
            char *str    = substring;

            char *split = strtok(str, delim); // line, split by spaces
            split = strtok(NULL, delim); // split is now ignoring first value

            // read each number in the coordinate
            while (split != NULL && cnt_num <= 2) {
                float saved = atof(split);
                split       = strtok(NULL, delim);

                // LOG_INFO("saved: %f", saved);

                points[cnt_coord][cnt_num] = saved;

                cnt_num++;
            }
            cnt_num = 0; // reset number
            cnt_coord++;
        }

        // break out if we are at the final coordinate. Move on to the next
        // stage of extraction
        if (cnt_coord >= 3) { break; }

        cnt_char++;
    }

    //
    // --- PLANE NORMAL
    //
    {
        vec3 pq, pr;
        glm_vec3_sub(points[1], points[0], pq);
        glm_vec3_sub(points[2], points[0], pr);
        glm_vec3_cross(pq, pr, normal);
    }

    //
    // --- READ TEXTURE DATA
    //

    // grab substring from here on out - separate by whitespace
    char substring[256];
    strncpy(substring, line + (cnt_char + 1), (strlen(line) - (cnt_char + 1)));
    // LOG_INFO("Length: %d", strlen(line) - (cnt_char + 1));
    substring[strlen(line) - (cnt_char + 1)] = '\0';
    // LOG_INFO("rest of string:");
    // LOG_INFO("%s", substring);

    // separate coordinate by spaces
    char delim[] = " ";
    char *str    = substring;

    char *split = strtok(str, delim); // line, split by spaces

    int i = -1; // -1 start for the texture name. Following this are the texture
                // coordinate properties.

    while (split != NULL) {
        if (i == -1) {
            texture_name = strdup(split);
        } else {
            texture_properties[i] = atof(split);
        }
        split = strtok(NULL, delim);
        i++;
    }

#if 1 // LOG_PLANE
    for (int i = 0; i < 3; i++) {

        LOG_TRACE(
          "x: %f y: %f z: %f", points[i][0], points[i][1], points[i][2]);
    }

    LOG_TRACE("Normal is: (%f, %f, %f)", normal[0], normal[1], normal[2]);

    LOG_TRACE("TEXTURE NAME IS: %s", texture_name);
    for (int i = 0; i < 5; i++) {
        LOG_TRACE("TEXTURE prop: %f", texture_properties[i]);
    }
#endif
}

static void
__qmap_container_add_entity(gsk_QMapContainer *p_container)
{
    gsk_QMapEntity ent;
    ent.list_brushes = array_list_init(sizeof(gsk_QMapBrush), 1);
    ent.ent_index    = p_container->list_entities.list_next;

    // push to Container
    array_list_push(&p_container->list_entities, (void *)&ent);
    p_container->total_entities++; // increment total

    if ((int)p_container->total_entities !=
        (int)p_container->list_entities.list_next) {
        LOG_ERROR("Failed to allocate correct number of entities");
    }

    // set the current buffer view in Container to entity
    void *data = p_container->list_entities.data.buffer;
    p_container->p_cnt_entity =
      &((gsk_QMapEntity *)data)[p_container->list_entities.list_next - 1];
}

// Add a brush to the currently pointed-at entity
static void
__qmap_container_add_brush(gsk_QMapContainer *p_container)
{
    gsk_QMapBrush brush;

    // push Brush to Container
    array_list_push(&p_container->p_cnt_entity->list_brushes, (void *)&brush);
    p_container->total_brushes++; // increment total

    if ((int)p_container->total_brushes !=
        (int)p_container->p_cnt_entity->list_brushes.list_next) {
        LOG_ERROR("Failed to allocate correct number of brushes");
    }

    // set the current buffer view in Container to entity's Brush
    void *data = p_container->p_cnt_entity->list_brushes.data.buffer;
    p_container->p_cnt_brush =
      &((gsk_QMapBrush *)
          data)[p_container->p_cnt_entity->list_brushes.list_next - 1];
}

//-----------------------------------------------------------------------------
// Main Operation
//-----------------------------------------------------------------------------

gsk_QMapContainer
gsk_load_qmap(const char *map_path)
{
    // initialize QMapContainer
    gsk_QMapContainer ret;
    ret.total_entities = 0;
    ret.total_brushes  = 0;
    ret.total_planes   = 0;
    ret.list_entities  = array_list_init(sizeof(gsk_QMapEntity), 12);

    FILE *stream = NULL;
    char line[256]; // 256 = MAX line_length

    if ((stream = fopen(map_path, "rb")) == NULL) {
        LOG_CRITICAL("Error opening %s\n", map_path);
        exit(1);
    }

    int current_mode   = QM_MODE_NONE; // reading operation
    int next_operation = QM_OP_NONE;   // memory operation

    while (fgets(line, sizeof(line), stream)) {
        switch (line[0]) {
        case '{':
            MODE_UP(current_mode);
            next_operation = QM_OP_NEW;
            break;
        case '}':
            MODE_DOWN(current_mode);
            next_operation = QM_OP_END;
            break;
        case '(': __read_plane(line); break;
        default: break;
        }

        if (next_operation == QM_OP_NEW) {
            next_operation = QM_OP_NONE;

            if (current_mode == QM_MODE_FILL_ENT) {
                LOG_INFO("Creating new entity");
                __qmap_container_add_entity(&ret);
            } else if (current_mode == QM_MODE_FILL_BSH) {
                LOG_INFO("Creating new brush");
                __qmap_container_add_brush(&ret);
            }
        }
    }

    return ret;
}