/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "animator.h"

#include "core/device/device.h"
#include "core/graphics/mesh/animation.h"
#include "core/graphics/mesh/mesh.h"

#include "entity/__generated__/components_gen.h"

#include <string.h>

#define ANIMATOR_REWIND 0 // TODO: Move to component as a setting
#define ENABLE_LERP     1

static gsk_Skeleton *
_allocate_new_skeleton(gsk_Mesh *p_mesh)
{
    gsk_Skeleton *ret = malloc(sizeof(gsk_Skeleton));

    ret->jointsCount = p_mesh->meshData->skeleton.jointsCount;
    glm_mat4_copy(p_mesh->meshData->skeleton.rootMatrix, ret->rootMatrix);
    ret->name   = strdup(p_mesh->meshData->skeleton.name);
    ret->joints = p_mesh->meshData->skeleton.joints;

    gsk_Joint **new_joints = malloc(sizeof(gsk_Joint *) * ret->jointsCount);

    for (int i = 0; i < ret->jointsCount; i++)
    {
        gsk_Joint *new_joint = malloc(sizeof(gsk_Joint));

        new_joint->childrenCount = ret->joints[i]->childrenCount;
        new_joint->id            = ret->joints[i]->id;
        new_joint->name          = strdup(ret->joints[i]->name);
        new_joint->override      = ret->joints[i]->override;
        new_joint->pose          = ret->joints[i]->pose;
        new_joint->parent        = NULL;
        new_joint->parent_id     = -1;

        glm_mat4_copy(ret->joints[i]->mInvBindPose, new_joint->mInvBindPose);

        if (ret->joints[i]->parent != NULL &&
            ret->joints[i]->parent_id <= ret->jointsCount &&
            ret->joints[i]->parent_id != -1)
        {
            new_joint->parent_id = ret->joints[i]->parent_id;
        }

        new_joints[i] = new_joint;
    }

    // UPDATE JOINTS LIST
    ret->joints = new_joints;

    for (int i = 0; i < ret->jointsCount; i++)
    {
        gsk_Joint *p_joint = ret->joints[i];

        if (p_joint->parent_id != -1)
        {
            p_joint->parent = ret->joints[p_joint->parent_id];
        }

        // TODO: ADD check somewhere to ensure parent_id does not equal our ID
        // TODO: change name of parent_id to parent_index
    }

    return ret;
}

static void
init(gsk_Entity e)
{
    if (!(gsk_ecs_has(e, C_ANIMATOR))) return;
    if (!(gsk_ecs_has(e, C_MODEL))) return;

    struct ComponentAnimator *cmp_animator = gsk_ecs_get(e, C_ANIMATOR);
    struct ComponentModel *cmp_model       = gsk_ecs_get(e, C_MODEL);

    gsk_Model *p_model      = (gsk_Model *)cmp_model->pModel;
    u32 skeleton_mesh_index = 0;

    // TODO: create new skeleton here on the model
    // TODO: grab animationset from model (if it has any)
    // - loop through meshes - find skinned mesh - find animationset

    for (int i = 0; i < p_model->meshesCount; i++)
    {
        gsk_Mesh *p_mesh = p_model->meshes[i];
        if (p_mesh->meshData->isSkinnedMesh)
        {
            cmp_model->_skeleton = _allocate_new_skeleton(p_mesh);
            skeleton_mesh_index  = (u32)i;
            break;
        }
    }

    if (cmp_model->_skeleton == NULL)
    {
        _gsk_ecs_set_internal(e, C_ANIMATOR, FALSE);
        return;
    }

    // TODO: should not automatically be set here
    gsk_AnimationSet *p_animation_set =
      &p_model->meshes[skeleton_mesh_index]->meshData->animations;

    // TODO: REMOVE SKELETON SHIT HERE
    // gsk_Animation *animation = p_skeleton->animation;
    gsk_Animation *animation =
      p_animation_set->p_animations[p_animation_set->cnt_animation_index];

    gsk_animation_set_keyframe(
      (gsk_Skeleton *)cmp_model->_skeleton, animation, 0);

    cmp_animator->p_animation_set = p_animation_set;

    cmp_animator->cntAnimation = animation;
    cmp_animator->cntTime      = 0;

    cmp_animator->timerNow = cmp_animator->timerStart;

    cmp_animator->cntKeyframeIndex = 0;
    cmp_animator->nxtKeyframeIndex = 1;

    cmp_animator->is_transition_delayed = TRUE;
    cmp_animator->is_looping            = FALSE;
    cmp_animator->is_playing            = TRUE;

    cmp_animator->force_replay = FALSE;
}

static void
update(gsk_Entity e)
{
    if (!(gsk_ecs_has(e, C_ANIMATOR))) return;
    if (!(gsk_ecs_has(e, C_MODEL))) return;

    struct ComponentAnimator *animator = gsk_ecs_get(e, C_ANIMATOR);
    struct ComponentModel *model       = gsk_ecs_get(e, C_MODEL);
    gsk_Mesh *mesh                     = model->mesh;

    gsk_Model *p_model = (gsk_Model *)model->pModel;

    if (!mesh->meshData->isSkinnedMesh) return;

    if (animator->is_playing)
    {
        animator->timerNow +=
          (gsk_device_getTime().delta_time) * gsk_device_getTime().time_scale;
    }

    gsk_Animation *cntAnimation = animator->cntAnimation;
    u32 cntKeyframeIndex        = animator->cntKeyframeIndex;
    u32 nxtKeyframeIndex        = cntKeyframeIndex + 1;

    u32 cnt_anim_index = cntAnimation->index;

    gsk_AnimationSet *p_animation_set =
      (gsk_AnimationSet *)animator->p_animation_set;

    // if (nxtKeyframeIndex >= cntAnimation->keyframesCount &&
    //    !animator->is_looping) {
    //    animator->is_playing = FALSE;
    //}

    // check if we want to have a delay in animation transition

    // u16 has_anim_changed = FALSE;
    u16 has_anim_changed = (cnt_anim_index != animator->animation_index);

    // switched + delayed
    u16 delay_ready = (!animator->is_transition_delayed && has_anim_changed);

    // switched + delayed + (looping + not playing)
    u16 has_anim_ended = (delay_ready) ||
                         (animator->is_looping && !animator->is_playing) ||
                         (has_anim_changed && !animator->is_playing);

    // NOTE: At this point, we probably don't need to check the timer.
    // Just ensure that the keyframe doesn't go out-of-bounds. That is
    // a lot more practical than a timing check..
    if (animator->timerNow >= cntAnimation->duration ||
        nxtKeyframeIndex > cntAnimation->keyframesCount - 1 || has_anim_ended ||
        animator->force_replay)
    {
        animator->timerNow         = 0;
        animator->cntKeyframeIndex = 0;

        animator->is_playing =
          (animator->force_replay) || (animator->is_looping);
        animator->force_replay = FALSE;

        // potentially update animation. NOTE: Important to leave this here -
        // we only want to switch animations if we are done with the current
        // one.
        // kind of cursed.
        animator->cntAnimation =
          p_animation_set->p_animations[animator->animation_index];

#if ANIMATOR_REWIND
        gsk_animation_set_keyframe(animator->cntAnimation, 1);
#endif // ANIMATOR_REWIND

        return;
    }

    if (!animator->is_playing && animator->is_holding_end_frame) return;

    gsk_Keyframe *cntKeyframe = cntAnimation->keyframes[cntKeyframeIndex];
    gsk_Keyframe *nxtKeyframe = cntAnimation->keyframes[nxtKeyframeIndex];

    float ratio = (animator->timerNow - cntKeyframe->frameTime) /
                  (nxtKeyframe->frameTime - cntKeyframe->frameTime);

#if ENABLE_LERP
    gsk_animation_set_keyframe_lerp((gsk_Skeleton *)model->_skeleton,
                                    animator->cntAnimation,
                                    cntKeyframeIndex,
                                    nxtKeyframeIndex,
                                    ratio);
#endif // ENABLE_LERP

    if (ratio >= nxtKeyframe->frameTime)
    {

#if !(ENABLE_LERP)
        gsk_animation_set_keyframe(
          p_model->p_skeleton, animator->cntAnimation, cntKeyframeIndex, ratio);
#endif // !(ENABLE_LERP)

        animator->cntKeyframeIndex = nxtKeyframeIndex;
    }
}

void
s_animator_init(gsk_ECS *ecs)
{
    //_ECS_DECL_COMPONENT(ecs, C_ANIMATOR, sizeof(struct ComponentAnimator));
    gsk_ecs_system_register(ecs,
                            ((gsk_ECSSystem) {
                              .init    = (gsk_ECSSubscriber)init,
                              .destroy = NULL,
                              .render  = NULL,
                              .update  = (gsk_ECSSubscriber)update,
                            }));
}
