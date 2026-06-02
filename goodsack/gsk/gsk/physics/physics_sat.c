/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "physics_sat.h"

#include "util/logger.h"
#include "util/maths.h"
#include "util/sysdefs.h"

#include "physics_types.h"

#define OBB_EPSILON        1e-6f
#define GSK_CLIP_MAX_VERTS 8

#define SAT_EDGE_AXIS_EPSILON       1e-5f
#define SAT_FACE_PREFERENCE_EPSILON 1e-3f

typedef struct gsk_ClipVertex
{
    vec3 p;
} gsk_ClipVertex;

// Helper: clamp a value to [minVal, maxVal]
// TODO: move helpers out of here
static inline float
clampf(float x, float minVal, float maxVal)
{
    if (x < minVal) return minVal;
    if (x > maxVal) return maxVal;
    return x;
}

// TODO: move helpers out of here
static inline float
signf_nonzero(float x)
{
    return (x >= 0.0f) ? 1.0f : -1.0f;
}

// TODO: move to helpers
static void
_closest_point_line_segment(vec3 a, vec3 b, vec3 point, float *dest)
{
    vec3 ab, p_a;
    glm_vec3_sub(b, a, ab);      // ab = B - A
    glm_vec3_sub(point, a, p_a); // p_a = Point - A

    f32 t = glm_dot(p_a, ab) / glm_dot(ab, ab);

    f32 offset = MIN(MAX(t, 0), 1); // saturate

    // return A + saturate(t) * AB
    glm_vec3_scale(ab, offset, dest);
    glm_vec3_add(a, dest, dest);
}

static u8
_test_obb(gsk_OBB *a, gsk_OBB *b)
{
    f32 ra = 0, rb = 0;
    mat3 R    = GLM_MAT3_IDENTITY_INIT;
    mat3 AbsR = GLM_MAT3_IDENTITY_INIT;

    // rotation matrix expressing b in a
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            R[i][j] = glm_dot(a->u[i], b->u[j]);
        }
    }

    // translational vector t
    vec3 t = GLM_VEC3_ZERO_INIT;
    {
        vec3 t0 = GLM_VEC3_ZERO_INIT;
        glm_vec3_sub(b->c, a->c, t0);

        t[0] = glm_dot(t0, a->u[0]);
        t[1] = glm_dot(t0, a->u[1]);
        t[2] = glm_dot(t0, a->u[2]);
    }

    // compute subexpressions
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            AbsR[i][j] = fabsf(R[i][j]);
            // TODO: add epsilon!
        }
    }

    // Test axes L = A0, L = A1, L = A2
    for (int i = 0; i < 3; i++)
    {
        ra = a->e[i];
        rb = b->e[0] * AbsR[i][0] + b->e[1] * AbsR[i][1] + b->e[2] * AbsR[i][2];
        if (fabsf(t[i]) > ra + rb) return 0;
    }

    // Test axes L = B0, L = B1, L = B2
    for (int i = 0; i < 3; i++)
    {
        ra = a->e[0] * AbsR[0][i] + a->e[1] * AbsR[1][i] + a->e[2] * AbsR[2][i];
        rb = b->e[i];

        if (fabsf(t[0] * R[0][i] + t[1] * R[1][i] + t[2] * R[2][i]) > ra + rb)
        {
            return 0;
        }
    }

    // Test axis L = A0 x B0
    ra = a->e[1] * AbsR[2][0] + a->e[2] * AbsR[1][0];
    rb = b->e[1] * AbsR[0][2] + b->e[2] * AbsR[0][1];
    if (fabsf(t[2] * R[1][0] - t[1] * R[2][0]) > ra + rb) { return 0; }

    // Test axis L = A0 x B1
    ra = a->e[1] * AbsR[2][1] + a->e[2] * AbsR[1][1];
    rb = b->e[0] * AbsR[0][2] + b->e[2] * AbsR[0][0];
    if (fabsf(t[2] * R[1][1] - t[1] * R[2][1]) > ra + rb) { return 0; }

    // Test axis L = A0 x B2
    ra = a->e[1] * AbsR[2][2] + a->e[2] * AbsR[1][2];
    rb = b->e[0] * AbsR[0][1] + b->e[1] * AbsR[0][0];
    if (fabsf(t[2] * R[1][2] - t[1] * R[2][2]) > ra + rb) { return 0; }

    // Test axis L = A1 x B0
    ra = a->e[0] * AbsR[2][0] + a->e[2] * AbsR[0][0];
    rb = b->e[1] * AbsR[1][2] + b->e[2] * AbsR[1][1];
    if (fabsf(t[0] * R[2][0] - t[2] * R[0][0]) > ra + rb) { return 0; }

    // Test axis L = A1 x B1
    ra = a->e[0] * AbsR[2][1] + a->e[2] * AbsR[0][1];
    rb = b->e[0] * AbsR[1][2] + b->e[2] * AbsR[1][0];
    if (fabsf(t[0] * R[2][1] - t[2] * R[0][1]) > ra + rb) { return 0; }

    // Test axis L = A1 x B2
    ra = a->e[0] * AbsR[2][2] + a->e[2] * AbsR[0][2];
    rb = b->e[0] * AbsR[1][1] + b->e[1] * AbsR[1][0];
    if (fabsf(t[0] * R[2][2] - t[2] * R[0][2]) > ra + rb) { return 0; }

    // Test axis L = A2 x B0
    ra = a->e[0] * AbsR[1][0] + a->e[1] * AbsR[0][0];
    rb = b->e[1] * AbsR[2][2] + b->e[2] * AbsR[2][1];
    if (fabsf(t[1] * R[0][0] - t[0] * R[1][0]) > ra + rb) { return 0; }

    // Test axis L = A2 x B1
    ra = a->e[0] * AbsR[1][1] + a->e[1] * AbsR[0][1];
    rb = b->e[0] * AbsR[2][2] + b->e[2] * AbsR[2][0];
    if (fabsf(t[1] * R[0][1] - t[0] * R[1][1]) > ra + rb) { return 0; }

    // Test axis L = A2 x B2
    ra = a->e[0] * AbsR[1][2] + a->e[1] * AbsR[0][2];
    rb = b->e[0] * AbsR[2][1] + b->e[1] * AbsR[2][0];
    if (fabsf(t[1] * R[0][2] - t[0] * R[1][2]) > ra + rb) { return 0; }

    return 1;
}

static u8
_sat_try_axis(gsk_OBB *a,
              gsk_OBB *b,
              vec3 axis_world,
              gsk_OBBAxisType type,
              int axis_a,
              int axis_b,
              gsk_OBBSatResult *out)
{
    float len2 = glm_vec3_norm2(axis_world);

    // skip near-parallel
    if (len2 < SAT_EDGE_AXIS_EPSILON) return TRUE;

    vec3 axis;
    glm_vec3_scale(axis_world, 1.0f / sqrtf(len2), axis);

    float center_a = glm_vec3_dot(a->c, axis);
    float center_b = glm_vec3_dot(b->c, axis);

    float radius_a = 0.0f;
    float radius_b = 0.0f;

    for (int i = 0; i < 3; ++i)
    {
        radius_a += a->e[i] * fabsf(glm_vec3_dot(axis, a->u[i]));
        radius_b += b->e[i] * fabsf(glm_vec3_dot(axis, b->u[i]));
    }

    float signed_dist = center_b - center_a;
    float overlap     = radius_a + radius_b - fabsf(signed_dist);

    if (overlap < 0.0f) return FALSE;

    if (signed_dist < 0.0f) glm_vec3_negate(axis);

    // prefer face-axis over edge-edge
    u8 should_replace = FALSE;

    if (out->type == GSK_OBB_AXIS_NONE)
    {
        should_replace = TRUE;
    } else if (type == GSK_OBB_AXIS_EDGE && out->type != GSK_OBB_AXIS_EDGE)
    {
        // edge can only replace face if it's clearly better
        if (overlap < out->depth - SAT_FACE_PREFERENCE_EPSILON)
            should_replace = TRUE;
    } else if (type != GSK_OBB_AXIS_EDGE && out->type == GSK_OBB_AXIS_EDGE)
    {
        if (overlap <= out->depth + SAT_FACE_PREFERENCE_EPSILON)
            should_replace = TRUE;
    } else
    {
        if (overlap < out->depth) should_replace = TRUE;
    }

    if (should_replace)
    {
        out->depth  = overlap;
        out->type   = type;
        out->axis_a = axis_a;
        out->axis_b = axis_b;
        glm_vec3_copy(axis, out->normal);
    }

    return TRUE;
}

static void
_obb_support_point(const gsk_OBB *o, vec3 dir, vec3 out)
{
    glm_vec3_copy(o->c, out);

    for (int i = 0; i < 3; ++i)
    {
        float s = (glm_vec3_dot(dir, o->u[i]) >= 0.0f) ? 1.0f : -1.0f;

        vec3 term;
        glm_vec3_scale(o->u[i], s * o->e[i], term);
        glm_vec3_add(out, term, out);
    }
}

static void
_obb_local_to_world(const gsk_OBB *o, vec3 local, vec3 out)
{
    glm_vec3_copy(o->c, out);

    for (int i = 0; i < 3; ++i)
    {
        vec3 term;
        glm_vec3_scale(o->u[i], local[i], term);
        glm_vec3_add(out, term, out);
    }
}

static void
_obb_world_to_local(const gsk_OBB *o, vec3 world, vec3 out_local)
{
    vec3 d;
    glm_vec3_sub(world, o->c, d);

    out_local[0] = glm_vec3_dot(d, o->u[0]);
    out_local[1] = glm_vec3_dot(d, o->u[1]);
    out_local[2] = glm_vec3_dot(d, o->u[2]);
}

static void
_obb_clamp_point_to_face(const gsk_OBB *ref,
                         int face_axis,
                         float face_sign,
                         vec3 world_point,
                         vec3 out)
{
    vec3 d;
    glm_vec3_sub(world_point, ref->c, d);

    vec3 local = GLM_VEC3_ZERO_INIT;

    for (int i = 0; i < 3; ++i)
    {
        local[i] = glm_vec3_dot(d, ref->u[i]);
        local[i] = clampf(local[i], -ref->e[i], ref->e[i]);
    }

    local[face_axis] = face_sign * ref->e[face_axis];

    _obb_local_to_world(ref, local, out);
}

static void
_closest_points_segments(vec3 p1, vec3 q1, vec3 p2, vec3 q2, vec3 c1, vec3 c2)
{
    vec3 d1, d2, r;
    glm_vec3_sub(q1, p1, d1);
    glm_vec3_sub(q2, p2, d2);
    glm_vec3_sub(p1, p2, r);

    float a = glm_vec3_dot(d1, d1);
    float e = glm_vec3_dot(d2, d2);
    float f = glm_vec3_dot(d2, r);

    float s, t;

    if (a <= OBB_EPSILON && e <= OBB_EPSILON)
    {
        glm_vec3_copy(p1, c1);
        glm_vec3_copy(p2, c2);
        return;
    }

    if (a <= OBB_EPSILON)
    {
        s = 0.0f;
        t = clampf(f / e, 0.0f, 1.0f);
    } else
    {
        float c = glm_vec3_dot(d1, r);

        if (e <= OBB_EPSILON)
        {
            t = 0.0f;
            s = clampf(-c / a, 0.0f, 1.0f);
        } else
        {
            float b     = glm_vec3_dot(d1, d2);
            float denom = a * e - b * b;

            if (denom != 0.0f)
                s = clampf((b * f - c * e) / denom, 0.0f, 1.0f);
            else
                s = 0.0f;

            t = (b * s + f) / e;

            if (t < 0.0f)
            {
                t = 0.0f;
                s = clampf(-c / a, 0.0f, 1.0f);
            } else if (t > 1.0f)
            {
                t = 1.0f;
                s = clampf((b - c) / a, 0.0f, 1.0f);
            }
        }
    }

    vec3 tmp;

    glm_vec3_scale(d1, s, tmp);
    glm_vec3_add(p1, tmp, c1);

    glm_vec3_scale(d2, t, tmp);
    glm_vec3_add(p2, tmp, c2);
}

static void
_get_obb_edge_segment(
  const gsk_OBB *o, int edge_axis, vec3 support_dir, vec3 out_a, vec3 out_b)
{
    vec3 local = GLM_VEC3_ZERO_INIT;

    for (int i = 0; i < 3; ++i)
    {
        if (i == edge_axis) continue;

        float d  = glm_vec3_dot(support_dir, o->u[i]);
        local[i] = signf_nonzero(d) * o->e[i];
    }

    local[edge_axis] = -o->e[edge_axis];
    _obb_local_to_world(o, local, out_a);

    local[edge_axis] = o->e[edge_axis];
    _obb_local_to_world(o, local, out_b);
}

/*************************************************************************
 * OBB clipping helpers
 *************************************************************************/

static void
_obb_get_face_vertices(const gsk_OBB *o,
                       int face_axis,
                       float face_sign,
                       gsk_ClipVertex out[4])
{
    int axis_u = (face_axis + 1) % 3;
    int axis_v = (face_axis + 2) % 3;

    vec3 local[4] = {
      {0},
      {0},
      {0},
      {0},
    };

    local[0][face_axis] = face_sign * o->e[face_axis];
    local[1][face_axis] = face_sign * o->e[face_axis];
    local[2][face_axis] = face_sign * o->e[face_axis];
    local[3][face_axis] = face_sign * o->e[face_axis];

    local[0][axis_u] = -o->e[axis_u];
    local[0][axis_v] = -o->e[axis_v];

    local[1][axis_u] = o->e[axis_u];
    local[1][axis_v] = -o->e[axis_v];

    local[2][axis_u] = o->e[axis_u];
    local[2][axis_v] = o->e[axis_v];

    local[3][axis_u] = -o->e[axis_u];
    local[3][axis_v] = o->e[axis_v];

    for (int i = 0; i < 4; ++i)
    {
        _obb_local_to_world(o, local[i], out[i].p);
    }
}

static void
_obb_find_incident_face(const gsk_OBB *incident,
                        vec3 reference_normal,
                        int *out_axis,
                        float *out_sign)
{
    float min_dot   = FLT_MAX;
    int best_axis   = 0;
    float best_sign = 1.0f;

    for (int i = 0; i < 3; ++i)
    {
        float d_pos = glm_vec3_dot(reference_normal, incident->u[i]);
        float d_neg = -d_pos;

        if (d_pos < min_dot)
        {
            min_dot   = d_pos;
            best_axis = i;
            best_sign = 1.0f;
        }

        if (d_neg < min_dot)
        {
            min_dot   = d_neg;
            best_axis = i;
            best_sign = -1.0f;
        }
    }

    *out_axis = best_axis;
    *out_sign = best_sign;
}

static int
_clip_polygon_plane(const gsk_ClipVertex *in,
                    int in_count,
                    gsk_ClipVertex *out,
                    vec3 plane_normal,
                    float plane_offset)
{
    if (in_count <= 0) return 0;

    int out_count = 0;

    gsk_ClipVertex prev = in[in_count - 1];
    float prev_dist     = glm_vec3_dot(plane_normal, prev.p) - plane_offset;
    u8 prev_inside      = prev_dist <= 0.0f;

    for (int i = 0; i < in_count; ++i)
    {
        gsk_ClipVertex curr = in[i];
        float curr_dist     = glm_vec3_dot(plane_normal, curr.p) - plane_offset;
        u8 curr_inside      = curr_dist <= 0.0f;

        if (curr_inside != prev_inside)
        {
            float denom = prev_dist - curr_dist;
            float t     = 0.0f;

            if (fabsf(denom) > 1e-6f) t = prev_dist / denom;

            vec3 edge;
            glm_vec3_sub(curr.p, prev.p, edge);
            glm_vec3_scale(edge, t, edge);

            glm_vec3_add(prev.p, edge, out[out_count].p);
            out_count++;
        }

        if (curr_inside)
        {
            glm_vec3_copy(curr.p, out[out_count].p);
            out_count++;
        }

        prev        = curr;
        prev_dist   = curr_dist;
        prev_inside = curr_inside;
    }

    return out_count;
}

/*************************************************************************
 * OBB-contacts helpers
 *************************************************************************/

static void
_make_obb_face_contacts(gsk_OBB *a,
                        gsk_OBB *b,
                        const gsk_OBBSatResult *sat,
                        gsk_CollisionManifold *out)
{
    const gsk_OBB *ref = NULL;
    const gsk_OBB *inc = NULL;

    int ref_axis   = 0;
    float ref_sign = 1.0f;

    vec3 n;
    glm_vec3_copy(sat->normal, n); // A -> B

    u8 reference_is_a = TRUE;

    if (sat->type == GSK_OBB_AXIS_FACE_A)
    {
        reference_is_a = TRUE;
        ref            = a;
        inc            = b;

        ref_axis = sat->axis_a;
        ref_sign = signf_nonzero(glm_vec3_dot(n, ref->u[ref_axis]));
    } else if (sat->type == GSK_OBB_AXIS_FACE_B)
    {
        reference_is_a = FALSE;
        ref            = b;
        inc            = a;

        /*
         * Reference is B, but manifold normal is still A -> B.
         * The outward reference face on B points opposite A -> B.
         */
        vec3 ref_normal;
        glm_vec3_negate_to(n, ref_normal);

        ref_axis = sat->axis_b;
        ref_sign = signf_nonzero(glm_vec3_dot(ref_normal, ref->u[ref_axis]));
    } else
    {
        return;
    }

    vec3 ref_normal;
    glm_vec3_scale(ref->u[ref_axis], ref_sign, ref_normal);

    /*
     * If reference is A, ref_normal should match n.
     * If reference is B, ref_normal should oppose n.
     */
    vec3 contact_normal;
    glm_vec3_copy(n, contact_normal);

    int inc_axis   = 0;
    float inc_sign = 1.0f;
    _obb_find_incident_face(inc, ref_normal, &inc_axis, &inc_sign);

    gsk_ClipVertex poly_a[GSK_CLIP_MAX_VERTS];
    gsk_ClipVertex poly_b[GSK_CLIP_MAX_VERTS];

    _obb_get_face_vertices(inc, inc_axis, inc_sign, poly_a);
    int count = 4;

    /*
     * Clip incident face against the 4 side planes of the reference face.
     */
    int side_axis_0 = (ref_axis + 1) % 3;
    int side_axis_1 = (ref_axis + 2) % 3;

    vec3 side_n;
    float side_offset;

    // +side_axis_0 plane: dot(+u, x) <= dot(+u, c) + extent
    glm_vec3_copy(ref->u[side_axis_0], side_n);
    side_offset = glm_vec3_dot(side_n, ref->c) + ref->e[side_axis_0];
    count = _clip_polygon_plane(poly_a, count, poly_b, side_n, side_offset);
    if (count == 0) return;

    // -side_axis_0 plane: dot(-u, x) <= dot(-u, c) + extent
    glm_vec3_negate_to(ref->u[side_axis_0], side_n);
    side_offset = glm_vec3_dot(side_n, ref->c) + ref->e[side_axis_0];
    count = _clip_polygon_plane(poly_b, count, poly_a, side_n, side_offset);
    if (count == 0) return;

    // +side_axis_1 plane
    glm_vec3_copy(ref->u[side_axis_1], side_n);
    side_offset = glm_vec3_dot(side_n, ref->c) + ref->e[side_axis_1];
    count = _clip_polygon_plane(poly_a, count, poly_b, side_n, side_offset);
    if (count == 0) return;

    // -side_axis_1 plane
    glm_vec3_negate_to(ref->u[side_axis_1], side_n);
    side_offset = glm_vec3_dot(side_n, ref->c) + ref->e[side_axis_1];
    count = _clip_polygon_plane(poly_b, count, poly_a, side_n, side_offset);
    if (count == 0) return;

    /*
     * Reference face plane.
     * Keep points that are behind/penetrating the reference face.
     */
    vec3 ref_face_center;
    glm_vec3_copy(ref->c, ref_face_center);

    vec3 face_offset;
    glm_vec3_scale(ref_normal, ref->e[ref_axis], face_offset);
    glm_vec3_add(ref_face_center, face_offset, ref_face_center);

    float ref_plane_offset = glm_vec3_dot(ref_normal, ref_face_center);

    out->has_collision  = TRUE;
    out->contacts_count = 0;
    out->depth          = sat->depth;
    glm_vec3_copy(contact_normal, out->normal);

    for (int i = 0;
         i < count && out->contacts_count < GSK_MAX_COLLISION_CONTACTS;
         ++i)
    {
        vec3 p_inc;
        glm_vec3_copy(poly_a[i].p, p_inc);

        /*
         * Signed distance from clipped incident point to reference face plane.
         *
         * If dist <= 0, point is inside / behind reference plane.
         */
        float dist = glm_vec3_dot(ref_normal, p_inc) - ref_plane_offset;

        if (dist <= OBB_EPSILON)
        {
            gsk_CollisionPoints *cp = &out->contacts[out->contacts_count++];

            cp->has_collision = TRUE;
            cp->penetration =
              -dist; // TODO: see if has to be negative (or clamp)
            cp->depth = cp->penetration;

            glm_vec3_copy(contact_normal, cp->normal);

            /*
             * p_inc lies on incident box.
             * Project it onto reference face to get corresponding ref point.
             */
            vec3 correction;
            glm_vec3_scale(ref_normal, dist, correction);

            vec3 p_ref;
            glm_vec3_sub(p_inc, correction, p_ref);

            if (reference_is_a)
            {
                glm_vec3_copy(p_ref, cp->point_a);
                glm_vec3_copy(p_inc, cp->point_b);
            } else
            {
                glm_vec3_copy(p_inc, cp->point_a);
                glm_vec3_copy(p_ref, cp->point_b);
            }
        }
    }

    if (out->contacts_count == 0) { out->has_collision = FALSE; }
}

static void
_make_obb_edge_contact(gsk_OBB *a,
                       gsk_OBB *b,
                       const gsk_OBBSatResult *sat,
                       gsk_CollisionManifold *out)
{
    vec3 a0, a1;
    vec3 b0, b1;

    vec3 neg_normal;
    glm_vec3_copy(sat->normal, neg_normal);
    glm_vec3_negate(neg_normal);

    _get_obb_edge_segment(a, sat->axis_a, sat->normal, a0, a1);
    _get_obb_edge_segment(b, sat->axis_b, neg_normal, b0, b1);

    gsk_CollisionPoints *cp = &out->contacts[0];

    _closest_points_segments(a0, a1, b0, b1, cp->point_a, cp->point_b);

    cp->has_collision = TRUE;
    cp->penetration   = sat->depth;
    cp->depth         = sat->depth;
    glm_vec3_copy(sat->normal, cp->normal);

    out->has_collision  = TRUE;
    out->contacts_count = 1;
    out->depth          = sat->depth;
    glm_vec3_copy(sat->normal, out->normal);
}

/*************************************************************************
 * Main testing functions
 *************************************************************************/

gsk_OBB
gsk_physics_sat_obb_make(const gsk_BoxCollider *col, vec3 pos, mat3 rot)
{
    gsk_OBB O = {0};

    vec3 minv, maxv;
    glm_vec3_copy(col->bounds[0], minv);
    glm_vec3_copy(col->bounds[1], maxv);

    // local-space center
    vec3 local_center;
    glm_vec3_add(minv, maxv, local_center);
    glm_vec3_scale(local_center, 0.5f, local_center);

    // Half-extents
    O.e[0] = 0.5f * (maxv[0] - minv[0]);
    O.e[1] = 0.5f * (maxv[1] - minv[1]);
    O.e[2] = 0.5f * (maxv[2] - minv[2]);

    // Axes rotation columns
    glm_vec3_copy(rot[0], O.u[0]);
    glm_vec3_copy(rot[1], O.u[1]);
    glm_vec3_copy(rot[2], O.u[2]);

    // ensure normalized
    glm_vec3_normalize(O.u[0]);
    glm_vec3_normalize(O.u[1]);
    glm_vec3_normalize(O.u[2]);

    // World-Space OBB Center
    vec3 center_offset_world = GLM_VEC3_ZERO_INIT;

    glm_vec3_scale(O.u[0], local_center[0], center_offset_world);

    vec3 term;
    glm_vec3_scale(O.u[1], local_center[1], term);
    glm_vec3_add(center_offset_world, term, center_offset_world);

    glm_vec3_scale(O.u[2], local_center[2], term);
    glm_vec3_add(center_offset_world, term, center_offset_world);

    glm_vec3_add(pos, center_offset_world, O.c);

    return O;
}

gsk_OBBSatResult
gsk_physics_sat_obb_test(gsk_OBB *a, gsk_OBB *b)
{
    gsk_OBBSatResult ret = {0};
    ret.has_collision    = TRUE;
    ret.depth            = FLT_MAX;
    ret.type             = GSK_OBB_AXIS_NONE;
    ret.axis_a           = -1;
    ret.axis_b           = -1;
    glm_vec3_zero(ret.normal);

    // A face axes
    for (int i = 0; i < 3; ++i)
    {
        if (!_sat_try_axis(a, b, a->u[i], GSK_OBB_AXIS_FACE_A, i, -1, &ret))
        {
            ret.has_collision = FALSE;
            return ret;
        }
    }

    // B face axes
    for (int i = 0; i < 3; ++i)
    {
        if (!_sat_try_axis(a, b, b->u[i], GSK_OBB_AXIS_FACE_B, -1, i, &ret))
        {
            ret.has_collision = FALSE;
            return ret;
        }
    }

    // Edge cross axes
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            vec3 axis;
            glm_vec3_cross(a->u[i], b->u[j], axis);

            if (!_sat_try_axis(a, b, axis, GSK_OBB_AXIS_EDGE, i, j, &ret))
            {
                ret.has_collision = FALSE;
                return ret;
            }
        }
    }

    return ret;
}

/*************************************************************************
 * OBB-SAT contacts functions
 *************************************************************************/

void
gsk_physics_sat_contact_single(gsk_OBB *a,
                               gsk_OBB *b,
                               const gsk_OBBSatResult *sat,
                               gsk_CollisionPoints *out)
{
    out->has_collision = TRUE;
    out->depth         = sat->depth;
    glm_vec3_negate_to(sat->normal, out->normal);

    if (sat->type == GSK_OBB_AXIS_FACE_A)
    {
        int axis = sat->axis_a;

        float face_sign =
          (glm_vec3_dot(sat->normal, a->u[axis]) >= 0.0f) ? 1.0f : -1.0f;

        // Clamp B center onto A's reference face.
        _obb_clamp_point_to_face(a, axis, face_sign, b->c, out->point_a);

        // Approximate corresponding point on B.
        vec3 offset;
        glm_vec3_scale(sat->normal, sat->depth, offset);
        glm_vec3_sub(out->point_a, offset, out->point_b);

        return;
    }

    if (sat->type == GSK_OBB_AXIS_FACE_B)
    {
        int axis = sat->axis_b;

        // B reference face should oppose A->B normal.
        vec3 neg_normal;
        glm_vec3_negate_to(sat->normal, neg_normal);

        float face_sign =
          (glm_vec3_dot(neg_normal, b->u[axis]) >= 0.0f) ? 1.0f : -1.0f;

        // Clamp A center onto B's reference face.
        _obb_clamp_point_to_face(b, axis, face_sign, a->c, out->point_b);

        // Approximate corresponding point on A.
        vec3 offset;
        glm_vec3_scale(sat->normal, sat->depth, offset);
        glm_vec3_add(out->point_b, offset, out->point_a);

        return;
    }

    if (sat->type == GSK_OBB_AXIS_EDGE)
    {
        vec3 a0, a1;
        vec3 b0, b1;

        vec3 neg_normal;
        glm_vec3_negate_to(sat->normal, neg_normal);

        _get_obb_edge_segment(a, sat->axis_a, sat->normal, a0, a1);
        _get_obb_edge_segment(b, sat->axis_b, neg_normal, b0, b1);

        _closest_points_segments(a0, a1, b0, b1, out->point_a, out->point_b);

        return;
    }

    // Fallback: support midpoint approximation.
    _obb_support_point(a, sat->normal, out->point_a);

    vec3 neg_normal;
    glm_vec3_negate_to(sat->normal, neg_normal);
    _obb_support_point(b, neg_normal, out->point_b);
}

/*************************************************************************
 * OBB-SAT testing functions
 *************************************************************************/

gsk_CollisionManifold
gsk_physics_sat_find_obb_obb_manifold(gsk_OBB *a, gsk_OBB *b)
{
    gsk_CollisionManifold ret = {0};

    gsk_OBBSatResult sat = gsk_physics_sat_obb_test(a, b);

    if (!sat.has_collision) return ret;

    if (sat.type == GSK_OBB_AXIS_EDGE)
    {
        _make_obb_edge_contact(a, b, &sat, &ret);
    } else
    {
        _make_obb_face_contacts(a, b, &sat, &ret);

        /*
         * Fallback: useful for numerical weirdness or nearly-degenerate face
         * cases.
         */
        if (!ret.has_collision)
        {
            gsk_CollisionPoints fallback = {0};
            gsk_physics_sat_contact_single(a, b, &sat, &fallback);

            ret.has_collision  = TRUE;
            ret.contacts_count = 1;
            ret.depth          = sat.depth;
            glm_vec3_copy(sat.normal, ret.normal);

            ret.contacts[0].has_collision = TRUE;
            glm_vec3_copy(fallback.point_a, ret.contacts[0].point_a);
            glm_vec3_copy(fallback.point_b, ret.contacts[0].point_b);
            glm_vec3_copy(sat.normal, ret.contacts[0].normal);
            ret.contacts[0].penetration = sat.depth;
            ret.contacts[0].depth       = sat.depth;
        }
    }

#if 1
    // TODO: maybe don't negate?
    for (int i = 0; i < ret.contacts_count; i++)
    {
        // glm_vec3_normalize(ret.contacts[i].normal);
        glm_vec3_negate(ret.contacts[i].normal);
        // ret.contacts[i].depth = fabsf(ret.contacts[i].depth);

        //_invert_points(ret.contacts[i].point_a, ret.contacts[i].point_b);
        //_invert_points(ret.contacts[i].local_point_a,
        //               ret.contacts[i].local_point_b);
    }
    glm_vec3_negate(ret.normal);
#endif

#if 0
    // create local points
    for (int i = 0; i < ret.contacts_count; i++)
    {
        _obb_world_to_local(
          a, ret.contacts[i].point_a, ret.contacts[i].local_point_a);

        _obb_world_to_local(
          b, ret.contacts[i].point_b, ret.contacts[i].local_point_b);
    }
#endif

    return ret;
}

gsk_CollisionPoints
gsk_pyhysics_sat_find_obb_sphere_points(const gsk_OBB *box,
                                        vec3 sphere_center,
                                        float sphere_radius)
{
    gsk_CollisionPoints ret = {0};

    vec3 sphere_local;
    _obb_world_to_local(box, sphere_center, sphere_local);

    vec3 closest_local;
    closest_local[0] = clampf(sphere_local[0], -box->e[0], box->e[0]);
    closest_local[1] = clampf(sphere_local[1], -box->e[1], box->e[1]);
    closest_local[2] = clampf(sphere_local[2], -box->e[2], box->e[2]);

    vec3 closest_world;
    _obb_local_to_world(box, closest_local, closest_world);

    vec3 box_to_sphere;
    glm_vec3_sub(sphere_center, closest_world, box_to_sphere);

    float dist2 = glm_vec3_norm2(box_to_sphere);
    float r2    = sphere_radius * sphere_radius;

    if (dist2 > r2) return ret;

    ret.has_collision = TRUE;

    float dist = sqrtf(dist2);

    if (dist > 1e-6f)
    {
        glm_vec3_scale(box_to_sphere, 1.0f / dist, ret.normal);

        glm_vec3_copy(closest_world, ret.point_a);

        vec3 sphere_offset;
        glm_vec3_scale(ret.normal, sphere_radius, sphere_offset);
        glm_vec3_sub(sphere_center, sphere_offset, ret.point_b);

        ret.depth = sphere_radius - dist;
    } else
    {
        /*
         * Sphere center is inside the box.
         * Need choose nearest box face as normal.
         */
        float min_pen = FLT_MAX;
        int axis      = 0;
        float sign    = 1.0f;

        for (int i = 0; i < 3; ++i)
        {
            float d_pos = box->e[i] - sphere_local[i];
            float d_neg = box->e[i] + sphere_local[i];

            if (d_pos < min_pen)
            {
                min_pen = d_pos;
                axis    = i;
                sign    = 1.0f;
            }

            if (d_neg < min_pen)
            {
                min_pen = d_neg;
                axis    = i;
                sign    = -1.0f;
            }
        }

        glm_vec3_scale(box->u[axis], sign, ret.normal);

        vec3 local_face_point;
        glm_vec3_copy(sphere_local, local_face_point);
        local_face_point[axis] = sign * box->e[axis];

        _obb_local_to_world(box, local_face_point, ret.point_a);

        vec3 sphere_offset;
        glm_vec3_scale(ret.normal, -sphere_radius, sphere_offset);
        glm_vec3_add(sphere_center, sphere_offset, ret.point_b);

        /*
         * Inside case depth includes distance to escape face plus sphere
         * radius.
         */
        ret.depth = sphere_radius + min_pen;
    }

    return ret;
}

#if 1
gsk_CollisionPoints
gsk_physics_sat_find_obb_capsule(const gsk_OBB *box,
                                 vec3 cap_a,
                                 vec3 cap_b,
                                 float radius)
{
    gsk_CollisionPoints ret = {0};

    /*
     * Approximation:
     * use closest point on capsule segment to box center,
     * then closest point on OBB to that point.
     */
    vec3 closest_seg;
    _closest_point_line_segment(
      cap_a, cap_b, (vec3) {box->c[0], box->c[1], box->c[2]}, closest_seg);

    vec3 closest_local;
    _obb_world_to_local(box, closest_seg, closest_local);

    vec3 closest_box_local = {
      clampf(closest_local[0], -box->e[0], box->e[0]),
      clampf(closest_local[1], -box->e[1], box->e[1]),
      clampf(closest_local[2], -box->e[2], box->e[2]),
    };

    vec3 closest_box_world;
    _obb_local_to_world(box, closest_box_local, closest_box_world);

    vec3 diff;
    glm_vec3_sub(closest_seg, closest_box_world, diff);

    float dist2 = glm_vec3_norm2(diff);

    if (dist2 > radius * radius) return ret;

    ret.has_collision = TRUE;

    float dist = sqrtf(dist2);

    if (dist > 1e-6f)
    {
        glm_vec3_scale(diff, 1.0f / dist, ret.normal);
    } else
    {
        glm_vec3_copy((vec3) {0, 1, 0}, ret.normal);
    }

    glm_vec3_copy(closest_box_world, ret.point_a);

    vec3 offset;
    glm_vec3_scale(ret.normal, radius, offset);
    glm_vec3_sub(closest_seg, offset, ret.point_b);

    ret.depth = radius - dist;

    return ret;
}
#endif