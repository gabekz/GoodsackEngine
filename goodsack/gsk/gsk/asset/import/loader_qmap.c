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

static u8
__get_intersection(
  f32 *n1, f32 *n2, f32 *n3, f32 d1, f32 d2, f32 d3, f32 *output)
{
    vec3 cross1, cross2, cross3;
    glm_vec3_cross(n2, n3, cross1);
    glm_vec3_cross(n3, n1, cross2);
    glm_vec3_cross(n1, n2, cross3);

    f32 denom = glm_vec3_dot(n1, cross1);

    if (denom == 0) {
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

//-----------------------------------------------------------------------------
// Parsing functions
//-----------------------------------------------------------------------------

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
    // --- plane normal and determinant
    //
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

    //
    // --- RETURN
    //

    // copy points
    glm_vec3_copy(points[0], ret.points[0]);
    glm_vec3_copy(points[1], ret.points[1]);
    glm_vec3_copy(points[2], ret.points[2]);

    // copy normal
    glm_vec3_copy(normal, ret.normal);

    ret.determinant = determinant;

    // copy texture offsets
    ret.tex_offset[0] = texture_properties[0];
    ret.tex_offset[1] = texture_properties[1];
    ret.tex_rotation  = texture_properties[2];
    ret.tex_scale[0]  = texture_properties[3];
    ret.tex_scale[1]  = texture_properties[4];

    return ret;
}

//-----------------------------------------------------------------------------
// Build functions
//-----------------------------------------------------------------------------

static void
__qmap_polygon_from_brush(gsk_QMapContainer *p_container,
                          gsk_QMapBrush *p_brush)
{
    f32 vL           = 0;
    s32 planes_count = p_brush->list_planes.list_next;
    void *data       = p_brush->list_planes.data.buffer;

    gsk_QMapPlane *p_planes = data;

#if POLY_PER_FACE
    // Create polygon for each face

    p_brush->list_polygons = array_list_init(
      sizeof(gsk_QMapPolygon), planes_count); // iterate at planes_count

    for (int i = 0; i < planes_count; i++) {
        gsk_QMapPolygon poly;
        poly.list_vertices = array_list_init(sizeof(vec3), 3);

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
    for (int i = 0; i < planes_count; i++) {
        for (int j = 0; j < planes_count; j++) {
            for (int k = 0; k < planes_count; k++) {
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
                for (int m = 0; m < planes_count; m++) {
                    f32 term1  = glm_vec3_dot(p_planes[m].normal, vertex);
                    f32 check1 = term1 + p_planes[m].determinant;
                    if (check1 > 0) {
                        is_illegal = TRUE;
                        break;
                    }
                }

                if (is_illegal == TRUE) { continue; }

                vL += 3; // increment vertexList track | TODO: Remove
#if POLY_PER_FACE
                gsk_QMapPolygon *poly =
                  array_list_get_at_index(&p_brush->list_polygons, i);

                array_list_push(&poly->list_vertices, &vertex);

                array_list_push(&p_container->vertices,
                                &vertex); // TODO: Remove once we construct the
                                          // meshdata from polygons
#else
                array_list_push(&p_container->vertices, &vertex);
#endif

#if 1
                LOG_TRACE(
                  "Vertex at (%f, %f, %f)", vertex[0], vertex[1], vertex[2]);
#endif
            }
        }
    }

    LOG_DEBUG("Assembled polygon with %d iterations", iterations);

    // ---------------------
    // Assemble MeshData
    // ---------------------

    // Buffers for storing input
    float *v = malloc(vL * sizeof(float) * 3);

    gsk_MeshData *meshdata = malloc(sizeof(gsk_MeshData));
    v                      = p_container->vertices.data.buffer;

    meshdata->buffers.out = v;
    meshdata->buffers.v   = v;

    meshdata->buffers.vL   = vL;
    meshdata->buffers.outI = vL * sizeof(float);

    meshdata->vertexCount = vL / 3;

    meshdata->buffers.vtL = 0;
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

//-----------------------------------------------------------------------------
// Memory functions
//-----------------------------------------------------------------------------

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
    brush.list_planes = array_list_init(sizeof(gsk_QMapPlane), 3);
    brush.brush_index = p_container->total_brushes;

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

#if 0
static void
__qmap_brush_add_plane(gsk_QMapBrush *p_brush, gsk_QMapPlane *p_plane)
{
}
#endif

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
        if (next_operation == QM_OP_NEW_GROUP) {
            next_operation = QM_OP_NONE;

            if (current_mode == QM_MODE_FILL_ENT) {
                LOG_DEBUG("Creating new entity");
                __qmap_container_add_entity(&ret);
            } else if (current_mode == QM_MODE_FILL_BSH) {
                LOG_DEBUG("Creating new brush");
                __qmap_container_add_brush(&ret);
            }
        }
        // Members/fields (entity field, plane)
        else if (next_operation == QM_OP_NEW_MEMBER) {
            next_operation = QM_OP_NONE;
            if (current_mode == QM_MODE_FILL_ENT) {
                // TODO:
                LOG_DEBUG("Fill entity member");
            } else if (current_mode == QM_MODE_FILL_BSH) {
                gsk_QMapPlane plane = __parse_plane_from_line(line);
                array_list_push(&ret.p_cnt_brush->list_planes, &plane);
            }
        }
    }

    fclose(stream);

    // Build a polygon from the last brush
    ret.vertices = array_list_init(sizeof(vec3), 20);

    __qmap_polygon_from_brush(&ret, ret.p_cnt_brush);

    return ret;
}