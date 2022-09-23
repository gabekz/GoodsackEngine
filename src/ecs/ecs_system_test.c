#include <ecs/ecs.h>

#include <model/mesh.h>

#define ECS_SYSTEM
#define SCENE0

#define C_MESH          0
#define C_POSITION      0

struct ComponentMesh {
    Material *material;
    Model *model;
    mat4 *modelMatrix;

    struct {
        ui16 drawMode: 2;
        ui16 cullMode: 3;
    } properties;

    ui32 vbo;
};

struct ComponentTest {
    ui32 x, y;
};

static void test_main() {

    struct ComponentMesh c = ((struct ComponentMesh){
        .material = NULL,
        .model = NULL,
        .modelMatrix = NULL,
        .properties = {
            .drawMode = 0x00,
            .cullMode = 0x000
        }
    });

    //printf("%x", c.properties.drawMode);
}

static void init(Entity entity){
    // components required
    //struct ComponentTest test = ecs_get(entity, C_TEST);

    // temp definition for testing (init can be found in main)

    if(!(ecs_has(entity, C_TEST))) return;

    struct ComponentTest *t = ecs_get(entity, C_TEST);
    //printf("\nEntity id: %d:\n data: %d, %d\n", (int)entity.id, t->x, t->y);

    //printf("\nInit from s_test\n");
}

static void update(Entity entity){
    if(!(ecs_has(entity, C_TEST))) return;
    //if(!(ecs_has(entity, C_TEST2))) return;

    struct ComponentTest *t = ecs_get(entity, C_TEST);
    t->x += 1;
    //printf("\n\n%d, ", t->x);
    /*
    ENTITY_QUERY(CMP_HUMAN) {
        if(entity.health <= 0) {
            entity.health = 0;
            entity.isDead = true;
            return;
        }
    }
    */
}

void s_test_init(ECS *ecs) {
    ecs_system_register(ecs, ((ECSSystem){
        .init       = (ECSSubscriber) init,
        .destroy    = NULL,
        .render     = NULL,
        .update     = (ECSSubscriber) update,
    })); 
}
