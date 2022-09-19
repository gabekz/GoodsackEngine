#if 0 // disable shit for testing

#include <ecs/system.h>

#define ECS_SYSTEM
#define SCENE0

component Human {
    int  health;
    bool isAlive;
}

component Transform {
    vec3 postition;
    vec3 rotation;
    vec3 scale;
};

component Mesh {
    Model     model;
    Material  material;
};

void START {
    Entity e = ENTITY_CREATE(CMP_MESH, CMP_TRANSFORM, mesh, 0, 0, 0);
    ENTITY_ADD_COMPONENT(e, CMP_HUMAN, 100, true);

    ENTITY_QUERY(CMP_MESH, CMP_TRANSFORM) {
        if(ENTITY_QUERY(query_result, CMP_HUMAN)) return;

        transform_send_matrix(e->material, e->transform->position);
    }
}

void UPDATE {
    ENTITY_QUERY(CMP_HUMAN) {
        if(entity.health <= 0) {
            entity.health = 0;
            entity.isDead = true;
            return;
        }
        //entity.health -= 1 * deltaTime;
    }
}

#endif

#include <stdio.h>

void entity_query(int e, int arg, ...) {
    // entity code
}

int test() {

    void (*fun_ptr)(int, int, ...) = &entity_query;
    (*fun_ptr)(2, 1);
    return 0;
}

#if 0
// definitions for ECS_SYSTEM would...

ecs_add_system(entity_query);

#endif
