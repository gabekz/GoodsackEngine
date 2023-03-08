
#ifndef H_ANIMATION
#define H_ANIMATION

#include <core/graphics/mesh/mesh.h>
#include <util/sysdefs.h>

#ifdef __cplusplus
extern "C" {
#endif

void
animation_play(Animation *animation, ui32 index);

void
animation_set_keyframe(Animation *animation, ui32 keyframe);

void
animation_set_keyframe_lerp(Animation *animation, ui32 keyframe, ui32 ratio);

#ifdef __cplusplus
}
#endif

#endif // H_ANIMATION