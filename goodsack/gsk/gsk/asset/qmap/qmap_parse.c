/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "qmap_parse.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asset/qmap/qmap_util.h"
#include "asset/qmap/qmapdefs.h"
#include "core/graphics/texture/texture_set.h"

#include "util/array_list.h"
#include "util/maths.h"
#include "util/sysdefs.h"

// Mode (ensure file is not currupted when reading)
#define QM_MODE_NONE     0
#define QM_MODE_FILL_ENT 1
#define QM_MODE_FILL_BSH 2

// Operation (allocation ops)
#define QM_OP_NONE       0
#define QM_OP_NEW_GROUP  1 // entities, brushes
#define QM_OP_NEW_MEMBER 2 // entity fields, planes
#define QM_OP_END        3

#define _FIX_POINT_FACING TRUE
#define _FIX_UV_FACING    TRUE

// helpers
#define MODE_UP(x)   x = __next_mode(x, TRUE);
#define MODE_DOWN(x) x = __next_mode(x, FALSE);

/**********************************************************************/
/*   Helper Functions                                                 */
/**********************************************************************/

/*--------------------------------------------------------------------*/
static int
__next_mode(int mode, u8 add)
{
    if ((mode >= QM_MODE_FILL_BSH && add == TRUE) ||
        (mode <= QM_MODE_NONE && add == FALSE))
    {
        LOG_CRITICAL("Corrupt file. %d ", mode);
    }

    return (add) ? mode + 1 : mode - 1;
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
static void
__qmap_container_add_entity(gsk_QMapContainer *p_container)
{
    gsk_QMapEntity ent;
    ent.list_brushes = array_list_init(sizeof(gsk_QMapBrush), 1);
    ent.list_fields  = array_list_init(sizeof(gsk_QMapEntityField), 1);
    ent.ent_index    = p_container->total_entities;

    // push to Container
    array_list_push(&p_container->list_entities, (void *)&ent);
    p_container->total_entities++; // increment total

    if ((int)p_container->total_entities !=
        (int)p_container->list_entities.list_next)
    {
        LOG_ERROR("Failed to allocate correct number of entities");
    }

    // set the current buffer view in Container to entity
    void *data = p_container->list_entities.data.buffer;
    p_container->p_cnt_entity =
      &((gsk_QMapEntity *)data)[p_container->list_entities.list_next - 1];
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
// Add a brush to the currently pointed-at entity
static void
__qmap_container_add_brush(gsk_QMapContainer *p_container)
{
    gsk_QMapBrush brush;
    brush.list_planes = array_list_init(sizeof(gsk_QMapPlane), 3);
    brush.brush_index = p_container->total_brushes;

    // push Brush to Container
    array_list_push(&p_container->p_cnt_entity->list_brushes, (void *)&brush);
    p_container->total_brushes++; // increment total

    if ((int)p_container->total_brushes !=
        (int)p_container->p_cnt_entity->list_brushes.list_next)
    {
        LOG_ERROR("Failed to allocate correct number of brushes");
    }

    // set the current buffer view in Container to entity's Brush
    void *data = p_container->p_cnt_entity->list_brushes.data.buffer;
    p_container->p_cnt_brush =
      &((gsk_QMapBrush *)
          data)[p_container->p_cnt_entity->list_brushes.list_next - 1];
}
/*--------------------------------------------------------------------*/

/**********************************************************************/
/*   Parsing Functions                                                */
/**********************************************************************/

/*--------------------------------------------------------------------*/
gsk_QMapPlane
gsk_qmap_parse_plane_from_line(char *line, gsk_TextureSet *p_texture_set)
{
    LOG_DEBUG("Parsing plane..");

    gsk_QMapPlane ret;

    int cnt_char  = 0;                  // cnt reading character of the line
    int cnt_coord = 0, cnt_num = 0;     // cnt coords and numbers
    int start_index = 0, end_index = 0; // index for parenthesis-split

    // points
    vec3 points[3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vec3 normal    = {0, 0, 0};
    f32 determinant;

    char *texture_name;
    f32 texture_properties[5]    = {0, 0, 0, 0, 0};
    int total_texture_properties = 0;
    vec3 u_axis = {0, 0, 0}, v_axis = {0, 0, 0};
    u8 is_uv_axes_found = FALSE;

    const f32 tex_default_size = QMAP_DEFAULT_TEXTURE_SIZE;
    vec2 tex_dimensions        = {tex_default_size, tex_default_size};

    /*==== Read vert coordinates =====================================*/

    // Loop through line
    while (cnt_char != strlen(line))
    {

        // start of coordinate
        if (line[cnt_char] == '(')
        {
            start_index = cnt_char;
            cnt_num     = 0;
        }

        // end of coordinate
        if (line[cnt_char] == ')')
        {
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
            while (split != NULL && cnt_num <= 2)
            {
                float saved = atof(split);
                split       = strtok(NULL, delim);

                // LOG_INFO("saved: %f", saved);

                // TODO: Scale is broken? Need epsilon somewhere
                points[cnt_coord][cnt_num] = saved * QMAP_IMPORT_SCALE;

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

    /*==== Flip y-z to match left-handed coordinate system ===========*/
#if _FIX_POINT_FACING
    for (int i = 0; i < 3; i++)
    {
        f32 saved    = points[i][2];
        points[i][2] = points[i][1];
        points[i][1] = saved;

        points[i][0] = -points[i][0];
        points[i][1] = -points[i][1];
    }
#endif /* _FIX_POINT_FACING */

    /*==== Plane normal and determinant ==============================*/
    {
        // normal
        vec3 pq, pr;
        glm_vec3_sub(points[1], points[0], pq);
        glm_vec3_sub(points[2], points[0], pr);
        glm_vec3_crossn(pq, pr, normal);

#if _FIX_POINT_FACING
        glm_vec3_negate(normal);
#endif /* _FIX_POINT_FACING */

        // determinant
        determinant = (normal[0] * points[0][0] + normal[1] * points[0][1] +
                       normal[2] * points[0][2]);

        if (determinant != glm_vec3_dot(normal, points[0]))
        {
            LOG_ERROR("Failed calculation");
        }
    }

    /*==== Read texture data =========================================*/

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

    u8 _collect_uv        = FALSE; // collect [n, n, n, _]
    u8 _collect_uv_offset = FALSE; // collect [_, _, _, n]
    int uv_idx            = 0;
    int uv_idx_offset     = 0;
    // f32 uvax[6]       = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

    vec3 uvs[2]    = {{0, 0, 0}, {0, 0, 0}};
    f32 offsets[2] = {0.0f, 0.0f};

    int uv_ax_idx = -1; /* [n][?] */
    int uv_ax_num = -1; /* [?][n] */

    while (split != NULL)
    {
        // determine if we are reading the uv-axis

        if (i == -1) /* begin texture name */
        {
            texture_name = strdup(split);

        } else if (split[0] == '[') /* begin check UV's */
        {
            is_uv_axes_found   = TRUE;
            _collect_uv        = TRUE;
            _collect_uv_offset = FALSE;

            uv_ax_idx++;
            uv_ax_num = 0;

        } else if (split[0] == ']')
        {
            /* stop collecting uv from bracket pack */
            _collect_uv = FALSE;

            i = 1; // TODO: Rename variable
            // 2 is the index at which the offsets are not included

        } else /* begin grabbing data */
        {
            if (_collect_uv == TRUE && _collect_uv_offset == FALSE)
            {
                uvs[uv_ax_idx][uv_ax_num] = atof(split);
                uv_ax_num++;

                if (uv_ax_num >= 3)
                {
                    // uv_ax_idx++;
                    uv_ax_num          = 0;
                    _collect_uv_offset = TRUE;
                }

            } else if (_collect_uv_offset == TRUE)
            {
                // grab offset
                texture_properties[uv_ax_idx] = atof(split);
                _collect_uv_offset            = FALSE;

            } else // don't collect UV
            {
                texture_properties[i] = atof(split);
                total_texture_properties++;
            }
        }

        // move to next delim
        split = strtok(NULL, delim);
        i++;
    }

    /*==== Calculate uv axes =========================================*/

    if (is_uv_axes_found == FALSE) /* format 0 (standard) */
    {
        vec3 axisref = {0.0f, 0.0f, 1.0f}; // z-axis reference
        f32 check    = glm_vec3_dot(normal, axisref);

        if (fabs(check) > 0.999f)
        {
            axisref[0] = 1.0f; // use x-axis if normal is close to z-axis
            axisref[2] = 0.0f;
        }

        glm_vec3_crossn(normal, axisref, u_axis);
        glm_vec3_crossn(normal, u_axis, v_axis);

    } else /* format 1 (valve) */
    {
        glm_vec3_copy(uvs[0], u_axis);
        glm_vec3_copy(uvs[1], v_axis);
    }

    /*---- Scale uv axes ---------------------------------------------*/
    glm_vec3_divs(u_axis, QMAP_IMPORT_SCALE, u_axis);
    glm_vec3_divs(v_axis, QMAP_IMPORT_SCALE, v_axis);

    /*---- Fix UV facing ---------------------------------------------*/
#if _FIX_UV_FACING
    {
        /* TODO: Seems to be broken for standard format */

        /* U - flip y-z axis */
        f32 saved = u_axis[2];
        u_axis[2] = u_axis[1];
        u_axis[1] = saved;

        /* U - negate Z */
        u_axis[2] = -u_axis[2];

        /* V - flip y-z axis */
        saved     = v_axis[2];
        v_axis[2] = v_axis[1];
        v_axis[1] = saved;

        /* V - negate X, negate Y */
        v_axis[0] = -v_axis[0];
        v_axis[1] = -v_axis[1];
    }
#endif /* _FIX_UV_FACING */

    /*==== Calculate texture width/height ============================*/
    gsk_Texture *p_tex =
      (gsk_Texture *)gsk_texture_set_get_by_name(p_texture_set, texture_name);

    if (p_tex != NULL)
    {
        tex_dimensions[0] = p_tex->width;
        tex_dimensions[1] = p_tex->height;
    }

    /*==== Logging (debugging) =======================================*/

#if _DEBUG_LOG_POINTS
    for (int i = 0; i < 3; i++)
    {

        LOG_TRACE(
          "x: %f y: %f z: %f", points[i][0], points[i][1], points[i][2]);
    }

    LOG_TRACE("Normal is: (%f, %f, %f)", normal[0], normal[1], normal[2]);

    LOG_TRACE("TEXTURE NAME IS: %s", texture_name);
    for (int i = 0; i < 5; i++)
    {
        LOG_TRACE("TEXTURE prop: %f", texture_properties[i]);
    }
#endif /* _DEBUG_LOG_POINTS */

    /*==== Return ====================================================*/

    /*---- Copy points -----------------------------------------------*/
    glm_vec3_copy(points[0], ret.points[0]);
    glm_vec3_copy(points[1], ret.points[1]);
    glm_vec3_copy(points[2], ret.points[2]);

    /*---- Copy uv axes ----------------------------------------------*/
    glm_vec3_copy(u_axis, ret.uv_axes[0]);
    glm_vec3_copy(v_axis, ret.uv_axes[1]);

    /*---- Copy normal and determinant -------------------------------*/
    glm_vec3_copy(normal, ret.normal);
    ret.determinant = determinant;

    /*---- check for minimums ----------------------------------------*/
    for (int i = 0; i < 5; i++)
    {
        if (fabs(texture_properties[i]) < 0.5f)
        {
            texture_properties[i] = 0.0f;
        }
    }

    /*---- Copy texture data -----------------------------------------*/
    ret.tex_offset[0] = texture_properties[0];
    ret.tex_offset[1] = texture_properties[1];

    ret.tex_rotation = texture_properties[2];

    ret.tex_scale[0] = texture_properties[3];
    ret.tex_scale[1] = texture_properties[4];
    strcpy(ret.tex_name, texture_name);

    glm_vec2_copy(tex_dimensions, ret.tex_dimensions);

    /*---- Return QMapPlane ------------------------------------------*/
    return ret;
}

/*--------------------------------------------------------------------*/
gsk_QMapEntityField
gsk_qmap_parse_field_from_line(char *line)
{

    gsk_QMapEntityField ret;

    int cnt_char    = 0;                // cnt reading character of the line
    int start_index = 0, end_index = 0; // index for parenthesis-split

    char key_str[256] = "";
    char val_str[256] = "";

    u8 key_found = FALSE;
    u8 val_found = FALSE;

    {
        char delim[] = "\"";
        char *str    = line;

        char *split = strtok(str, delim); // line, split by '"'
        int iter    = 0;

        while (split != NULL)
        {
            u8 is_valid_token = FALSE;

            if (strlen(split) > 1 && iter <= 2) { is_valid_token = TRUE; }

            // map - key
            if (is_valid_token && key_found == FALSE)
            {
                strcpy(key_str, split);
                LOG_TRACE("Key is %s", key_str);
                key_found = TRUE;

            }
            // map - value
            else if (is_valid_token && val_found == FALSE)
            {
                strcpy(val_str, split);
                LOG_TRACE("Value is %s", val_str);
                val_found = TRUE;
            }

            // move to next delim AND increase iteration
            split = strtok(NULL, delim);
            iter++;
        }
    }

    // check the value type

    s32 total_values = 0;
    s32 value_type   = 0; // 0 - num | 1 - str

    f32 val_numbers[QMAP_MAX_FIELD_MEMBERS] = {0}; // buffer for numeric data

    {
        char delim[] = " ";
        char *str    = val_str;

        char *split = strtok(str, delim); // line, split by ' '

        while (split != NULL)
        {
            for (int i = 0; i < strlen(split); i++)
            {
                if (isalpha(split[i]))
                {
                    value_type = 1;
                    break;
                }
            }

            // start filling numbers
            // TODO: Error handling
            if (value_type == 0 && total_values <= QMAP_MAX_FIELD_MEMBERS)
            {
                val_numbers[total_values] = atof(split);
            }

            // LOG_INFO("\tspaced out: %s", split);
            split = strtok(NULL, delim);
            total_values++;
        }
    }
    LOG_DEBUG("Total values in key-value pair: %d. Type is: %d",
              total_values,
              value_type);

    // copy data
    ret.field_type = value_type;
    strcpy(ret.key, key_str);

    if (value_type == 1)
    {
        strcpy(ret.value, val_str);
    } else
    {
        for (int i = 0; i < QMAP_MAX_FIELD_MEMBERS; i++)
        {
            ret.members[i] = val_numbers[i];
        }
    }

    return ret;
}
/*--------------------------------------------------------------------*/

/**********************************************************************/
/*   Main Operation                                                   */
/**********************************************************************/
/*--------------------------------------------------------------------*/
gsk_QMapContainer
gsk_qmap_parse_map_file(const char *map_path, gsk_TextureSet *p_textureset)
{
    // initialize QMapContainer
    gsk_QMapContainer ret;
    ret.total_entities = 0;
    ret.total_brushes  = 0;
    ret.total_planes   = 0;
    ret.list_entities  = array_list_init(sizeof(gsk_QMapEntity), 12);

    ret.is_map_compiled = FALSE;
    ret.is_model_loaded = FALSE;

    // attach textureset
    ret.p_texture_set = p_textureset;

    FILE *stream = NULL;
    char line[256]; // 256 = MAX line_length

    if ((stream = fopen(map_path, "rb")) == NULL)
    {
        LOG_CRITICAL("Error opening %s\n", map_path);
        exit(1);
    }

    int current_mode   = QM_MODE_NONE; // reading operation
    int next_operation = QM_OP_NONE;   // memory operation

    while (fgets(line, sizeof(line), stream))
    {
        switch (line[0])
        {
        case '{':
            MODE_UP(current_mode);
            next_operation = QM_OP_NEW_GROUP;
            break;
        case '}':
            MODE_DOWN(current_mode);
            next_operation = QM_OP_END;
            break;
        case '(': next_operation = QM_OP_NEW_MEMBER; break;
        case '"': next_operation = QM_OP_NEW_MEMBER; break;
        default: break;
        }

        // Groups (entities, brushes)
        if (next_operation == QM_OP_NEW_GROUP)
        {
            next_operation = QM_OP_NONE;

            if (current_mode == QM_MODE_FILL_ENT)
            {
                LOG_DEBUG("Creating new entity");
                __qmap_container_add_entity(&ret);
            } else if (current_mode == QM_MODE_FILL_BSH)
            {
                LOG_DEBUG("Creating new brush");
                __qmap_container_add_brush(&ret);
            }
        }
        // Members/fields (entity field, plane)
        else if (next_operation == QM_OP_NEW_MEMBER)
        {
            next_operation = QM_OP_NONE;
            // entity field
            if (current_mode == QM_MODE_FILL_ENT)
            {
                LOG_DEBUG("Fill entity member");
                gsk_QMapEntityField field =
                  gsk_qmap_parse_field_from_line(line);
                array_list_push(&ret.p_cnt_entity->list_fields, &field);
            }
            // plane
            else if (current_mode == QM_MODE_FILL_BSH)
            {
                gsk_QMapPlane plane =
                  gsk_qmap_parse_plane_from_line(line, p_textureset);
                array_list_push(&ret.p_cnt_brush->list_planes, &plane);
            }
        }
    }

    // close stream - tick compiled check
    fclose(stream);
    ret.is_map_compiled = TRUE;

    return ret;
}
/*--------------------------------------------------------------------*/