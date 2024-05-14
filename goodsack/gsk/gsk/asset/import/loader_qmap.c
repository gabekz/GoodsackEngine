/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "loader_qmap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/graphics/material/material.h"
#include "core/graphics/mesh/mesh.h"
#include "core/graphics/mesh/model.h"
#include "core/graphics/shader/shader.h"
#include "core/graphics/texture/texture_set.h"

#include "util/filesystem.h"
#include "util/logger.h"
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

#define QM_ALLOC_ITER 1 // default number for realloc'ing bloks

// Options
#define POLY_PER_FACE TRUE // generate a polygon for each face
#define IMPORT_SCALE  0.02f
//#define IMPORT_SCALE 1.0f

// Stuff that breaks
#define _NORMALIZE_UV  FALSE
#define _USE_CENTER_UV FALSE
#define _CALCULATE_TBN TRUE

#define _FIX_POINT_FACING TRUE
#define _FIX_UV_FACING    TRUE

#define DEFAULT_TEXTURE_SIZE 512.0f

#define _DEBUG_LOG_POINTS FALSE

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
static u8
__get_intersection(
  f32 *n1, f32 *n2, f32 *n3, f32 d1, f32 d2, f32 d3, f32 *output)
{
    vec3 cross1, cross2, cross3;
    glm_vec3_cross(n2, n3, cross1);
    glm_vec3_cross(n3, n1, cross2);
    glm_vec3_cross(n1, n2, cross3);

    f32 denom = glm_vec3_dot(n1, cross1);

    if (denom >= -0.1f && denom <= 0.1f)
    {
        LOG_TRACE("NO intersection: denom %f", denom);
        return FALSE; // No intersection, the planes are parallel or
        // coincident
    }

    vec3 term1, term2, term3;
    glm_vec3_scale(cross1, -d1, term1);
    glm_vec3_scale(cross2, -d2, term2);
    glm_vec3_scale(cross3, -d3, term3);

    vec3 sum, result;
    glm_vec3_add(term1, term2, sum);
    glm_vec3_add(sum, term3, result);

    glm_vec3_divs(result, denom, output);

    LOG_TRACE("HAS intersection: denom %f", denom);
    return TRUE;
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
static u8
__compare_vertices(const f32 *v1, const f32 *v2, const f32 epsilon)
{
    if (fabs(v1[0] - v2[0]) < epsilon && fabs(v1[1] - v2[1]) < epsilon &&
        fabs(v1[2] - v2[2]) < epsilon)
    {
        return 1; // Vertices are the same
    }
    return 0; // Vertices are different
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
static void
__calculate_uv_coords(vec3 vertex,
                      vec3 u_axis,
                      vec3 v_axis,
                      f32 s,
                      f32 t,
                      f32 scale_x,
                      f32 scale_y,
                      f32 tex_width,
                      f32 tex_height,
                      f32 *output)
{
    // TODO: ensure uv-axes are normalized

    output[0] =
      ((glm_vec3_dot(vertex, u_axis) / tex_width) / scale_x) + (s / tex_width);

    output[1] = ((glm_vec3_dot(vertex, v_axis) / tex_height) / scale_y) +
                (t / tex_height);
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
static void
__calculate_plane_from_points(
  vec3 p1, vec3 p2, vec3 p3, f32 *norm_out, f32 *deter_out)
{
    // normal
    vec3 pq, pr;
    glm_vec3_sub(p2, p1, pq);
    glm_vec3_sub(p3, p1, pr);
    glm_vec3_crossn(pq, pr, norm_out);

    // determinant
    *deter_out =
      (norm_out[0] * p1[0] + norm_out[1] * p1[1] + norm_out[2] * p1[2]);
}
/*--------------------------------------------------------------------*/

#define BACK     0
#define FRONT    1
#define ON_PLANE 3

/*--------------------------------------------------------------------*/
static s32
__classify_point(vec3 point, vec3 plane_norm, f32 plane_deter)
{
    f32 result = glm_vec3_dot(plane_norm, point) - plane_deter;

    if (result > 0.0f)
    {
        return FRONT;
    } else if (result < 0.0f)
    {
        return BACK;
    }

    return ON_PLANE;
}
/*--------------------------------------------------------------------*/

/**********************************************************************/
/*   Parsing Functions                                                */
/**********************************************************************/

/*--------------------------------------------------------------------*/
static gsk_QMapPlane
__parse_plane_from_line(char *line, gsk_TextureSet *p_texture_set)
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

    const f32 tex_default_size = DEFAULT_TEXTURE_SIZE;
    vec2 tex_dimensions        = {tex_default_size, tex_default_size};

    /*---- determine map type from line -------------------------------------*/
    for (int i = 0; i < strlen(line); i++)
    {
        if (line[i] == '[') { LOG_INFO("MAP IS VALVE"); }
    }

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
                points[cnt_coord][cnt_num] = saved * IMPORT_SCALE;

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
    glm_vec3_divs(u_axis, IMPORT_SCALE, u_axis);
    glm_vec3_divs(v_axis, IMPORT_SCALE, v_axis);

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

/**********************************************************************/
/*   Build Functions                                                  */
/**********************************************************************/

/*--------------------------------------------------------------------*/
static void
__qmap_polygons_from_brush(gsk_QMapContainer *p_container,
                           gsk_QMapBrush *p_brush)
{
    // f32 vL           = 0;
    // f32 vtL          = 0;
    s32 planes_count = p_brush->list_planes.list_next;
    void *data       = p_brush->list_planes.data.buffer;

    gsk_QMapPlane *p_planes   = data;
    p_container->total_planes = planes_count;

#if POLY_PER_FACE
    // Create polygon for each face

    p_brush->list_polygons = array_list_init(
      sizeof(gsk_QMapPolygon), planes_count); // iterate at planes_count

    for (int i = 0; i < planes_count; i++)
    {
        gsk_QMapPolygon poly = {0};
        poly.list_vertices =
          array_list_init(sizeof(gsk_QMapPolygonVertex), QM_ALLOC_ITER);

        // push this polygon to the list on the brush
        array_list_push(&p_brush->list_polygons, &poly);
    }
#endif

    // ----------------------------------------------
    // Polygon generation from intersecting planes
    // ----------------------------------------------

    LOG_DEBUG("Assembling polygon from brush (brush_index: %d)",
              p_brush->brush_index);

    u32 iterations = 0; // for debugging purposes
    for (int i = 0; i < planes_count; i++)
    {
        LOG_INFO("Checking intersections for poly %d", i);

        for (int j = 0; j < planes_count; j++)
        {
            for (int k = 0; k < planes_count; k++)
            {
                iterations++;

                // do not check same
                if (i == j || i == k || j == k) { continue; }

                vec3 vertex; // filled by __get_intersection()
                u8 is_illegal   = FALSE;
                u8 is_duplicate = FALSE;
                u8 is_intersect = __get_intersection(p_planes[i].normal,
                                                     p_planes[j].normal,
                                                     p_planes[k].normal,
                                                     p_planes[i].determinant,
                                                     p_planes[j].determinant,
                                                     p_planes[k].determinant,
                                                     vertex);

                // continue without intersection
                if (is_intersect == FALSE) { continue; }

                // check for illegal point
                for (int m = 0; m < planes_count; m++)
                {
                    f32 term1  = glm_vec3_dot(p_planes[m].normal, vertex);
                    f32 check1 = term1 + p_planes[m].determinant;
                    if (check1 > 0.1f)
                    {
                        is_illegal = TRUE;
                        LOG_TRACE(
                          "Illegal point - vertex: %d (term: %f)", m, check1);
                        break;
                    }
                }

                if (is_illegal == TRUE) { continue; }

                // increment vertexList track | TODO: Remove
                // vL += 3;
                // vtL += 2;
#if POLY_PER_FACE
                gsk_QMapPolygon *poly =
                  array_list_get_at_index(&p_brush->list_polygons, i);

                vec3 p0 = {0, 0, 0};

                // Get p0
                if (poly->list_vertices.is_list_empty == TRUE)
                {
                    glm_vec3_copy(vertex, p0);
                } else
                {
                    gsk_QMapPolygonVertex *first =
                      array_list_get_at_index(&poly->list_vertices, 0);
                    glm_vec3_copy(first->position, p0);
                }

                // polygon vertex
                gsk_QMapPolygonVertex vert;
                glm_vec3_copy(vertex, vert.position);

                glm_vec3_copy(p_planes[i].normal, vert.normal);
                glm_vec3_zero(vert.tangent);

                // calculate UV coords
                __calculate_uv_coords(vertex,
                                      p_planes[i].uv_axes[0],
                                      p_planes[i].uv_axes[1],
                                      p_planes[i].tex_offset[0],
                                      p_planes[i].tex_offset[1],
                                      p_planes[i].tex_scale[0],
                                      p_planes[i].tex_scale[1],
                                      p_planes[i].tex_dimensions[0],
                                      p_planes[i].tex_dimensions[1],
                                      vert.texture);

                // glm_vec3_copy(p_planes[i].normal, vert.normal);

                // check for duplicates
                for (int n = 0; n < poly->list_vertices.list_next; n++)
                {
                    gsk_QMapPolygonVertex *compare =
                      array_list_get_at_index(&poly->list_vertices, n);

                    if (__compare_vertices(
                          vert.position, compare->position, 0.001f))
                    {
                        is_duplicate = TRUE;
                        LOG_TRACE("vertex is duplicate..");
                    }
                }

                if (is_duplicate == TRUE) { continue; }

                // array_list_push(&poly->list_vertices, &vertex);
                array_list_push(&poly->list_vertices, &vert);

#if 0
                array_list_push(&p_container->vertices,
                                &vert); // TODO: Remove once we construct the
                                        // meshdata from polygons
#endif

#else
                array_list_push(&p_container->vertices, &vertex);
#endif // POLY_PER_FACE

#if 1
                LOG_TRACE(
                  "Vertex at (%f, %f, %f)", vertex[0], vertex[1], vertex[2]);
#endif
            }
        }
    }

    LOG_DEBUG("Assembled polygon with %d iterations", iterations);

    // ----------------------------------------------
    // Sort polygon vertices
    // ----------------------------------------------

    int num_poly = p_brush->list_polygons.list_next;

    for (int i = 0; i < num_poly; i++)
    {
        LOG_DEBUG("Sorting polygon %d", i);

        // if (i != 2) continue;

        // get polygon center
        vec3 center = {1.1f, 1.0f, 0.9f};
        glm_vec3_scale(center, IMPORT_SCALE, center);
        // NOTE: Perfect squares won't work unless we offset the center by a
        // small amount

        gsk_QMapPolygon *poly =
          array_list_get_at_index(&p_brush->list_polygons, i);

        int num_vert = poly->list_vertices.list_next;

        // calculate center
        for (int j = 0; j < num_vert; j++)
        {

            gsk_QMapPolygonVertex *vert =
              array_list_get_at_index(&poly->list_vertices, j);

            glm_vec3_add(center, vert->position, center);
        }

        // glm_vec3_divs(center, num_vert, center);
        // glm_vec3_divs(center, num_vert + 1, center);
        glm_vec3_divs(center, num_vert, center);
        glm_vec3_copy(center, poly->center);

        // start sorting

        for (int n = 0; n < num_vert - 2; n++)
        {
            // get current vert
            gsk_QMapPolygonVertex *vert =
              array_list_get_at_index(&poly->list_vertices, n);

            // get A and P
            vec3 a = {0, 0, 0};

            // perpendicular plane
            vec3 plane_norm = {0, 0, 0};
            f32 plane_deter = 0.0f;

            // angle check
            f64 smallest_angle = -1;
            s32 smallest       = -1;

            // calculate A (perpendicular)
            {
                glm_vec3_sub(vert->position, center, a);
                // glm_vec3_sub(center, vert->position, a);
                glm_vec3_normalize(a);
            }

            // calculate plane from points
            {
                vec3 p1 = {0}, p2 = {0}, p3 = {0};
                vec3 normal = {0};

                glm_vec3_copy(p_planes[i].normal, normal);

                glm_vec3_copy(vert->position, p1); /* p1 = vertices[n]     */
                glm_vec3_copy(center, p2);         /* p2 = center          */
                glm_vec3_add(center, normal, p3);  /* p3 = center + normal */

                __calculate_plane_from_points(
                  p1, p2, p3, plane_norm, &plane_deter);

                // glm_vec3_normalize(plane_norm);
            }

            for (int m = n + 1; m < num_vert; m++)
            {
                // if (m == n + 1) { smallest = m + 1; }

                gsk_QMapPolygonVertex *vert_m =
                  array_list_get_at_index(&poly->list_vertices, m);

                s32 facing =
                  __classify_point(vert_m->position, plane_norm, plane_deter);

                if (facing != BACK)
                {
                    vec3 b    = {0, 0, 0};
                    f32 angle = 0;

                    // calculate B
                    glm_vec3_sub(vert_m->position, center, b);
                    // glm_vec3_sub(center, vert_m->position, b);
                    glm_vec3_normalize(b);

                    // calculate angle
                    angle = glm_vec3_dot(a, b);

                    if (angle > smallest_angle)
                    {
                        smallest_angle = angle;
                        smallest       = m;
                    }
                } else
                {
                    LOG_TRACE("Poly %d, vert %d, m %d facing is: %s",
                              i,
                              n,
                              m,
                              (facing == BACK)
                                ? "BACK"
                                : (facing == FRONT ? "FRONT" : "ON_PLANE"));
                }
            }

#if 1
            if (smallest <= -1)
            {
                LOG_ERROR(
                  "Failed to find closest vertex for: poly %d, vert %d", i, n);
                continue;
            }
#endif

            gsk_QMapPolygonVertex *vert2 =
              array_list_get_at_index(&poly->list_vertices, n + 1);

            gsk_QMapPolygonVertex *vert3 =
              array_list_get_at_index(&poly->list_vertices, smallest);

            gsk_QMapPolygonVertex swap_copy = *vert2;
            *vert2                          = *vert3;
            *vert3                          = swap_copy;
        }
    }
// ----------------------------------------------
// Calculate Tangent and Bitangent
// ----------------------------------------------
#if _CALCULATE_TBN
    for (int i = 0; i < num_poly; i++)
    {
        // poly

        gsk_QMapPolygon *poly =
          array_list_get_at_index(&p_brush->list_polygons, i);

        // vertices

        gsk_QMapPolygonVertex *vert_0 =
          array_list_get_at_index(&poly->list_vertices, 0);

        gsk_QMapPolygonVertex *vert_1 =
          array_list_get_at_index(&poly->list_vertices, 1);

        gsk_QMapPolygonVertex *vert_2 =
          array_list_get_at_index(&poly->list_vertices, 2);

        // data

        vec3 edge1 = {0.0f}, edge2 = {0.0f};
        vec2 delta1 = {0.0f}, delta2 = {0.0f};
        vec3 tangent = {0.0f}, bitangent = {0.0f};

        // get edges
        glm_vec3_sub(vert_1->position, vert_0->position, edge1);
        glm_vec3_sub(vert_2->position, vert_0->position, edge2);

        // delta 1
        delta1[0] = (vert_1->texture[0] - vert_0->texture[0]);
        delta1[1] = (vert_1->texture[1] - vert_0->texture[1]);

        // delta 2
        delta2[0] = (vert_2->texture[0] - vert_0->texture[1]);
        delta2[1] = (vert_2->texture[1] - vert_0->texture[1]);

        f32 f = 1.0f / (delta1[0] * delta2[1] - delta2[0] * delta1[1]);

        // tangent
        tangent[0] = f * (delta2[1] * edge1[0] - delta1[1] * edge2[0]);
        tangent[1] = f * (delta2[1] * edge1[1] - delta1[1] * edge2[1]);
        tangent[2] = f * (delta2[1] * edge1[2] - delta1[1] * edge2[2]);

        // bi-tangent
        bitangent[0] = f * (-delta2[0] * edge1[0] + delta1[0] * edge2[0]);
        bitangent[1] = f * (-delta2[0] * edge1[1] + delta1[0] * edge2[1]);
        bitangent[2] = f * (-delta2[0] * edge1[2] + delta1[0] * edge2[2]);

        glm_vec3_normalize(tangent);
        glm_vec3_normalize(bitangent);

        // copy data
        s32 num_vert = poly->list_vertices.list_next;

        for (int j = 0; j < num_vert; j++)
        {
            gsk_QMapPolygonVertex *vert =
              array_list_get_at_index(&poly->list_vertices, j);

            glm_vec3_copy(tangent, vert->tangent);
            // glm_vec3_copy(bitangent, vert->bitangent);
        }
    }
#endif _CALCULATE_TBN

    // ---------------------
    // Fix UV coordinates
    // ---------------------
#if _NORMALIZE_UV
    for (int i = 0; i < num_poly; i++)
    {
        gsk_QMapPolygon *poly =
          array_list_get_at_index(&p_brush->list_polygons, i);

        s32 num_vert = poly->list_vertices.list_next;

        gsk_QMapPolygonVertex *vert_first =
          array_list_get_at_index(&poly->list_vertices, 0);

        f32 nearest[2] = {vert_first->texture[0], vert_first->texture[1]};
        s32 nearest_indices[2] = {0, 0};

        // loop through each vertex
        for (int j = 0; j < num_vert; j++)
        {
            gsk_QMapPolygonVertex *vert =
              array_list_get_at_index(&poly->list_vertices, j);

            for (int k = 0; k < 2; k++)
            {
                if (fabs(vert->texture[k]) > 1)
                {
                    if (fabs(vert->texture[k]) < fabs(nearest[k]))
                    {
                        nearest_indices[k] = j;
                        nearest[k]         = vert->texture[k];
                    }
                } else
                {
                    // coordinates are already normalized
                    continue;
                }
            }
        }

        for (int j = 0; j < num_vert; j++)
        {

            gsk_QMapPolygonVertex *vert =
              array_list_get_at_index(&poly->list_vertices, j);
            vert->texture[0] = vert->texture[0] - nearest[0];
            vert->texture[1] = vert->texture[1] - nearest[1];
        }
    }
#endif

    // ---------------------
    // Assemble MeshData
    // ---------------------

    // loop through all polys
    for (int i = 0; i < num_poly; i++)
    {
        s32 vL  = 0; /* vertex positions count */
        s32 vtL = 0; /* vertex textures count  */
        s32 vnL = 0; /* vertex normals count   */

        gsk_QMapPolygon *poly =
          array_list_get_at_index(&p_brush->list_polygons, i);

#if _USE_CENTER_UV
        gsk_QMapPolygonVertex center_vert = {.texture  = {0.0f, 0.0f},
                                             .position = {0.0f, 0.0f, 0.0f}};
        glm_vec3_copy(poly->center, center_vert.position);

        // push the center vertex
        array_list_insert(&poly->list_vertices, &center_vert, 0);
        vL += 3;
        vtL += 2;

#if 0
        gsk_QMapPolygonVertex *second_vert =
          array_list_get_at_index(&poly->list_vertices, 1);
        array_list_push(&poly->list_vertices, &second_vert);
        vL += 3;
        vtL += 2;
#endif
#endif

        s32 num_vert = poly->list_vertices.list_next;

        // push the center vertex FIRST (Triangle-Fan rendering)

        // push vertex to buffer
        for (int j = 0; j < num_vert; j++)
        {
            // if (i != 0) continue;

            gsk_QMapPolygonVertex *vert =
              array_list_get_at_index(&poly->list_vertices, j);

            // increment vertex buffer lengths
            vL += 3;
            vtL += 2;
            vnL += 3;

            // poly fixation
#if 0
            if (j == num_vert - 1)
            {
                gsk_QMapPolygonVertex *vert_first =
                  array_list_get_at_index(&poly->list_vertices, 0);

                array_list_push(&p_container->vertices,
                                vert_first); // TODO: Remove once we construct
                                             // the meshdata from polygons

                // increment vertex buffer lengths
                vL += 3;
                vtL += 2;
            }
#endif
        }

        // Buffers for storing input
        // pos + tex + norm + tan
        s32 buff_count = vL + vnL + vtL + vnL;
        float *v       = malloc(buff_count * (sizeof(float) * 3));
        // v        = poly->list_vertices.data.buffer;

        // fill V
        s32 iter = 0;
        for (int m = 0; m < poly->list_vertices.list_next; m++)
        {
            gsk_QMapPolygonVertex *vert =
              array_list_get_at_index(&poly->list_vertices, m);

            v[iter + 0] = vert->position[0];
            v[iter + 1] = vert->position[1];
            v[iter + 2] = vert->position[2];
            iter += 3;

            v[iter + 0] = vert->texture[0];
            v[iter + 1] = vert->texture[1];
            iter += 2;

            v[iter + 0] = vert->normal[0];
            v[iter + 1] = vert->normal[1];
            v[iter + 2] = vert->normal[2];
            iter += 3;

            v[iter + 0] = vert->tangent[0];
            v[iter + 1] = vert->tangent[1];
            v[iter + 2] = vert->tangent[2];
            iter += 3;

            // glm_vec3_copy(vert->position, v + 0);
            // glm_vec2_copy(vert->texture, v[3]);
            // glm_vec3_copy(vert->normal, v + 5);
        }

        gsk_MeshData *meshdata = malloc(sizeof(gsk_MeshData));
        poly->p_mesh_data      = meshdata;

        meshdata->buffers.out = v;

        meshdata->buffers.outI = (buff_count) * sizeof(float);

        meshdata->vertexCount = vL / 3;

        meshdata->buffers.vL  = vL;
        meshdata->buffers.vtL = vtL;
        meshdata->buffers.vnL = vnL;
        meshdata->hasTBN      = MESH_TBN_MODE_GLTF;

        meshdata->buffers.bufferIndices_size = 0;
        meshdata->isSkinnedMesh              = 0;
        meshdata->has_indices                = FALSE;
        meshdata->primitive_type             = GSK_PRIMITIVE_TYPE_FAN;

        glm_vec3_zero(meshdata->boundingBox[0]);
        glm_vec3_zero(meshdata->boundingBox[1]);

        // p_container->mesh_data = meshdata;
    }

    /*
    for (int i = 0; i < p_container->vertices.list_next; i++) {
        // meshdata->buffers.out[i] = *(float
        // *)p_container->vertices.data.buffer+i;
    }
    */

    // LOG_INFO("iterations: %d", iterations);
}
/*--------------------------------------------------------------------*/

/**********************************************************************/
/*   Memory Functions                                                 */
/**********************************************************************/

/*--------------------------------------------------------------------*/
static void
__qmap_container_add_entity(gsk_QMapContainer *p_container)
{
    gsk_QMapEntity ent;
    ent.list_brushes = array_list_init(sizeof(gsk_QMapBrush), 1);
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
/*   Main Operation                                                   */
/**********************************************************************/

#define MODE_UP(x)   x = __next_mode(x, TRUE);
#define MODE_DOWN(x) x = __next_mode(x, FALSE);

/*--------------------------------------------------------------------*/
gsk_QMapContainer
gsk_qmap_load(const char *map_path, gsk_TextureSet *p_textureset)
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
        case '(':
            next_operation = QM_OP_NEW_MEMBER;
            //__read_plane(line);
            break;
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
            if (current_mode == QM_MODE_FILL_ENT)
            {
                // TODO:
                LOG_DEBUG("Fill entity member");
            } else if (current_mode == QM_MODE_FILL_BSH)
            {
                gsk_QMapPlane plane =
                  __parse_plane_from_line(line, p_textureset);
                array_list_push(&ret.p_cnt_brush->list_planes, &plane);
            }
        }
    }

    // close stream - tick compiled check
    fclose(stream);
    ret.is_map_compiled = TRUE;

    // build polygons for each brush
    for (int i = 0; i < ret.list_entities.list_next; i++)
    {
        gsk_QMapEntity *ent = array_list_get_at_index(&ret.list_entities, i);
        for (int j = 0; j < ent->list_brushes.list_next; j++)
        {
            gsk_QMapBrush *brush =
              array_list_get_at_index(&ent->list_brushes, j);

            __qmap_polygons_from_brush(&ret, brush);
        }
    }

    return ret;
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
gsk_Model *
gsk_qmap_load_model(gsk_QMapContainer *p_container)
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

    /* load shader */

    gsk_ShaderProgram *qmap_shader =
      gsk_shader_program_create(GSK_PATH("gsk://shaders/lit-diffuse.shader"));

    /* default material */
    gsk_Material *p_material_err = gsk_material_create(
      qmap_shader,
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

                qmap_model->meshes[cnt_poly] =
                  gsk_mesh_assemble((gsk_MeshData *)poly->p_mesh_data);

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
                      qmap_shader,
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
    return qmap_model;
}
/*--------------------------------------------------------------------*/