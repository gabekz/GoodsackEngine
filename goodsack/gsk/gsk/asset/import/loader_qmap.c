/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "loader_qmap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/graphics/mesh/mesh.h"

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

    if (denom == 0)
    {
        return FALSE; // No intersection, the planes are parallel or coincident
    }

    vec3 term1, term2, term3;
    glm_vec3_scale(cross1, -d1, term1);
    glm_vec3_scale(cross2, -d2, term2);
    glm_vec3_scale(cross3, -d3, term3);

    vec3 sum, result;
    glm_vec3_add(term1, term2, sum);
    glm_vec3_add(sum, term3, result);

    glm_vec3_scale(result, 1.0f / denom, output);

    return TRUE;
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
static void
__calculate_uv_coords(vec3 vertex,
                      vec3 p0,
                      vec3 u_axis,
                      vec3 v_axis,
                      f32 s,
                      f32 t,
                      f32 scale_x,
                      f32 scale_y,
                      f32 *output)
{
    vec3 adjusted_vertex = {0, 0, 0};
    glm_vec3_sub(p0, vertex, adjusted_vertex);

    // TODO: ensure uv-axes are normalized

    output[0] = glm_vec3_dot(adjusted_vertex, u_axis) / scale_x + s; // u
    output[1] = glm_vec3_dot(adjusted_vertex, v_axis) / scale_y + t; // v

    glm_vec2_normalize(output);
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
    glm_vec3_cross(pq, pr, norm_out);

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
#if 0

    vec3 check1 = {0}, check2 = {0};

    glm_vec3_scale(plane_norm, -plane_deter, check1);
    glm_vec3_sub(check1, point, check2);

    f32 result = glm_vec3_dot(check2, plane_norm);

#elif 0

    f32 result = plane_norm[0] * point[0] + plane_norm[1] * point[1] +
                 plane_norm[2] * point[2] + plane_deter;
#else
    f32 result = glm_vec3_dot(plane_norm, point) + plane_deter;
#endif

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
__parse_plane_from_line(char *line)
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
    f32 texture_properties[5] = {0, 0, 0, 0, 0};
    vec3 u_axis = {0, 0, 0}, v_axis = {0, 0, 0};

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

    /*==== Plane normal and determinant ==============================*/
    {
        // normal
        vec3 pq, pr;
        glm_vec3_sub(points[1], points[0], pq);
        glm_vec3_sub(points[2], points[0], pr);
        glm_vec3_cross(pq, pr, normal);

        // determinant
        determinant = (normal[0] * points[0][0] + normal[1] * points[0][1] +
                       normal[2] * points[0][2]);
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

    while (split != NULL)
    {
        if (i == -1)
        {
            texture_name = strdup(split);
        } else
        {
            texture_properties[i] = atof(split);
        }
        split = strtok(NULL, delim);
        i++;
    }

    /*==== Calculate uv axes =========================================*/
    /*   TODO: Find out if uv axes come from .map file format         */
    {
        vec3 axisref = {0.0f, 0.0f, 1.0f}; // z-axis reference

        if (fabs(glm_vec3_dot(normal, axisref)) > 0.999f)
        {
            axisref[0] = 1.0f; // use x-axis if normal is close to z-axis
            axisref[2] = 0.0f;
        }

        glm_vec3_crossn(normal, axisref, u_axis);
        glm_vec3_crossn(normal, u_axis, v_axis);
    }

#if 1 // LOG_PLANE
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
#endif

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
    f32 vL           = 0;
    f32 vtL          = 0;
    s32 planes_count = p_brush->list_planes.list_next;
    void *data       = p_brush->list_planes.data.buffer;

    gsk_QMapPlane *p_planes = data;

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
        for (int j = 0; j < planes_count; j++)
        {
            for (int k = 0; k < planes_count; k++)
            {
                iterations++;

                // do not check same
                if (i == j || i == k || j == k) { continue; }

                vec3 vertex; // filled by __get_intersection()
                u8 is_illegal   = FALSE;
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
                    if (check1 > 0)
                    {
                        is_illegal = TRUE;
                        break;
                    }
                }

                if (is_illegal == TRUE) { continue; }

                // increment vertexList track | TODO: Remove
                vL += 3;
                vtL += 2;
#if POLY_PER_FACE
                gsk_QMapPolygon *poly =
                  array_list_get_at_index(&p_brush->list_polygons, i);

                vec3 p0 = {0, 0, 0};

                // Get p0
                if (&poly->list_vertices.list_next <= 0)
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

                // calculate UV coords
                __calculate_uv_coords(vertex,
                                      p0,
                                      p_planes[i].uv_axes[0],
                                      p_planes[i].uv_axes[1],
                                      p_planes->tex_offset[0],
                                      p_planes->tex_offset[1],
                                      p_planes->tex_scale[0],
                                      p_planes->tex_scale[1],
                                      vert.texture);

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
        // get polygon center
        vec3 center = {0, 0, 0};

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
        glm_vec3_divs(center, num_vert + 1, center);

        // start sorting

        for (int n = 0; n < num_vert - 2; n++)
        {
            // get current vert
            gsk_QMapPolygonVertex *vert =
              array_list_get_at_index(&poly->list_vertices, n);

// I don't know if I need this here......
#if 0
            array_list_push(&p_container->vertices,
                            vert); // TODO: Remove once we construct the
                                   // meshdata from polygons
#endif

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
                    LOG_WARN("Poly %d, vert %d, m %d is: %d", i, n, m, facing);
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

        // push vertex to buffer
        for (int j = 0; j < num_vert; j++)
        {
            // if (i != 0) continue;

            gsk_QMapPolygonVertex *vert =
              array_list_get_at_index(&poly->list_vertices, j);

            array_list_push(&p_container->vertices,
                            vert); // TODO: Remove once we construct the
                                   // meshdata from polygons
        }
    }

    // ---------------------
    // Assemble MeshData
    // ---------------------

    // Buffers for storing input
    float *v = malloc((vL + vtL) * sizeof(float) * 3);

    gsk_MeshData *meshdata = malloc(sizeof(gsk_MeshData));
    v                      = p_container->vertices.data.buffer;

    meshdata->buffers.out = v;
    meshdata->buffers.v   = v;

    meshdata->buffers.outI = (vL + vtL) * sizeof(float);

    meshdata->vertexCount = vL / 3;

    meshdata->buffers.vL  = vL;
    meshdata->buffers.vtL = vtL;
    meshdata->buffers.vnL = 0;

    meshdata->buffers.bufferIndices_size = 0;
    meshdata->isSkinnedMesh              = 0;
    meshdata->hasTBN                     = 0;
    meshdata->has_indices                = FALSE;
    meshdata->primitive_type             = GSK_PRIMITIVE_TYPE_TRIANGLE;

    glm_vec3_zero(meshdata->boundingBox[0]);
    glm_vec3_zero(meshdata->boundingBox[1]);

    p_container->mesh_data = meshdata;

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
                gsk_QMapPlane plane = __parse_plane_from_line(line);
                array_list_push(&ret.p_cnt_brush->list_planes, &plane);
            }
        }
    }

    fclose(stream);

    // Build a polygon from the last brush
    ret.vertices = array_list_init(sizeof(gsk_QMapPolygonVertex), 20);

    __qmap_polygons_from_brush(&ret, ret.p_cnt_brush);

    return ret;
}
/*--------------------------------------------------------------------*/