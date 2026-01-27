#ifndef __GSK_PHYSICS_SOLVER_DATA_H__
#define __GSK_PHYSICS_SOLVER_DATA_H__

#include "entity/ecs.h"
#include "physics/physics_types.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct gsk_PhysicsSolverData
{
    gsk_C_Rigidbody *const p_rigidbody;
    gsk_C_Transform *const p_transform;
    gsk_CollisionResult *p_collision_result;
    const gsk_Entity entity;
    const f64 delta;
} gsk_PhysicsSolverData;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __GSK_PHYSICS_SOLVER_DATA_H__