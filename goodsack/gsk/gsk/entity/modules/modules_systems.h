/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef _MODULES_SYSTEMS_H__
#define _MODULES_SYSTEMS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "entity/modules/camera/camera.h"
#include "entity/modules/model/model.h"
#include "entity/modules/transform/transform.h"

#include "entity/modules/audio/audio_listener.h"
#include "entity/modules/audio/audio_source.h"

#include "entity/modules/animator/animator.h"

#include "entity/modules/model/model.h"
#include "entity/modules/model/model_draw.h"

#include "entity/modules/physics/collider_debug_draw-system.h"
#include "entity/modules/physics/collider_setup-system.h"
#include "entity/modules/physics/rigidbody-system.h"
#include "entity/modules/physics/rigidbody_forces-system.h"

#include "entity/modules/misc/health_setup.h"
#include "entity/modules/player/player_controller-system.h"

#include "entity/modules/particles_ecs/particles_ecs-system.h"

#ifdef __cplusplus
}
#endif

#endif // _MODULES_SYSTEMS_H__