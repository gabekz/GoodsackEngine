#include "transform.h"

#include <core/ecs.h>
#include <core/shader.h>

static void UpdateMatrix(ShaderProgram *shader, const char *str) {

}

static void init(Entity e) {
    if(!(ecs_has(e, C_TRANSFORM))) return;
    struct ComponentTransform *transform = ecs_get(e, C_TRANSFORM);
}

static void update(Entity e) {
    if(!(ecs_has(e, C_TRANSFORM))) return;
    struct ComponentTransform *transform = ecs_get(e, C_TRANSFORM);
    //struct ComponentTransform *material = ecs_get(e, C_TRANSFORM);

    //UpdateMatrix();
}

static void s_transform_init(ECS *ecs) {
    ecs_component_register(ecs, C_TRANSFORM , sizeof(struct ComponentTransform));
    ecs_system_register(ecs, ((ECSSystem){
        .init       = (ECSSubscriber) init,
        .destroy    = NULL,
        .render     = NULL,
        .update     = (ECSSubscriber) update,

    }));
}
