#ifndef H_SCENE
#define H_SCENE


    /* scene steps::
     * 1) Initialize and set active camera
     * 2) Create Light information
     [Standard]
     * 3) Renderer tick [all logic updates + shader updates]
     * 4) Render [all meshes in meshList]
     [ECS]
     * 3) Process Systems
     */

#include <util/sysdefs.h>

#include <model/mesh.h>
#include <core/lighting.h>

typedef struct _scene Scene;

struct _scene {
    ui32 id;

    ui32 meshC, lightC;
    Mesh **meshL;
    Light **lightL;

    struct _ecs *ecs;
};

void scene_update(Scene *self);
void scene_draw(Scene *self, bool isExplicit, Material *override);

#endif // H_SCENE
