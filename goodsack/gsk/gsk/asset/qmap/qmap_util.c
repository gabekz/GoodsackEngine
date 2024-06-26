
/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "qmap_util.h"

#include "util/maths.h"
#include "util/sysdefs.h"

#include "asset/qmap/qmapdefs.h"

/*--------------------------------------------------------------------*/
u8
gsk_qmap_util_get_intersection(
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
u8
gsk_qmap_util_compare_verts(const f32 *v1, const f32 *v2, const f32 epsilon)
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
void
gsk_qmap_util_plane_from_points(
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

/*--------------------------------------------------------------------*/
s32
gsk_qmap_util_classify_point(vec3 point, vec3 plane_norm, f32 plane_deter)
{
    f32 result = glm_vec3_dot(plane_norm, point) - plane_deter;

    if (result > 0.0f)
    {
        return QMAP_CLASS_FRONT;
    } else if (result < 0.0f)
    {
        return QMAP_CLASS_BACK;
    }

    return QMAP_CLASS_ON_PLANE;
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
gsk_QMapEntityField *
gsk_qmap_util_get_field(gsk_QMapEntity *p_entity, const char *key_string)
{
    for (int i = 0; i < p_entity->list_fields.list_next; i++)
    {
        gsk_QMapEntityField *field =
          array_list_get_at_index(&p_entity->list_fields, i);
        if (!strcmp(field->key, key_string)) { return field; }
    }

    return NULL;
}
/*--------------------------------------------------------------------*/