/*
 * Copyright (c) 2024-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "qmap_build.h"

#include "asset/qmap/qmap_util.h"
#include "asset/qmap/qmapdefs.h"

#include "core/graphics/material/material.h"
#include "core/graphics/mesh/mesh.h"
#include "core/graphics/mesh/model.h"
#include "core/graphics/shader/shader.h"
#include "core/graphics/texture/texture_set.h"

#include "util/filesystem.h"
#include "util/logger.h"
#include "util/maths.h"
#include "util/sysdefs.h"

// Options
#define POLY_PER_FACE TRUE // generate a polygon for each face

// Stuff that breaks
#define _NORMALIZE_UV  FALSE
#define _USE_CENTER_UV FALSE
#define _CALCULATE_TBN TRUE
#define _USING_INDICES TRUE

#define _FIX_POINT_FACING TRUE
#define _FIX_UV_FACING    TRUE

#define EPSILON_ILLEGAL_POINT 0.001f

/**********************************************************************/
/*   Helper Functions                                                 */
/**********************************************************************/

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

/**********************************************************************/
/*   Build Functions                                                  */
/**********************************************************************/

/*--------------------------------------------------------------------*/
void
gsk_qmap_build_polys_from_brush(gsk_QMapContainer *p_container,
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
          array_list_init(sizeof(gsk_QMapPolygonVertex), QMAP_ALLOC_ITER);

        // push this polygon to the list on the brush
        array_list_push(&p_brush->list_polygons, &poly);
    }
#endif

    // ----------------------------------------------
    // Polygon generation from intersecting planes
    // ----------------------------------------------

#if 0
    LOG_TRACE("Assembling polygon from brush (brush_index: %d)",
              p_brush->brush_index);
#endif

    u32 iterations = 0; // for debugging purposes
    for (int i = 0; i < planes_count; i++)
    {
        // LOG_DEBUG("Checking intersections for poly %d", i);

        for (int j = 0; j < planes_count; j++)
        {
            for (int k = 0; k < planes_count; k++)
            {
                iterations++;

                // do not check same
                if (i == j || i == k || j == k) { continue; }

                vec3 vertex; // filled by get_intersection()
                u8 is_illegal   = FALSE;
                u8 is_duplicate = FALSE;
                u8 is_intersect =
                  gsk_qmap_util_get_intersection(p_planes[i].normal,
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
                    if (check1 > EPSILON_ILLEGAL_POINT)
                    {
                        is_illegal = TRUE;
#if 0
                        LOG_TRACE(
                          "Illegal point - vertex: %d (term: %f)", m, check1);
#endif
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

                    if (gsk_qmap_util_compare_verts(
                          vert.position, compare->position, 0.00001f))
                    {
                        is_duplicate = TRUE;
                        // LOG_TRACE("vertex is duplicate..");
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

#if 0
                LOG_TRACE(
                  "Vertex at (%f, %f, %f)", vertex[0], vertex[1], vertex[2]);
#endif
            }
        }
    }

    LOG_TRACE("Assembled polygon with %d iterations", iterations);

    // ----------------------------------------------
    // Sort polygon vertices
    // ----------------------------------------------

    int num_poly = p_brush->list_polygons.list_next;

    for (int i = 0; i < num_poly; i++)
    {
        // LOG_TRACE("Sorting polygon %d", i);

        // if (i != 2) continue;

        // get polygon center
        vec3 center = {1.1f, 1.0f, 0.9f};
        glm_vec3_scale(center, QMAP_IMPORT_SCALE, center);
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

                gsk_qmap_util_plane_from_points(
                  p1, p2, p3, plane_norm, &plane_deter);

                // glm_vec3_normalize(plane_norm);
            }

            for (int m = n + 1; m < num_vert; m++)
            {
                // if (m == n + 1) { smallest = m + 1; }

                gsk_QMapPolygonVertex *vert_m =
                  array_list_get_at_index(&poly->list_vertices, m);

                s32 facing = gsk_qmap_util_classify_point(
                  vert_m->position, plane_norm, plane_deter);

                if (facing != QMAP_CLASS_BACK)
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
                }
#if 0
                else
                {
                    LOG_TRACE("Poly %d, vert %d, m %d facing is: %s",
                              i,
                              n,
                              m,
                              (facing == QMAP_CLASS_BACK)
                                ? "QMAP_CLASS_BACK"
                                : (facing == QMAP_CLASS_FRONT
                                     ? "QMAP_CLASS_FRONT"
                                     : "QMAP_CLASS_ON_PLANE"));
                }
#endif
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

        u32 num_vert    = poly->list_vertices.list_next;
        u32 num_indices = (num_vert - 2) * 3;

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

        // setup bounds
        // TODO: float minmax
        vec3 minBounds = {10000, 10000, 10000};
        vec3 maxBounds = {-10000, -10000, -10000};

        // Buffers for storing input
        // pos + tex + norm + tan
        s32 buff_count    = vL + vnL + vtL + vnL;
        float *buff_verts = malloc(buff_count * (sizeof(float) * 3));
#if _USING_INDICES
        u32 *buff_indices = malloc(num_indices * sizeof(u32));
#endif // _USING_INDICES

        // fill V
        s32 iter = 0;
        for (int m = 0; m < poly->list_vertices.list_next; m++)
        {
            gsk_QMapPolygonVertex *vert =
              array_list_get_at_index(&poly->list_vertices, m);

            buff_verts[iter + 0] = vert->position[0];
            buff_verts[iter + 1] = vert->position[1];
            buff_verts[iter + 2] = vert->position[2];
            iter += 3;

            {
                if (vert->position[0] < minBounds[0])
                    minBounds[0] = vert->position[0];
                if (vert->position[0] > maxBounds[0])
                    maxBounds[0] = vert->position[0];

                if (vert->position[1] < minBounds[1])
                    minBounds[1] = vert->position[1];
                if (vert->position[1] > maxBounds[1])
                    maxBounds[1] = vert->position[1];

                if (vert->position[2] < minBounds[2])
                    minBounds[2] = vert->position[2];
                if (vert->position[2] > maxBounds[2])
                    maxBounds[2] = vert->position[2];
            }

            buff_verts[iter + 0] = vert->texture[0];
            buff_verts[iter + 1] = vert->texture[1];
            iter += 2;

            buff_verts[iter + 0] = vert->normal[0];
            buff_verts[iter + 1] = vert->normal[1];
            buff_verts[iter + 2] = vert->normal[2];
            iter += 3;

            buff_verts[iter + 0] = vert->tangent[0];
            buff_verts[iter + 1] = vert->tangent[1];
            buff_verts[iter + 2] = vert->tangent[2];
            iter += 3;

            // glm_vec3_copy(vert->position, v + 0);
            // glm_vec2_copy(vert->texture, v[3]);
            // glm_vec3_copy(vert->normal, v + 5);
        }

#if _USING_INDICES
        int k = 0;
        for (int n = 1; n < num_vert - 1; n++)
        {
            buff_indices[k++] = 0;
            buff_indices[k++] = n;
            buff_indices[k++] = n + 1;
        }
#endif // _USING_INDICES

        gsk_MeshData *meshdata = malloc(sizeof(gsk_MeshData));
        poly->p_mesh_data      = meshdata;

        meshdata->mesh_buffers_count = 0;
        meshdata->usage_draw         = GskOglUsageType_Static;

        meshdata->mesh_buffers_count++;
        meshdata->mesh_buffers[0] = (gsk_MeshBuffer) {
          .buffer_flags =
            (GskMeshBufferFlag_Positions | GskMeshBufferFlag_Textures |
             GskMeshBufferFlag_Normals | GskMeshBufferFlag_Tangents),
          .p_buffer    = buff_verts,
          .buffer_size = buff_count * sizeof(float),
        };
#if _USING_INDICES
        meshdata->mesh_buffers_count++;
        meshdata->mesh_buffers[1] = (gsk_MeshBuffer) {
          .buffer_flags = (GskMeshBufferFlag_Indices),
          .p_buffer     = buff_indices,
          .buffer_size  = num_indices * sizeof(u32),
        };
#endif // _USING_INDICES

        meshdata->vertexCount    = vL / 3;
        meshdata->indicesCount   = (_USING_INDICES) ? num_indices : 0;
        meshdata->primitive_type = (_USING_INDICES)
                                     ? GskMeshPrimitiveType_Triangle
                                     : GskMeshPrimitiveType_Fan;

        // copy bounds
        glm_vec3_copy(minBounds, meshdata->boundingBox[0]);
        glm_vec3_copy(maxBounds, meshdata->boundingBox[1]);

#if 0
        // calculate local-space bounds with aabb-center
        glm_aabb_center(meshdata->boundingBox, meshdata->world_pos);

        glm_vec3_sub(minBounds, meshdata->world_pos, meshdata->boundingBox[0]);
        glm_vec3_sub(maxBounds, meshdata->world_pos, meshdata->boundingBox[1]);
#endif
    }
}
/*--------------------------------------------------------------------*/